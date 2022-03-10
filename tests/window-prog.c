
#include "psy-window.h"
#include <stdlib.h>

static gint         n_monitor       = 0;
static gboolean     all_monitors    = FALSE;

static GOptionEntry entries[] = {
    {"monitor-number", 'n', 0, G_OPTION_ARG_INT, &n_monitor, "The number of the desired monitor", "N"},
    {"all-monitors", 'a', 0, G_OPTION_ARG_NONE, &all_monitors, "Display window on all monitors", NULL},
    {NULL}
};


gboolean
on_delete (GtkWidget *widget, GdkEvent *event, gpointer data)
{
    (void) widget, (void) event;
    g_application_quit(G_APPLICATION(data));
    return FALSE;
}

static void
on_activate(GtkApplication* app, gpointer data)
{
    (void) data;
    PsyWindow* window = NULL;
    if (all_monitors) {
        GdkDisplay* display = gdk_display_get_default();
        guint num_monitors = g_list_model_get_n_items(gdk_display_get_monitors(display));
        for (guint n = 0; n < num_monitors; n++) {
            window = psy_window_new_for_monitor(n);
            gtk_application_add_window(app, GTK_WINDOW(window));
        }
    }
    else {
        window = psy_window_new_for_monitor(n_monitor);
        gtk_application_add_window(app, GTK_WINDOW(window));
    }


    gtk_widget_show(GTK_WIDGET(window));
}

int main(int argc, char**argv) {

    int ret = EXIT_SUCCESS;
    GtkApplication* app;

    GError *error = NULL;
    GOptionContext* context = g_option_context_new("");
    g_option_context_add_main_entries(context, entries, NULL); 

    if (!g_option_context_parse(context, &argc, &argv, &error)) {
        g_printerr("Unable to parse options: %s\n", error->message);
        ret = EXIT_FAILURE;
        goto fail;
    }

    app = gtk_application_new(NULL, G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);


    ret = g_application_run(G_APPLICATION(app), argc, argv);

fail:

    g_option_context_free(context);
    if (error)
        g_error_free(error);

    return ret;
}
