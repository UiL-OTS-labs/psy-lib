
#define G_LOG_DOMAIN "SdlVSync"

#include <psylib.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_opengl.h>
#include <assert.h>

#include "cmd_vsync_opts.h"
#include "monitor_shader_paths.h"

PsyDuration *g_frame_dur = NULL;

typedef struct Stimulus {
    int64_t start_frame; // The number of the frame this stimulus should start
    int64_t num_frames;  // The number of frames this stimulus lasts
} Stimulus;

typedef struct FrameStats {

    int64_t nth_frame;        // nth_frame to be presented starts a -1
    int64_t n_missed_frames;  // the number of missed frames
    int64_t last_known_frame; // the number of the last known frame.

    PsyTimePoint *last_frame_time;
} FrameStats;

static FrameStats *
frame_stats_new(void)
{
    FrameStats *stats = calloc(1, sizeof(FrameStats));

    return stats;
}

static void
frame_stats_free(FrameStats *stats)
{
    if (stats->last_frame_time)
        psy_time_point_free(stats->last_frame_time);

    free(stats);
}

void
update_frame_stats(FrameStats   *self,
                   int64_t       n,
                   int64_t       n_missed_frames,
                   PsyTimePoint *tp_next_frame)
{
    if (!self) {
        g_critical("Self pointer is NULL");
        return;
    }

    self->last_known_frame = self->nth_frame + 1;

    self->nth_frame += (n + n_missed_frames);
    self->n_missed_frames += n_missed_frames;

    if (self->last_frame_time)
        psy_time_point_free(self->last_frame_time);

    self->last_frame_time = tp_next_frame;
}

const SDL_DisplayMode *
get_display_mode_for_monitor(int nth_display, SDL_Rect *out)
{
    int                    num;
    SDL_DisplayID         *displays = SDL_GetDisplays(&num);
    const SDL_DisplayMode *ret      = NULL;

    if (nth_display < 0)
        nth_display = 0;

    if (nth_display >= num)
        nth_display = num - 1;

    if (nth_display < 0) // unlikely
        goto cleanup;

    ret = SDL_GetCurrentDisplayMode(displays[nth_display]);
    if (!ret) {
        g_critical("Unable to get display mode: %s", SDL_GetError());
        goto cleanup;
    }
    SDL_GetDisplayBounds(displays[nth_display], out);
cleanup:
    SDL_free(displays);
    return ret;
}

PsyShaderProgram *
init_shaders(void)
{
    PsyShaderProgram *prog  = PSY_SHADER_PROGRAM(psy_gl_program_new());
    GError           *error = NULL;

    char vert_shader[1024];
    char frag_shader[1024];

    g_snprintf(
        vert_shader, 1024, "%s/%s", PSY_BUILD_PSY_DIR, "uniform-color.vert");
    g_snprintf(
        frag_shader, 1024, "%s/%s", PSY_BUILD_PSY_DIR, "uniform-color.frag");

    g_info("Vert shader path = %s", vert_shader);
    g_info("Frag shader path = %s", frag_shader);

    if (!prog) {
        g_critical("Unable to create shader program");
        return NULL;
    }

    psy_shader_program_set_vertex_shader_from_path(prog, vert_shader, &error);
    if (error) {
        g_critical("Unable to set shader path: %s", error->message);
        g_clear_error(&error);
        goto error;
    }
    psy_shader_program_set_fragment_shader_from_path(prog, frag_shader, &error);
    if (error) {
        g_critical("Unable to set shader path: %s", error->message);
        g_clear_error(&error);
        goto error;
    }

    psy_shader_program_link(prog, &error);
    if (error) {
        g_critical("Unable to link program: %s", error->message);
        g_clear_error(&error);
        goto error;
    }

    return prog;

error:
    g_clear_object(&prog);
    return prog;
}

void
render_loop(SDL_Window *win)
{
    int               vsync;
    int               running  = 1;
    GError           *error    = NULL;
    float             white[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    float             sw       = 250; // square_width and height
    PsyGlVBuffer     *vbuffer  = NULL;
    PsyShaderProgram *program  = NULL;

    FrameStats *stats = frame_stats_new();

    PsyClock           *clk     = psy_clock_new();
    PsyParallelTrigger *trigger = psy_parallel_trigger_new();

    FILE *outfile = fopen("sdl_vsync.txt", "wb");

    SDL_GLContext context = SDL_GL_CreateContext(win);
    SDL_GL_MakeCurrent(win, context);
    int w, h;
    SDL_GetWindowSizeInPixels(win, &w, &h);

    if (SDL_GetWindowSurfaceVSync(win, &vsync) == true) {
        g_info("Default vsync = %d", vsync);
    }
    else {
        if (SDL_GL_GetSwapInterval(&vsync))
            g_info("Default vsync = %d", vsync);
        else {
            g_critical("Oops unable to query window vsync :%s", SDL_GetError());
        }
    }

    psy_parallel_trigger_open(trigger, 0, &error);
    if (error) {
        g_critical("Unable to open trigger device: %s", error->message);
        g_clear_error(&error);
        g_clear_object(&trigger); // work without the trigger
    }

    glViewport(0, 0, w, h);

    PsyMatrix4 *projection = psy_matrix4_new_ortographic(
        -w / 2.0, w / 2.0f, -h / 2.0f, h / 2.0f, 0.0f, 100.0f);

    PsyMatrix4 *model  = psy_matrix4_new_identity();
    float       vec[3] = {-w / 2 + sw / 2, h / 2 - sw / 2, 0.f};
    PsyVector3 *v      = psy_vector3_new_data(3, vec);

    psy_matrix4_translate(model, v);
    psy_vector3_free(v);

    program = init_shaders();
    if (!program) {
        g_critical("Unable to create shader program");
        goto error;
    }
    psy_shader_program_use(program, &error);
    if (error) {
        g_critical("Unable to use program: %s\n", error->message);
        goto error;
    }
    psy_shader_program_set_uniform_4f(program, "ourColor", white, &error);
    if (error) {
        g_critical("Unable to set color: %s", error->message);
        goto error;
    }
    psy_shader_program_set_uniform_matrix4(
        program, "projection", projection, &error);
    if (error) {
        g_critical("Unable to set projection matrix: %s", error->message);
        goto error;
    }
    psy_shader_program_set_uniform_matrix4(program, "model", model, &error);
    if (error) {
        g_critical("Unable to set view matrix: %s", error->message);
        goto error;
    }
    psy_matrix4_free(projection);
    psy_matrix4_free(model);

    vbuffer = psy_gl_vbuffer_new();

    // clang-format off

    PsyVertex verts[] = {
        {
            .pos = {sw / 2.0f, sw / 2.0f, 0.0f},
        },
        {
            .pos = {-sw / 2.0f, sw/2.0f, 0.0f},
        },
        {
            .pos = {-sw / 2, -sw/2, 0},
        },
        {
            .pos = {sw / 2.0f, -sw / 2.0f, 0},
        },
    };

    // clang-format on
    psy_vbuffer_set_from_data(
        PSY_VBUFFER(vbuffer), verts, sizeof(verts) / sizeof(verts[0]));

    uint64_t start = SDL_GetTicks();

    while (true) { // wait a half of a second for the vsync to stabilize.
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        SDL_GL_SwapWindow(win);
        if ((SDL_GetTicks() - start) > 500)
            break;
    }

    PsyTimePoint *last = psy_clock_now(clk);

    while (running) {
        SDL_Event e;

        while (SDL_PollEvent(&e) == true) {
            if (e.type == SDL_EVENT_QUIT) {
                running = false;
                g_debug("Quit event encountered");
            }
            if (e.type == SDL_EVENT_WINDOW_RESIZED) {
                gint width  = e.display.data1;
                gint height = e.display.data2;
                g_debug("Setting viewport to 0 0 %d %d", width, height);

                glViewport(0, 0, width, height);
                // error handing on glViewPort
                assert(glGetError() == GL_NO_ERROR);

                PsyMatrix4 *p = psy_matrix4_new_ortographic(-width / 2.0f,
                                                            width / 2.0f,
                                                            -height / 2.0f,
                                                            height / 2.0f,
                                                            0.f,
                                                            100.f);
                psy_shader_program_set_uniform_matrix4(
                    program, "projection", p, &error);
                if (error) {
                    g_critical("Unable to set projection matrix: %s",
                               error->message);
                    g_clear_error(&error);
                }
                psy_matrix4_free(p);

                float vec[3]  = {-width / 2 + sw / 2, height / 2 - sw / 2, 0};
                PsyVector3 *v = psy_vector3_new_data(3, vec);

                PsyMatrix4 *m = psy_matrix4_new_identity();
                psy_matrix4_translate(m, v);
                psy_vector3_free(v);

                psy_shader_program_set_uniform_matrix4(
                    program, "model", m, &error);
                if (error) {
                    g_critical("Unable to set projection matrix: %s",
                               error->message);
                    g_clear_error(&error);
                }
                psy_matrix4_free(m);
            }
        }

        if (!running)
            break;

        // Drawing one frame

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        psy_vbuffer_draw_triangle_fan(PSY_VBUFFER(vbuffer), &error);
        if (error) {
            g_critical("Unable to draw triangle string: %s", error->message);
            goto error;
        }

        SDL_GL_SwapWindow(win);
        PsyTimePoint *temp = psy_clock_now(clk);
        PsyDuration  *dur  = psy_time_point_subtract(temp, last);

        // update frame stats
        int64_t n_frames = psy_duration_divide_rounded(dur, g_frame_dur);
        int64_t n_missed = n_frames > 0 ? n_frames - 1 : 0;
        g_debug("n_frames = %d, n_missed = %d", (int) n_frames, (int) n_missed);
        g_debug("g_frame_dur = %lf, dur = %lf",
                psy_duration_get_seconds(g_frame_dur),
                psy_duration_get_seconds(dur));

        update_frame_stats(
            stats, n_frames, n_missed, psy_time_point_add(temp, g_frame_dur));

        fprintf(outfile, "%lf\n", psy_duration_get_seconds(dur));
        psy_duration_free(dur);
        psy_time_point_free(last);
        last = temp;
    }

    fprintf(stdout,
            "n_frames = %" G_GINT64_FORMAT ", n_missed = %" G_GINT64_FORMAT
            "\n",
            stats->nth_frame,
            stats->n_missed_frames);

    psy_time_point_free(last);

error:

    if (outfile)
        fclose(outfile);

    if (error)
        g_clear_error(&error);

    if (stats)
        frame_stats_free(stats);

    if (vbuffer)
        psy_gl_vbuffer_free(vbuffer);

    if (program)
        g_object_unref(program);

    SDL_GL_DestroyContext(context);

    psy_clock_free(clk);
}

int
main(int argc, char **argv)
{
    int      ret          = EXIT_SUCCESS;
    SDL_Rect monitor_rect = {0};

    cmd_parse(&argc, &argv);
    const CmdOptions *opts = cmd_get_options();
    if (!opts) {
        ret = EXIT_FAILURE;
        goto opt_error;
    }

    SDL_Init(SDL_INIT_VIDEO);
    PsyInitializer *psy_init = g_object_new(
        PSY_TYPE_INITIALIZER, "gstreamer", FALSE, "portaudio", FALSE, NULL);

    SDL_Window *win = NULL;

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_PROFILE_CORE);

    win = SDL_CreateWindow("Psy test SDL_Window timing",
                           640,
                           480,
                           SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!win) {
        g_critical("Oops unable to create window: %s", SDL_GetError());
        ret = EXIT_FAILURE;
        goto error;
    }

    const SDL_DisplayMode *mode
        = get_display_mode_for_monitor(opts->nth_monitor, &monitor_rect);
    g_frame_dur = psy_duration_new(1.0f
                                   / ((double) mode->refresh_rate_numerator
                                      / mode->refresh_rate_denominator));

    g_info("Setting montitor to x=%d, y=%d", monitor_rect.x, monitor_rect.y);
    g_info("Framedur = %d/%d or %lfs",
           mode->refresh_rate_numerator,
           mode->refresh_rate_denominator,
           psy_duration_get_seconds(g_frame_dur));

    if (!SDL_SetWindowPosition(win, monitor_rect.x, monitor_rect.y)) {
        g_critical("Unable to set window position: %s", SDL_GetError());
    }

    if (opts->fullscreen) {
        if (!SDL_SetWindowFullscreen(win, true)) {
            g_critical("Unable to set window fullscreen");
        }
    }

    render_loop(win);

error:
    psy_duration_free(g_frame_dur);
    psy_initializer_free(psy_init);
    SDL_Quit();
opt_error:
    return ret;
}
