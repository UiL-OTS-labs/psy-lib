
#pragma once

#include <glib-object.h>
#include <gio/gio.h>

#include "ddd-duration.h"

G_BEGIN_DECLS

#define DDD_TYPE_TIME_POINT ddd_time_point_get_type()
G_DECLARE_FINAL_TYPE(DddTimePoint, ddd_time_point, DDD, TIME_POINT, GObject)

G_MODULE_EXPORT DddTimePoint*
ddd_time_point_add(DddTimePoint* self, DddDuration* dur);

G_MODULE_EXPORT DddDuration*
ddd_time_point_subtract(DddTimePoint* self, DddTimePoint* other);

G_MODULE_EXPORT DddTimePoint*
ddd_time_point_subtract_dur(DddTimePoint* self, DddDuration* dur);

G_MODULE_EXPORT DddDuration*
ddd_time_point_duration_since_start(DddTimePoint* self);

G_MODULE_EXPORT gboolean
ddd_time_point_less(DddTimePoint* self, DddTimePoint* other);

G_MODULE_EXPORT gboolean
ddd_time_point_less_equal(DddTimePoint* self, DddTimePoint* other);

G_MODULE_EXPORT gboolean
ddd_time_point_equal(DddTimePoint* self, DddTimePoint* other);

G_MODULE_EXPORT gboolean
ddd_time_point_not_equal(DddTimePoint* self, DddTimePoint* other);

G_MODULE_EXPORT gboolean
ddd_time_point_greater_equal(DddTimePoint* self, DddTimePoint* other);

G_MODULE_EXPORT gboolean
ddd_time_point_greater(DddTimePoint* self, DddTimePoint* other);

G_END_DECLS
