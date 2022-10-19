
#include "psy-window.h"
#include <backend_gtk/psy-gtk-window.h>
#include <stdlib.h>

gboolean stop_loop(gpointer data) {
    GMainLoop* loop = data;
    g_main_loop_quit(loop);
    return G_SOURCE_REMOVE;
}

int main() {

    GMainLoop*    loop = g_main_loop_new(NULL, FALSE);

    PsyGtkWindow* window = psy_gtk_window_new_for_monitor(1);

    g_timeout_add_seconds(1, stop_loop, loop);
    g_main_loop_run(loop);

    g_print("The width = %d mm and height = %d mm\n",
            psy_window_get_width_mm(PSY_WINDOW(window)),
            psy_window_get_height_mm(PSY_WINDOW(window))
            );

    g_main_loop_unref(loop);
    g_object_unref(window);

    return EXIT_SUCCESS;
}
