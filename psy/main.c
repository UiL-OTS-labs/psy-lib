
#include <gtk/gtk.h>
#include "ddd-widget.h"

static void
activate(GtkApplication* app, gpointer data)
{
    (void) data;
    DddWidget* dwidget;
    GtkWidget* window;


    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "3D app");
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 300);

    dwidget = ddd_widget_new();

    gtk_window_set_child(GTK_WINDOW(window), GTK_WIDGET(dwidget));

    gtk_widget_show(window);
}


int main(int argc, char** argv) {
    GtkApplication *app;

    app = gtk_application_new("org.gtk.example", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);

    return status;
}
