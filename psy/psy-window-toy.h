
#ifndef PSY_WINDOW_TOY_H
#define PSY_WINDOW_TOY_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define PSY_TYPE_WINDOW_TOY psy_window_toy_get_type()
G_DECLARE_FINAL_TYPE(PsyWindowToy, psy_window_toy, PSY, WINDOW_TOY, GtkWindow)


G_MODULE_EXPORT PsyWindowToy*
psy_window_toy_new(void);

G_MODULE_EXPORT PsyWindowToy*
psy_window_toy_new_for_monitor(guint monitor);

G_MODULE_EXPORT void
psy_window_toy_set_monitor(PsyWindowToy* window, guint nth_window);

G_MODULE_EXPORT guint 
psy_window_toy_get_monitor(PsyWindowToy* window);


G_END_DECLS


#endif
