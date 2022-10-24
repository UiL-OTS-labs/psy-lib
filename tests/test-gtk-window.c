
#include "psy-visual-stimulus.h"
#include <psy-duration.h>
#include <psy-time-point.h>
#include <psy-stimulus.h>
#include <psy-clock.h>
#include <psy-window.h>
#include <backend_gtk/psy-gtk-window.h>
#include <psy-circle.h>
#include <stdlib.h>

gboolean stop_loop(gpointer data) {
    GMainLoop* loop = data;
    g_main_loop_quit(loop);
    return G_SOURCE_REMOVE;
}

int main() {
    
    PsyClock* clk = psy_clock_new();
    PsyTimePoint* tp, *start;
    PsyDuration* dur = psy_duration_new_ms(500);

    GMainLoop*    loop = g_main_loop_new(NULL, FALSE);

    PsyGtkWindow* window = psy_gtk_window_new_for_monitor(1);

    PsyCircle* circle = psy_circle_new(window);

    g_timeout_add(250, stop_loop, loop);

    tp = psy_clock_now(clk);
    start = psy_time_point_add(tp, dur);

    psy_stimulus_play_for(PSY_STIMULUS(circle), start, dur);

    g_main_loop_run(loop);

    g_print("The width = %d mm and height = %d mm\n",
            psy_window_get_width_mm(PSY_WINDOW(window)),
            psy_window_get_height_mm(PSY_WINDOW(window))
            );

    g_main_loop_unref(loop);
    g_object_unref(window);

    return EXIT_SUCCESS;
}
