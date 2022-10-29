
#include "psy-visual-stimulus.h"
#include <psy-duration.h>
#include <psy-time-point.h>
#include <psy-stimulus.h>
#include <psy-clock.h>
#include <psy-window.h>
#include <backend_gtk/psy-gtk-window.h>
#include <psy-circle.h>
#include <stdlib.h>
#include <math.h>

// Globals
PsyClock* clk; 
PsyTimePoint* g_tstart = NULL;
PsyTimePoint* g_tstop = NULL;

// related to option parsing
gint n_monitor;
gdouble g_duration = 4.0f;
int g_nvertices = 10;
gdouble g_radius = 0.5;
gdouble g_amplitude = 0.25;
gdouble g_frequency = 0.5;

static GOptionEntry entries[] = {
    {"monitor-number", 'n', G_OPTION_FLAG_NONE, G_OPTION_ARG_INT, &n_monitor, "The number of the desired monitor", "N"},
    {"duration", 'd', G_OPTION_FLAG_NONE, G_OPTION_ARG_DOUBLE, &g_duration, "The duration of the stimulus in seconds", "seconds"},
    {"radius", 'r', G_OPTION_FLAG_NONE, G_OPTION_ARG_DOUBLE, &g_radius, "The base radius of the circle", "units"},
    {"amplitude", 'a', G_OPTION_FLAG_NONE, G_OPTION_ARG_DOUBLE, &g_amplitude, "The base amplitude of the pulsing of the circle", "units"},
    {"frequency", 'f', G_OPTION_FLAG_NONE, G_OPTION_ARG_DOUBLE, &g_frequency, "The base frequency of the pulsing of the circle", "seconds"},
    {"vertices", 'v', G_OPTION_FLAG_NONE, G_OPTION_ARG_INT, &g_nvertices,   "The number of vertices of the circle", "number"},
    {0}
};


void
update_circle(
        PsyVisualStimulus* stim,
        PsyTimePoint* tp,
        gint64 nth_frame,
        gpointer data)
{
    (void) data;
    (void) tp;
    (void) nth_frame;
    PsyCircle* circle = PSY_CIRCLE(stim);
    gfloat radius;
    PsyDuration* dur;
    if (g_tstart)
        dur = psy_time_point_subtract(tp, g_tstart);
    else 
        dur = psy_time_point_subtract(tp, tp);

    radius = g_radius + sin(
            psy_duration_get_seconds(dur) * g_frequency * 2 * M_PI
            ) * g_amplitude;

    psy_circle_set_radius(circle, radius);
}

void
circle_started(PsyCircle* circle, PsyTimePoint* tstart, gpointer data)
{
    (void) circle;
    PsyTimePoint* tzero = data;
    PsyDuration* dur = psy_time_point_subtract(tstart, tzero);
    g_print("Circle started after %lf seconds\n", psy_duration_get_seconds(dur));
    g_tstart = psy_time_point_new_copy(tstart);
    g_object_unref(dur);
}

void
circle_stopped(PsyCircle* circle, PsyTimePoint* tstop, gpointer data)
{
    (void) circle;
    PsyTimePoint* tzero = data;
    g_tstop = psy_time_point_new_copy(tstop);
    PsyDuration* dur = psy_time_point_subtract(tstop, tzero);
    g_print("Circle stopped after %lf seconds\n", psy_duration_get_seconds(dur));
    g_object_unref(dur);
}

void
stop_loop(PsyCircle* circle, PsyTimePoint* tp, gpointer data)
{
    (void) circle;
    (void) tp;
    GMainLoop* loop = data;
    g_main_loop_quit(loop);
}

int main(int argc, char**argv) {

    int ret = EXIT_SUCCESS;
    PsyTimePoint* tp, *start;
    GError* error = NULL;
    
    GOptionContext* context = g_option_context_new("");
    g_option_context_add_main_entries(context, entries, NULL); 

    if (!g_option_context_parse(context, &argc, &argv, &error)) {
        g_printerr("Unable to parse options: %s\n", error->message);
        g_option_context_free(context);
        return EXIT_FAILURE;
    }
    
    clk = psy_clock_new();
    tp = psy_clock_now(clk);

    PsyDuration* start_dur = psy_duration_new_ms(500);
    PsyDuration* dur = psy_duration_new(g_duration);

    GMainLoop*    loop = g_main_loop_new(NULL, FALSE);

    PsyGtkWindow* window = psy_gtk_window_new_for_monitor(n_monitor);

    PsyCircle* circle = psy_circle_new_full(PSY_WINDOW(window), 0, 0, .5, g_nvertices);
    g_signal_connect(circle, "update", G_CALLBACK(update_circle), tp);
    g_signal_connect(circle, "started", G_CALLBACK(circle_started), tp);
    g_signal_connect(circle, "stopped", G_CALLBACK(circle_stopped), tp);
    g_signal_connect(circle, "stopped", G_CALLBACK(stop_loop), loop); // stop the loop

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

    g_print("Difference between start and stop = %lf\n", psy_duration_get_seconds(diff));


    g_main_loop_unref(loop);
    g_object_unref(window);
    g_object_unref(clk);
    g_object_unref(dur);

    return ret;
}
