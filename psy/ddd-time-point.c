
#include "ddd-time-point.h"
#include "ddd-duration.h"
#include "ddd-safe-int-private.h"

typedef struct _DddTimePoint {
    GObject parent;
    gint64  ticks_since_start;
} DddTimePoint;

typedef enum {
    PROP_NULL,
    PROP_NUM_TICKS,
    NUM_PROPERTIES
} DddTimePointProperty;

G_DEFINE_TYPE(
        DddTimePoint,
        ddd_time_point,
        G_TYPE_OBJECT
        )

static GParamSpec* obj_properties[NUM_PROPERTIES];

static void
ddd_time_point_set_property(
        GObject      *object,
        guint         property_id,
        const GValue *value,
        GParamSpec   *pspec
        )
{
    DddTimePoint *tp = DDD_TIME_POINT(object);
    switch ((DddTimePointProperty) property_id) {
        case PROP_NUM_TICKS:
            tp->ticks_since_start = g_value_get_int64(value);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

static void
ddd_time_point_get_property(
        GObject    *object,
        guint       property_id,
        GValue     *value,
        GParamSpec *pspec
        )
{
    DddTimePoint* self = DDD_TIME_POINT(object);
    switch ((DddTimePointProperty) property_id) {
        case PROP_NUM_TICKS:
            g_value_set_int64(value, self->ticks_since_start);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

static void
ddd_time_point_init(DddTimePoint* self)
{
    (void) self;
}

static void
ddd_time_point_class_init(DddTimePointClass *klass)
{
    GObjectClass *obj_class = G_OBJECT_CLASS(klass);

    obj_class->set_property = ddd_time_point_set_property;
    obj_class->get_property = ddd_time_point_get_property;

    obj_properties[PROP_NUM_TICKS] = g_param_spec_int64(
            "num-ticks",
            "num_ticks",
            "The number of microseconds since the start of the clock",
            G_MININT64,
            G_MAXINT64,
            0,
            G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY
            );
    g_object_class_install_properties(obj_class, NUM_PROPERTIES, obj_properties);
}

DddDuration*
ddd_time_point_subtract(DddTimePoint* self, DddTimePoint* other)
{
    g_return_val_if_fail(DDD_IS_TIME_POINT(self), NULL);
    g_return_val_if_fail(DDD_IS_TIME_POINT(other), NULL);

    gint64 us_result;
    gboolean over_or_underflows = ddd_safe_sub_gint64(
            self->ticks_since_start,
            other->ticks_since_start,
            &us_result
            );
    g_return_val_if_fail(!over_or_underflows, NULL);

    return ddd_duration_new_us(us_result);
}

DddTimePoint*
ddd_time_point_subtract_dur(DddTimePoint* self, DddDuration* dur)
{
    g_return_val_if_fail(DDD_IS_TIME_POINT(self), NULL);
    g_return_val_if_fail(DDD_IS_DURATION(dur), NULL);

    gint64 new_ticks, ticks, us;
    us = ddd_duration_get_us(dur);
    ticks = self->ticks_since_start;

    gboolean over_or_under_flows;
    over_or_under_flows = ddd_safe_sub_gint64(ticks, us, &new_ticks);

    g_return_val_if_fail(!over_or_under_flows, NULL);
    DddTimePoint *tret = g_object_new(DDD_TYPE_TIME_POINT, NULL);
    tret->ticks_since_start = new_ticks;

    return tret;
}

DddTimePoint*
ddd_time_point_add(DddTimePoint* self, DddDuration* dur)
{
    g_return_val_if_fail(DDD_IS_TIME_POINT(self), NULL);
    g_return_val_if_fail(DDD_IS_DURATION(dur), NULL);

    gint64 us = ddd_duration_get_us(dur);
    gint64 new_ticks = 0;

    gboolean over_or_under_flows = ddd_safe_add_gint64(
            self->ticks_since_start,
            us,
            &new_ticks
            );
    g_return_val_if_fail(!over_or_under_flows, NULL);

    DddTimePoint *tp = g_object_new(DDD_TYPE_TIME_POINT, NULL);
    tp->ticks_since_start = new_ticks;
    return tp;
}

DddDuration*
ddd_time_point_duration_since_start(DddTimePoint* self)
{
    g_return_val_if_fail(DDD_IS_TIME_POINT(self), NULL);
    DddTimePoint *tzero = g_object_new(DDD_TYPE_TIME_POINT, NULL);

    DddDuration *dur = ddd_time_point_subtract(self, tzero);

    g_object_unref(tzero);
    return dur;
}

gboolean
ddd_time_point_less(DddTimePoint* self, DddTimePoint* other)
{
    g_return_val_if_fail(DDD_IS_TIME_POINT(self) && DDD_IS_TIME_POINT(other), FALSE);
    return self->ticks_since_start < other->ticks_since_start;
}

gboolean
ddd_time_point_less_equal(DddTimePoint* self, DddTimePoint* other)
{
    return ddd_time_point_less(self, other) || ddd_time_point_equal(self, other);
}

gboolean
ddd_time_point_equal(DddTimePoint* self, DddTimePoint* other)
{
    g_return_val_if_fail(DDD_IS_TIME_POINT(self) && DDD_IS_TIME_POINT(other), FALSE);
    return self->ticks_since_start == other->ticks_since_start;
}

gboolean
ddd_time_point_not_equal(DddTimePoint* self, DddTimePoint* other)
{
    return !ddd_time_point_equal(self, other);
}

gboolean
ddd_time_point_greater_equal(DddTimePoint* self, DddTimePoint* other)
{
    return ddd_time_point_equal(self, other) || !ddd_time_point_less(self, other);
}

gboolean
ddd_time_point_greater(DddTimePoint *self, DddTimePoint* other)
{
    return !ddd_time_point_equal(self, other) && !ddd_time_point_less(self, other);
}
