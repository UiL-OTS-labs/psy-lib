
#include "psy-time-point.h"
#include "psy-duration.h"
#include "psy-safe-int-private.h"

typedef struct _PsyTimePoint {
    GObject parent;
    gint64  ticks_since_start;
} PsyTimePoint;

typedef enum {
    PROP_NULL,
    PROP_NUM_TICKS,
    NUM_PROPERTIES
} PsyTimePointProperty;

G_DEFINE_TYPE(
        PsyTimePoint,
        psy_time_point,
        G_TYPE_OBJECT
        )

static GParamSpec* obj_properties[NUM_PROPERTIES];

static void
psy_time_point_set_property(
        GObject      *object,
        guint         property_id,
        const GValue *value,
        GParamSpec   *pspec
        )
{
    PsyTimePoint *tp = PSY_TIME_POINT(object);
    switch ((PsyTimePointProperty) property_id) {
        case PROP_NUM_TICKS:
            tp->ticks_since_start = g_value_get_int64(value);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

static void
psy_time_point_get_property(
        GObject    *object,
        guint       property_id,
        GValue     *value,
        GParamSpec *pspec
        )
{
    PsyTimePoint* self = PSY_TIME_POINT(object);
    switch ((PsyTimePointProperty) property_id) {
        case PROP_NUM_TICKS:
            g_value_set_int64(value, self->ticks_since_start);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

static void
psy_time_point_init(PsyTimePoint* self)
{
    (void) self;
}

static void
psy_time_point_class_init(PsyTimePointClass *klass)
{
    GObjectClass *obj_class = G_OBJECT_CLASS(klass);

    obj_class->set_property = psy_time_point_set_property;
    obj_class->get_property = psy_time_point_get_property;

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

PsyDuration*
psy_time_point_subtract(PsyTimePoint* self, PsyTimePoint* other)
{
    g_return_val_if_fail(PSY_IS_TIME_POINT(self), NULL);
    g_return_val_if_fail(PSY_IS_TIME_POINT(other), NULL);

    gint64 us_result;
    gboolean over_or_underflows = psy_safe_sub_gint64(
            self->ticks_since_start,
            other->ticks_since_start,
            &us_result
            );
    g_return_val_if_fail(!over_or_underflows, NULL);

    return psy_duration_new_us(us_result);
}

PsyTimePoint*
psy_time_point_subtract_dur(PsyTimePoint* self, PsyDuration* dur)
{
    g_return_val_if_fail(PSY_IS_TIME_POINT(self), NULL);
    g_return_val_if_fail(PSY_IS_DURATION(dur), NULL);

    gint64 new_ticks, ticks, us;
    us = psy_duration_get_us(dur);
    ticks = self->ticks_since_start;

    gboolean over_or_under_flows;
    over_or_under_flows = psy_safe_sub_gint64(ticks, us, &new_ticks);

    g_return_val_if_fail(!over_or_under_flows, NULL);
    PsyTimePoint *tret = g_object_new(PSY_TYPE_TIME_POINT, NULL);
    tret->ticks_since_start = new_ticks;

    return tret;
}

PsyTimePoint*
psy_time_point_add(PsyTimePoint* self, PsyDuration* dur)
{
    g_return_val_if_fail(PSY_IS_TIME_POINT(self), NULL);
    g_return_val_if_fail(PSY_IS_DURATION(dur), NULL);

    gint64 us = psy_duration_get_us(dur);
    gint64 new_ticks = 0;

    gboolean over_or_under_flows = psy_safe_add_gint64(
            self->ticks_since_start,
            us,
            &new_ticks
            );
    g_return_val_if_fail(!over_or_under_flows, NULL);

    PsyTimePoint *tp = g_object_new(PSY_TYPE_TIME_POINT, NULL);
    tp->ticks_since_start = new_ticks;
    return tp;
}

PsyDuration*
psy_time_point_duration_since_start(PsyTimePoint* self)
{
    g_return_val_if_fail(PSY_IS_TIME_POINT(self), NULL);
    PsyTimePoint *tzero = g_object_new(PSY_TYPE_TIME_POINT, NULL);

    PsyDuration *dur = psy_time_point_subtract(self, tzero);

    g_object_unref(tzero);
    return dur;
}

gboolean
psy_time_point_less(PsyTimePoint* self, PsyTimePoint* other)
{
    g_return_val_if_fail(PSY_IS_TIME_POINT(self) && PSY_IS_TIME_POINT(other), FALSE);
    return self->ticks_since_start < other->ticks_since_start;
}

gboolean
psy_time_point_less_equal(PsyTimePoint* self, PsyTimePoint* other)
{
    return psy_time_point_less(self, other) || psy_time_point_equal(self, other);
}

gboolean
psy_time_point_equal(PsyTimePoint* self, PsyTimePoint* other)
{
    g_return_val_if_fail(PSY_IS_TIME_POINT(self) && PSY_IS_TIME_POINT(other), FALSE);
    return self->ticks_since_start == other->ticks_since_start;
}

gboolean
psy_time_point_not_equal(PsyTimePoint* self, PsyTimePoint* other)
{
    return !psy_time_point_equal(self, other);
}

gboolean
psy_time_point_greater_equal(PsyTimePoint* self, PsyTimePoint* other)
{
    return psy_time_point_equal(self, other) || !psy_time_point_less(self, other);
}

gboolean
psy_time_point_greater(PsyTimePoint *self, PsyTimePoint* other)
{
    return !psy_time_point_equal(self, other) && !psy_time_point_less(self, other);
}
