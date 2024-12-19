
#pragma once

#include <glib.h>

G_BEGIN_DECLS

GPtrArray *
psy_win_window_enumerate_adapters(void);

GPtrArray *
psy_win_window_enumerate_outputs(void);

GPtrArray *
psy_win_window_enumerate_displays(void);

G_END_DECLS