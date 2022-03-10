
#pragma once

#include <glib-object.h>
#include <gio/gio.h>

G_BEGIN_DECLS

#define DDD_TYPE_DURATION ddd_duration_get_type()
G_DECLARE_FINAL_TYPE(DddDuration, ddd_duration, DDD, DURATION, GObject)

G_MODULE_EXPORT DddDuration*
ddd_duration_new(gdouble seconds);

G_MODULE_EXPORT DddDuration*
ddd_duration_new_us(gint64 us);

G_MODULE_EXPORT DddDuration*
ddd_duration_new_ms(gint64 ms);

G_MODULE_EXPORT DddDuration*
ddd_duration_new_s(gint64 s);

G_MODULE_EXPORT gint64
ddd_duration_get_us(DddDuration* self);

G_MODULE_EXPORT gint64
ddd_duration_get_ms(DddDuration* self);

G_MODULE_EXPORT gint64
ddd_duration_get_s(DddDuration* self);

G_MODULE_EXPORT gdouble
ddd_duration_get_seconds(DddDuration* self);

G_MODULE_EXPORT gint64
ddd_duration_divide(DddDuration* self, DddDuration* other);

G_MODULE_EXPORT DddDuration*
ddd_duration_divide_scalar(DddDuration* self, gint64 scalar);

G_MODULE_EXPORT DddDuration*
ddd_duration_multiply_scalar(DddDuration* self, gint64 scalar);

G_MODULE_EXPORT DddDuration*
ddd_duration_add(DddDuration* self, DddDuration* other);

G_MODULE_EXPORT DddDuration*
ddd_duration_subtract(DddDuration* self, DddDuration* other);

G_MODULE_EXPORT gboolean
ddd_duration_less(DddDuration* self, DddDuration* other);

G_MODULE_EXPORT gboolean
ddd_duration_less_equal(DddDuration* self, DddDuration* other);

G_MODULE_EXPORT gboolean
ddd_duration_equal(DddDuration* self, DddDuration* other);

G_MODULE_EXPORT gboolean
ddd_duration_not_equal(DddDuration* self, DddDuration* other);

G_MODULE_EXPORT gboolean
ddd_duration_greater_equal(DddDuration* self, DddDuration* other);

G_MODULE_EXPORT gboolean
ddd_duration_greater(DddDuration* self, DddDuration* other);

G_END_DECLS
