
#include "psy-visual-stimulus.h"
#include <psy-duration.h>
#include <psy-time-point.h>
#include <psy-stimulus.h>
#include <psy-clock.h>
#include <psy-window.h>
#include <psy-color.h>
#include <backend_gtk/psy-gtk-window.h>
#include <psy-circle.h>
#include <psy-cross.h>
#include <stdlib.h>
#include <math.h>

// Global variable
PsyClock* clk; 
PsyTimePoint* g_tstart = NULL;
PsyTimePoint* g_tstop = NULL;

// related to option parsing
gint n_monitor;
gdouble g_duration = 4.0f;
int g_nvertices = 10;
gdouble g_radius = 50;
gdouble g_amplitude = 25;
gdouble g_frequency = 0.5;
gdouble g_x = 0.0;
gdouble g_y = 0.0;
gdouble g_z = 0.0;

char* g_origin = "center";
char* g_units = "pixels";
gboolean circle_first = FALSE;

static GOptionEntry entries[] = {
    {"monitor-number", 'n', G_OPTION_FLAG_NONE, G_OPTION_ARG_INT, &n_monitor, "The number of the desired monitor", "N"},
    {"duration", 'd', G_OPTION_FLAG_NONE, G_OPTION_ARG_DOUBLE, &g_duration, "The duration of the stimulus in seconds", "seconds"},
    {"radius", 'r', G_OPTION_FLAG_NONE, G_OPTION_ARG_DOUBLE, &g_radius, "The base radius of the circle", "units"},
    {"amplitude", 'a', G_OPTION_FLAG_NONE, G_OPTION_ARG_DOUBLE, &g_amplitude, "The base amplitude of the pulsing of the circle", "units"},
    {"frequency", 'f', G_OPTION_FLAG_NONE, G_OPTION_ARG_DOUBLE, &g_frequency, "The base frequency of the pulsing of the circle", "seconds"},
    {"vertices", 'v', G_OPTION_FLAG_NONE, G_OPTION_ARG_INT, &g_nvertices, "The number of vertices of the circle", "number"},
    {"origin", 'o', G_OPTION_FLAG_NONE, G_OPTION_ARG_STRING, &g_origin, "The center C style or center", "c|center"},
    {"units", 'u', G_OPTION_FLAG_NONE, G_OPTION_ARG_STRING, &g_units, "The units of the coordinate system", "pix|m|mm|visdeg"},
    {"x", 'x', G_OPTION_FLAG_NONE, G_OPTION_ARG_DOUBLE, &g_x, "The x-coordinate of the circle", "units depends on projection"},
    {"y", 'y', G_OPTION_FLAG_NONE, G_OPTION_ARG_DOUBLE, &g_y, "The y-coordinate of the circle", "units depends on projection"},
    {"z", 'z', G_OPTION_FLAG_NONE, G_OPTION_ARG_DOUBLE, &g_z, "The z-coordinate of the circle", "units depends on projection"},
    {"circle-first", 'c', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &circle_first, "Whether or not to present the circle first", NULL},
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
    PsyDuration* dur = NULL;
    if (g_tstart)
        dur = psy_time_point_subtract(tp, g_tstart);
    else 
        dur = psy_time_point_subtract(tp, tp);

    radius = g_radius + sin(
            psy_duration_get_seconds(dur) * g_frequency * 2 * M_PI
            ) * g_amplitude;

    psy_circle_set_radius(circle, radius);
    g_object_unref(dur);
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

gint get_window_style(void)
{
    const char* center = "center";
    const char* c = "c";

    const char* pixels = "pixels";
    const char* m = "m";
    const char* mm = "mm";
    const char* visdeg = "visual degrees";

    gint style = 0;
    if (g_strcmp0(g_origin, center) == 0)
        style |= PSY_WINDOW_PROJECTION_STYLE_CENTER;
    else if (g_strcmp0(g_origin, c) == 0)
        style |= PSY_WINDOW_PROJECTION_STYLE_C;
    else {
        g_warning("The origin wasn't %s nor %s, defaulting to %s",
                center, c, center);
        style |= PSY_WINDOW_PROJECTION_STYLE_CENTER;
    }
    
    if (g_strcmp0(g_units, pixels) == 0)
        style |= PSY_WINDOW_PROJECTION_STYLE_PIXELS;
    else if (g_strcmp0(g_units, m) == 0)
        style |= PSY_WINDOW_PROJECTION_STYLE_METER;
    else if (g_strcmp0(g_units, mm) == 0)
        style |= PSY_WINDOW_PROJECTION_STYLE_MILLIMETER;
    else if (g_strcmp0(g_units, visdeg) == 0)
        style |= PSY_WINDOW_PROJECTION_STYLE_VISUAL_DEGREES;
    else {
        g_warning("The units wasn't one of: %s, %s or %s, %s, defaulting to %s",
                pixels, m, mm, visdeg, pixels
                );
        style |= PSY_WINDOW_PROJECTION_STYLE_PIXELS;
    }

    return style;
}

int main(int argc, char**argv) {

    int ret = EXIT_SUCCESS;
    PsyTimePoint* tp, *start;
    GError* error = NULL;
    gint window_style;
    PsyColor* circle_color = psy_color_new_rgb(1.0, 0, 0);
    PsyColor* cross_color = psy_color_new_rgb(1.0, 1.0, 0);
    
    GOptionContext* context = g_option_context_new("");
    g_option_context_add_main_entries(context, entries, NULL); 

    if (!g_option_context_parse(context, &argc, &argv, &error)) {
        g_printerr("Unable to parse options: %s\n", error->message);
        g_option_context_free(context);
        return EXIT_FAILURE;
    }
    g_option_context_free(context);
    
    clk = psy_clock_new();
    tp = psy_clock_now(clk);

    PsyDuration* start_dur = psy_duration_new_ms(500);
    PsyDuration* dur = psy_duration_new(g_duration);

    GMainLoop*    loop = g_main_loop_new(NULL, FALSE);

    PsyGtkWindow* window = psy_gtk_window_new_for_monitor(n_monitor);

    window_style = get_window_style();
    psy_window_set_projection_style(PSY_WINDOW(window), window_style);

    PsyCircle* circle = psy_circle_new_full(
            PSY_WINDOW(window), g_x, g_y, g_radius, g_nvertices
            );
    PsyCross* cross = psy_cross_new_full(PSY_WINDOW(window), 0, 0, 200, 10);
    
    psy_visual_stimulus_set_color(PSY_VISUAL_STIMULUS(circle), circle_color);
    g_object_set(cross,
           "color", cross_color,
           NULL);

    g_print("Circle @ %p,\tCross @ %p\n", circle, cross);
    g_signal_connect(circle, "update", G_CALLBACK(update_circle), tp);
    g_signal_connect(circle, "started", G_CALLBACK(circle_started), tp);
    g_signal_connect(circle, "stopped", G_CALLBACK(circle_stopped), tp);
    g_signal_connect(circle, "stopped", G_CALLBACK(stop_loop), loop); // stop the loop

    start = psy_time_point_add(tp, start_dur);

    if (circle_first) {
        psy_stimulus_play_for(PSY_STIMULUS(circle), start, dur);
        psy_stimulus_play_for(PSY_STIMULUS(cross), start, dur);
    }
    else {
        psy_stimulus_play_for(PSY_STIMULUS(cross), start, dur);
        psy_stimulus_play_for(PSY_STIMULUS(circle), start, dur);
    }

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
    g_object_unref(start_dur);
    g_object_unref(diff);
    g_object_unref(start);

    return ret;
}
