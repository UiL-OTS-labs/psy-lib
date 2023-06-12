
#pragma once

#include <gio/gio.h>

G_MODULE_EXPORT void
psy_enumerate_font_families(gchar ***families, gint *n);
