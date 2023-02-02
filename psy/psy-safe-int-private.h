
#pragma once

#include <gio/gio.h>
#include <glib-object.h>

G_BEGIN_DECLS

gboolean
psy_safe_add_gint64(gint64 a, gint64 b, gint64 *result);
gboolean
psy_safe_sub_gint64(gint64 a, gint64 b, gint64 *result);
gboolean
psy_safe_mul_gint64(gint64 a, gint64 b, gint64 *result);

G_END_DECLS
