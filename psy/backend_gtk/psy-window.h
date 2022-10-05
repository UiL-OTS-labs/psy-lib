
#ifndef PSY_WINDOW_H
#define PSY_WINDOW_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define PSY_TYPE_WINDOW psy_window_get_type()
G_DECLARE_FINAL_TYPE(PsyWindow, psy_window, PSY, WINDOW, GtkWindow)


G_MODULE_EXPORT PsyWindow*
psy_window_new(void);

G_MODULE_EXPORT PsyWindow*
psy_window_new_for_monitor(guint monitor);

G_MODULE_EXPORT void
psy_window_set_monitor(PsyWindow* window, guint nth_window);

G_MODULE_EXPORT guint 
psy_window_get_monitor(PsyWindow* window);


G_END_DECLS


#endif
