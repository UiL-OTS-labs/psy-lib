
#pragma once

#include <gio/gio.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define PSY_TYPE_DURATION psy_duration_get_type()

/* Implementation of PsyDuration is considered private */

typedef struct PsyDuration PsyDuration;

G_MODULE_EXPORT GType
psy_duration_get_type(void);

G_MODULE_EXPORT PsyDuration *
psy_duration_new(gdouble seconds);

G_MODULE_EXPORT void
psy_duration_free(PsyDuration *self);

G_MODULE_EXPORT PsyDuration *
psy_duration_new_us(gint64 us);

G_MODULE_EXPORT PsyDuration *
psy_duration_new_ms(gint64 ms);

G_MODULE_EXPORT PsyDuration *
psy_duration_new_s(gint64 s);

G_MODULE_EXPORT PsyDuration *
psy_duration_copy(PsyDuration *self);

G_MODULE_EXPORT gint64
psy_duration_get_us(PsyDuration *self);

G_MODULE_EXPORT gint64
psy_duration_get_ms(PsyDuration *self);

G_MODULE_EXPORT gint64
psy_duration_get_s(PsyDuration *self);

G_MODULE_EXPORT gdouble
psy_duration_get_seconds(PsyDuration *self);

G_MODULE_EXPORT gint64
psy_duration_divide(PsyDuration *self, PsyDuration *other);

G_MODULE_EXPORT gint64
psy_duration_divide_rounded(PsyDuration *self, PsyDuration *other);

G_MODULE_EXPORT PsyDuration *
psy_duration_divide_scalar(PsyDuration *self, gint64 scalar);

G_MODULE_EXPORT PsyDuration *
psy_duration_multiply_scalar(PsyDuration *self, gint64 scalar);

G_MODULE_EXPORT PsyDuration *
psy_duration_add(PsyDuration *self, PsyDuration *other);

G_MODULE_EXPORT PsyDuration *
psy_duration_subtract(PsyDuration *self, PsyDuration *other);

G_MODULE_EXPORT gboolean
psy_duration_less(PsyDuration *self, PsyDuration *other);

G_MODULE_EXPORT gboolean
psy_duration_less_equal(PsyDuration *self, PsyDuration *other);

G_MODULE_EXPORT gboolean
psy_duration_equal(PsyDuration *self, PsyDuration *other);

G_MODULE_EXPORT gboolean
psy_duration_not_equal(PsyDuration *self, PsyDuration *other);

G_MODULE_EXPORT gboolean
psy_duration_greater_equal(PsyDuration *self, PsyDuration *other);

G_MODULE_EXPORT gboolean
psy_duration_greater(PsyDuration *self, PsyDuration *other);

G_END_DECLS
