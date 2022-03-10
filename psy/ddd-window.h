
#ifndef DDD_WINDOW_H
#define DDD_WINDOW_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define DDD_TYPE_WINDOW ddd_window_get_type()
G_DECLARE_FINAL_TYPE(DddWindow, ddd_window, DDD, WINDOW, GtkWindow)


G_MODULE_EXPORT DddWindow*
ddd_window_new(void);

G_MODULE_EXPORT DddWindow*
ddd_window_new_for_monitor(guint monitor);

G_MODULE_EXPORT void
ddd_window_set_monitor(DddWindow* window, guint nth_window);

G_MODULE_EXPORT guint 
ddd_window_get_monitor(DddWindow* window);


G_END_DECLS


#endif
