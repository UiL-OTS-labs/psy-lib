
#include <psylib.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_opengl.h>

#include "cmd_vsync_opts.h"

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

void
render_loop(SDL_Window *win)
{
    int running = 1;

    SDL_GLContext context = SDL_GL_CreateContext(win);
    SDL_GL_MakeCurrent(win, context);

    PsyClock     *clk  = psy_clock_new();
    PsyTimePoint *last = psy_clock_now(clk);

    while (running) {
        SDL_Event e;

        while (SDL_PollEvent(&e) == true) {
            if (e.type == SDL_EVENT_QUIT) {
                running = false;
            }
        }

        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        SDL_GL_SwapWindow(win);
        PsyTimePoint *temp = psy_clock_now(clk);
        PsyDuration  *dur  = psy_time_point_subtract(temp, last);
        // g_print("Time since last swap = %lfs\n",
        // psy_duration_get_seconds(dur));
        psy_duration_free(dur);
        psy_time_point_free(last);
        last = temp;
    }

    SDL_GL_DestroyContext(context);

    psy_clock_free(clk);
}

int
main(int argc, char **argv)
{
    int      ret = EXIT_SUCCESS;
    SDL_Rect monitor_rect;

    cmd_parse(&argc, &argv);
    const CmdOptions *opts = cmd_get_options();

    SDL_Init(SDL_INIT_VIDEO);
    PsyInitializer *psy_init = g_object_new(
        PSY_TYPE_INITIALIZER, "gstreamer", FALSE, "portaudio", FALSE, NULL);

    SDL_Window *win = NULL;

    win = SDL_CreateWindow(
        "Psy test SDL_Window timing", 640, 480, SDL_WINDOW_OPENGL);
    if (!win) {
        g_critical("Oops unable to create window: %s", SDL_GetError());
        ret = EXIT_FAILURE;
        goto error;
    }

    const SDL_DisplayMode *mode
        = get_display_mode_for_monitor(opts->nth_monitor, &monitor_rect);

    g_info("Setting montitor to x=%d, y=%d", monitor_rect.x, monitor_rect.y);

    if (!SDL_SetWindowPosition(win, monitor_rect.x, monitor_rect.y)) {
        g_critical("Unable to set window position: %s", SDL_GetError());
    }

    if (!SDL_SetWindowFullscreen(win, true)) {
        g_critical("Unable to set window fullscreen");
    }

    //    if (!SDL_SetWindowFullscreenMode(win, mode)) {
    //        g_critical("Unable to set window fullscreen: %s", SDL_GetError());
    //    }
    if (!SDL_ShowWindow(win)) {
        g_critical("Unable to show window: %s", SDL_GetError());
    }

    render_loop(win);

error:
    psy_initializer_free(psy_init);
    SDL_Quit();
    return ret;
}
