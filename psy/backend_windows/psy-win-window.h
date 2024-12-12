
#ifndef PSY_WIN_WINDOW_H
#define PSY_WIN_WINDOW_H

#include <psy-window.h>

G_BEGIN_DECLS

#define PSY_TYPE_WIN_WINDOW psy_win_window_get_type()

G_MODULE_EXPORT
G_DECLARE_FINAL_TYPE(PsyWinWindow, psy_win_window, PSY, WIN_WINDOW, PsyWindow)

G_MODULE_EXPORT PsyWinWindow *
psy_win_window_new(void);

G_MODULE_EXPORT PsyWinWindow *
psy_win_window_new_for_monitor(gint monitor);

G_MODULE_EXPORT void
psy_win_window_free(PsyWinWindow *self);

G_END_DECLS

#endif
