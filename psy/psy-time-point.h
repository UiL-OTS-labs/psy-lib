
#pragma once

#include <gio/gio.h>
#include <glib-object.h>

#include "psy-duration.h"

G_BEGIN_DECLS

typedef struct PsyTimePoint PsyTimePoint;

struct PsyTimePoint {
    /* <private> */
    gint64 ticks_since_start;
};

#define PSY_TYPE_TIME_POINT psy_time_point_get_type()

G_MODULE_EXPORT GType
psy_time_point_get_type(void);

G_MODULE_EXPORT PsyTimePoint *
psy_time_point_new(void);

G_MODULE_EXPORT PsyTimePoint *
psy_time_point_new_monotonic(gint64 monotonic_time);

G_MODULE_EXPORT void
psy_time_point_free(PsyTimePoint *self);

G_MODULE_EXPORT PsyTimePoint *
psy_time_point_copy(PsyTimePoint *self);

G_MODULE_EXPORT PsyTimePoint *
psy_time_point_add(PsyTimePoint *self, PsyDuration *dur);

G_MODULE_EXPORT PsyDuration *
psy_time_point_subtract(PsyTimePoint *self, PsyTimePoint *other);

G_MODULE_EXPORT PsyTimePoint *
psy_time_point_subtract_dur(PsyTimePoint *self, PsyDuration *dur);

G_MODULE_EXPORT PsyDuration *
psy_time_point_duration_since_start(PsyTimePoint *self);

G_MODULE_EXPORT gboolean
psy_time_point_less(PsyTimePoint *self, PsyTimePoint *other);

G_MODULE_EXPORT gboolean
psy_time_point_less_equal(PsyTimePoint *self, PsyTimePoint *other);

G_MODULE_EXPORT gboolean
psy_time_point_equal(PsyTimePoint *self, PsyTimePoint *other);

G_MODULE_EXPORT gboolean
psy_time_point_not_equal(PsyTimePoint *self, PsyTimePoint *other);

G_MODULE_EXPORT gboolean
psy_time_point_greater_equal(PsyTimePoint *self, PsyTimePoint *other);

G_MODULE_EXPORT gboolean
psy_time_point_greater(PsyTimePoint *self, PsyTimePoint *other);

G_END_DECLS
