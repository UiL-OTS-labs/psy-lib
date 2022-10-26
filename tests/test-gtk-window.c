
#include "psy-visual-stimulus.h"
#include <psy-duration.h>
#include <psy-time-point.h>
#include <psy-stimulus.h>
#include <psy-clock.h>
#include <psy-window.h>
#include <backend_gtk/psy-gtk-window.h>
#include <psy-circle.h>
#include <stdlib.h>
#include <stdio.h>

PsyClock* clk; 
PsyTimePoint* g_tstart = NULL;
PsyTimePoint* g_tstop = NULL;

gboolean stop_loop(gpointer data) {
    GMainLoop* loop = data;
    g_main_loop_quit(loop);
    return G_SOURCE_REMOVE;
}

void update_circle(
        PsyVisualStimulus* stim,
        PsyTimePoint* tp,
        gint64 nth_frame,
        gpointer data)
{
    (void) data;
    (void) tp;
    PsyCircle* circle = PSY_CIRCLE(stim);
    gfloat radius = psy_circle_get_radius(circle);
    printf("Circle frame %ld radius = %f\n", nth_frame, radius);
    psy_circle_set_radius(circle, radius + 1);
}

void circle_started(PsyCircle* circle, PsyTimePoint* tstart, gpointer data)
{
    (void) circle;
    PsyTimePoint* tzero = data;
    PsyDuration* dur = psy_time_point_subtract(tstart, tzero);
    printf("Circle started after %lf seconds\n", psy_duration_get_seconds(dur));
    g_tstart = psy_time_point_new_copy(tstart);
    g_object_unref(dur);
}

void circle_stopped(PsyCircle* circle, PsyTimePoint* tstop, gpointer data)
{
    (void) circle;
    PsyTimePoint* tzero = data;
    g_tstop = psy_time_point_new_copy(tstop);
    PsyDuration* dur = psy_time_point_subtract(tstop, tzero);
    g_print("Circle stopped after %lf seconds\n", psy_duration_get_seconds(dur));
    g_object_unref(dur);
}


int main(void) {
    
    PsyTimePoint* tp, *start;
    clk = psy_clock_new();
    tp = psy_clock_now(clk);

    PsyDuration* start_dur = psy_duration_new_ms(500);
    PsyDuration* dur = psy_duration_new_ms(100);

    GMainLoop*    loop = g_main_loop_new(NULL, FALSE);

    PsyGtkWindow* window = psy_gtk_window_new_for_monitor(0);

    PsyCircle* circle = psy_circle_new(PSY_WINDOW(window));
    g_signal_connect(circle, "update", G_CALLBACK(update_circle), tp);
    g_signal_connect(circle, "started", G_CALLBACK(circle_started), tp);
    g_signal_connect(circle, "stopped", G_CALLBACK(circle_stopped), tp);

    g_timeout_add(1000, stop_loop, loop);

    start = psy_time_point_add(tp, start_dur);

    psy_stimulus_play_for(PSY_STIMULUS(circle), start, dur);

    g_main_loop_run(loop);

    PsyDuration* diff = psy_time_point_subtract(g_tstop, g_tstart);

    g_print("The width = %d mm and height = %d mm\n",
            psy_window_get_width_mm(PSY_WINDOW(window)),
            psy_window_get_height_mm(PSY_WINDOW(window))
            );

    g_print("circle->num_frames = %ld\n",
            psy_visual_stimulus_get_num_frames(PSY_VISUAL_STIMULUS(circle)));

    g_print("Differnence between start and stop = %lf\n", psy_duration_get_seconds(diff));

    g_main_loop_unref(loop);
    g_object_unref(window);
    g_object_unref(clk);
    g_object_unref(dur);

    return EXIT_SUCCESS;
}
