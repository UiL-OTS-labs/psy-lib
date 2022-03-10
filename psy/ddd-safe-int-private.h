
#pragma once

#include <glib-object.h>
#include <gio/gio.h>

G_BEGIN_DECLS

gboolean ddd_safe_add_gint64(gint64 a, gint64 b, gint64* result);
gboolean ddd_safe_sub_gint64(gint64 a, gint64 b, gint64* result);
gboolean ddd_safe_mul_gint64(gint64 a, gint64 b, gint64* result);

G_END_DECLS
