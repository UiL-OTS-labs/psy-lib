
#ifndef PSY_GTK_WINDOW_H
#define PSY_GTK_WINDOW_H

#include <psy-window.h>

G_BEGIN_DECLS

#define PSY_TYPE_GTK_WINDOW psy_gtk_window_get_type()
G_DECLARE_FINAL_TYPE(
        PsyGtkWindow, psy_gtk_window, PSY, GTK_WINDOW, PsyWindow
        )

G_MODULE_EXPORT PsyGtkWindow*
psy_gtk_window_new(void);

G_MODULE_EXPORT PsyGtkWindow*
psy_gtk_window_new_for_monitor(gint monitor);

G_END_DECLS


#endif
