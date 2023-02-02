
#pragma once

#include <gio/gio.h>
#include <glib-object.h>

#include "psy-duration.h"

G_BEGIN_DECLS

#define PSY_TYPE_TIME_POINT psy_time_point_get_type()
G_DECLARE_FINAL_TYPE(PsyTimePoint, psy_time_point, PSY, TIME_POINT, GObject)

G_MODULE_EXPORT PsyTimePoint *
psy_time_point_new_copy(PsyTimePoint *other);

G_MODULE_EXPORT PsyTimePoint *
psy_time_point_new(gint64 monotonic_time);

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
