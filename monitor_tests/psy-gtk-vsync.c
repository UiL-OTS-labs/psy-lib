
#define G_LOG_DOMAIN "VSync"

#include <psylib.h>

#include "cmd_vsync_opts.h"

typedef struct PsyGtkContext {
    GMainLoop    *loop;
    PsyCanvas    *win;
    PsyRectangle *rect;
    PsyDuration  *isi_dur;
    PsyDuration  *stim_dur;
    PsyClock     *clock;
} PsyGtkContext;

static PsyGtkContext g_context = {0};

void
print_dur_since_start(const gchar *msg, PsyTimePoint *tp)
{
    PsyTimePoint *tp_zero = psy_time_point_new();
    PsyDuration  *t_diff  = psy_time_point_subtract(tp, tp_zero);
    psy_time_point_free(tp_zero);
    g_print("%ld ms, %s\n", psy_duration_get_ms(t_diff), msg);
    psy_duration_free(t_diff);
}

void
on_rect_stopped(PsyStimulus *rect, PsyTimePoint *tp_stop, gpointer data)
{
    PsyGtkContext *context = data;
    PsyTimePoint  *next    = psy_time_point_add(tp_stop, context->isi_dur);

    print_dur_since_start("stop event", tp_stop);
    print_dur_since_start("next stim start", next);

    PsyTimePoint *now  = psy_clock_now(context->clock);
    PsyDuration  *diff = psy_time_point_subtract(now, tp_stop);
    g_info("on_stopped diff = %lf", psy_duration_get_seconds(diff));
    psy_stimulus_play(rect, next);
    psy_time_point_free(now);
    psy_time_point_free(next);
    psy_duration_free(diff);
}

int
main(int argc, char **argv)
{
    gint        win_width, win_height;
    const float rect_width = 250, rect_height = 250;

    PsyInitializer *init = g_object_new(
        PSY_TYPE_INITIALIZER, "gstreamer", FALSE, "portaudio", FALSE, NULL);

    if (!cmd_vsync_parse(&argc, &argv))
        goto error;

    const CmdVSyncOptions *opts = cmd_vsync_get_options();

    g_context.loop = g_main_loop_new(NULL, false);

    g_context.isi_dur  = psy_duration_new(opts->isi_dur);
    g_context.stim_dur = psy_duration_new(opts->stim_dur);
    g_context.clock    = psy_clock_new();

    // create window
    g_context.win
        = PSY_CANVAS(psy_gtk_window_new_for_monitor(opts->nth_monitor));
    PsyColor *back_ground_color = psy_color_new();
    PsyColor *stim_color        = psy_color_new_rgb(1, 1, 1);
    psy_canvas_set_background_color(g_context.win, back_ground_color);
    psy_color_free(back_ground_color);

    win_width  = psy_canvas_get_width(g_context.win);
    win_height = psy_canvas_get_height(g_context.win);

    g_context.rect
        = psy_rectangle_new_full(g_context.win,
                                 (float) -win_width / 2.0f + rect_width / 2.0f,
                                 (float) win_height / 2.0f - rect_height / 2.0f,
                                 rect_width,
                                 rect_height);
    psy_visual_stimulus_set_color(PSY_VISUAL_STIMULUS(g_context.rect),
                                  stim_color);
    psy_color_free(stim_color);

    PsyTimePoint *now     = psy_time_point_new(); // start of clock.
    PsyDuration  *one_sec = psy_duration_new_s(1);
    PsyTimePoint *start   = psy_time_point_add(now, one_sec);
    print_dur_since_start("Requested start time", start);

    psy_stimulus_play_for(
        PSY_STIMULUS(g_context.rect), start, g_context.stim_dur);

    g_signal_connect(
        g_context.rect, "stopped", G_CALLBACK(on_rect_stopped), &g_context);

    g_main_loop_run(g_context.loop);

error:
    psy_initializer_free(init);
}
