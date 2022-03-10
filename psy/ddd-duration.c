
#include "ddd-duration.h"
#include "ddd-safe-int-private.h"
#include <math.h>

#define MS_PER_US 1000
#define US_PER_S  1000000

typedef struct _DddDuration {
    GObject parent;
    gint64  us;
} DddDuration;

typedef enum {
    PROP_NULL, //
    PROP_US,        // Number of microseconds.
    PROP_MS,        // Number of milliseconds.
    PROP_S,         // Number of seconds.
    PROP_SECONDS,   // Number of seconds with floating point result
    NUM_PROPERTIES
} DddDurationProperty;

G_DEFINE_TYPE (
        DddDuration,
        ddd_duration,
        G_TYPE_OBJECT
        )

static GParamSpec* obj_properties[NUM_PROPERTIES];

static void
ddd_duration_set_property(
        GObject      *object,
        guint         property_id,
        const GValue *value,
        GParamSpec   *pspec
        )
{
    DddDuration *self = DDD_DURATION(object);

    switch ((DddDurationProperty) property_id) {
        case PROP_US:
            self->us = g_value_get_int64(value);
            break;
        case PROP_MS: // readable
        case PROP_S:  // readable
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

static void
ddd_duration_get_property(
        GObject    *object,
        guint       property_id,
        GValue     *value,
        GParamSpec *pspec
        )
{
    DddDuration * self = DDD_DURATION(object);
    switch ((DddDurationProperty) property_id) {
        case PROP_US:
            g_value_set_int64(value, ddd_duration_get_us(self));
            break;
        case PROP_MS:
            g_value_set_int64(value, ddd_duration_get_ms(self));
            break;
        case PROP_S:
            g_value_set_int64(value, ddd_duration_get_s(self));
            break;
        case PROP_SECONDS:
            g_value_set_double(value, ddd_duration_get_seconds(self));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

static void
ddd_duration_init(DddDuration *self)
{
    (void) self;
}

static void
ddd_duration_class_init(DddDurationClass* klass)
{
    GObjectClass *obj_class = G_OBJECT_CLASS(klass);

    obj_class->set_property = ddd_duration_set_property;
    obj_class->get_property = ddd_duration_get_property;

    obj_properties[PROP_US] = g_param_spec_int64(
            "us",
            "µs",
            "The number of microseconds of this duration.",
            G_MININT64,
            G_MAXINT64,
            0,
            G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY
            );

    obj_properties[PROP_MS] = g_param_spec_int64(
            "ms",
            "milliseconds",
            "The number of milliseconds of this duration.",
            G_MININT64 / 1000,
            G_MAXINT64 / 1000,
            0,
            G_PARAM_READABLE
            );

    obj_properties[PROP_S] = g_param_spec_int64(
            "s",
            "seconds",
            "The number of seconds of this duration.",
            G_MININT64,
            G_MAXINT64,
            0,
            G_PARAM_READABLE
            );

    obj_properties[PROP_SECONDS] = g_param_spec_double(
            "seconds",
            "Seconds",
            "The number of seconds of this duration in floating point format.",
            (gdouble) G_MININT64 / US_PER_S,
            (gdouble) G_MAXINT64 / US_PER_S,
            0,
            G_PARAM_READABLE
    );

    g_object_class_install_properties(obj_class, NUM_PROPERTIES, obj_properties);
}

DddDuration*
ddd_duration_new(gdouble seconds)
{
    const double max_dur_us = (gdouble) G_MAXINT64 / US_PER_S;
    const double min_dur_us = (gdouble) G_MININT64 / US_PER_S;
    gdouble us = round(seconds * US_PER_S);

    gboolean overflowed = us < min_dur_us || us > max_dur_us;

    g_return_val_if_fail(!overflowed, NULL);

    gint64 i_us = (gint64) us;

    DddDuration *dur = g_object_new(
            DDD_TYPE_DURATION,
            "us", i_us,
            NULL
            );

    return dur;
}

DddDuration*
ddd_duration_new_us(gint64 us)
{
    DddDuration *dur = g_object_new(DDD_TYPE_DURATION, "us", us, NULL);
    return dur;
}

DddDuration*
ddd_duration_new_ms(gint64 ms)
{
    gint64 us;
    gboolean overflows = ddd_safe_mul_gint64(ms, 1000, &us);

    g_return_val_if_fail(!overflows, NULL);

    DddDuration *dur = g_object_new(DDD_TYPE_DURATION, "us", us, NULL);
    return dur;
}

DddDuration*
ddd_duration_new_s(gint64 s)
{
    const gint64 num_us_in_s = 1000000;
    gboolean overflows = (G_MAXINT64 / num_us_in_s < s) ||
                         (G_MININT64 / num_us_in_s > s);

    g_return_val_if_fail(!overflows, NULL);

    gint64 us = s * num_us_in_s;
    DddDuration *dur = g_object_new(DDD_TYPE_DURATION, "us", us, NULL);
    return dur;
}

gint64
ddd_duration_get_us(DddDuration* self)
{
    g_return_val_if_fail(DDD_IS_DURATION(self), G_MININT64);
    return self->us;
}

gint64
ddd_duration_get_ms(DddDuration* self)
{
    g_return_val_if_fail(DDD_IS_DURATION(self), G_MININT64);
    return self->us / 1000;
}

gint64
ddd_duration_get_s(DddDuration* self)
{
    g_return_val_if_fail(DDD_IS_DURATION(self), G_MININT64);
    return self->us / 1000000;
}

gdouble
ddd_duration_get_seconds(DddDuration* self)
{
    g_return_val_if_fail(DDD_IS_DURATION(self), G_MININT64);
    return ((gdouble) self->us) / 1000000;
}

gint64
ddd_duration_divide(DddDuration* self, DddDuration *other)
{
    g_return_val_if_fail(DDD_IS_DURATION(self) && DDD_IS_DURATION(other), 0);
    return self->us / other->us;
}

DddDuration*
ddd_duration_divide_scalar(DddDuration* self, gint64 scalar)
{
    g_return_val_if_fail(DDD_IS_DURATION(self), NULL);

    gint64 us = self->us / scalar;
    return g_object_new(DDD_TYPE_DURATION, "us", us, NULL);
}

DddDuration*
ddd_duration_multiply_scalar(DddDuration* self, gint64 scalar)
{
    g_return_val_if_fail(DDD_IS_DURATION(self), NULL);
    gint64 us;
    gboolean over_or_under_flows = ddd_safe_mul_gint64(self->us, scalar, &us);

    g_return_val_if_fail(!over_or_under_flows, NULL);

    return ddd_duration_new_us(us);
}

DddDuration*
ddd_duration_add(DddDuration* self, DddDuration* other)
{
    g_return_val_if_fail(DDD_IS_DURATION(self) && DDD_IS_DURATION(other), NULL);
    gint64 us;
    gboolean over_flows_or_under_flows;

    over_flows_or_under_flows = ddd_safe_add_gint64(self->us, other->us, &us);
    g_return_val_if_fail(!over_flows_or_under_flows, NULL);

    return g_object_new(DDD_TYPE_DURATION, "us", us, NULL);
}

DddDuration*
ddd_duration_subtract(DddDuration* self, DddDuration* other)
{
    g_return_val_if_fail(DDD_IS_DURATION(self) && DDD_IS_DURATION(other), NULL);
    gint64 us;
    gboolean over_flows_or_under_flows;

    over_flows_or_under_flows = ddd_safe_sub_gint64(self->us, other->us, &us);
    g_return_val_if_fail(!over_flows_or_under_flows, NULL);

    return g_object_new(DDD_TYPE_DURATION, "us", us, NULL);
}

gboolean
ddd_duration_less(DddDuration* self, DddDuration* other)
{
    g_return_val_if_fail(DDD_IS_DURATION(self) && DDD_IS_DURATION(other), FALSE);
    return self->us < other->us;
}

gboolean
ddd_duration_less_equal(DddDuration* self, DddDuration* other)
{
    return ddd_duration_less(self, other) || ddd_duration_equal(self, other);
}

gboolean
ddd_duration_equal(DddDuration* self, DddDuration* other)
{
    g_return_val_if_fail(DDD_IS_DURATION(self) && DDD_IS_DURATION(other), FALSE);
    return self->us == other->us;
}

gboolean
ddd_duration_not_equal(DddDuration *self, DddDuration *other)
{
    return !ddd_duration_equal(self, other);
}

gboolean
ddd_duration_greater_equal(DddDuration* self, DddDuration* other)
{
    return ddd_duration_equal(self, other) || !ddd_duration_less(self, other);
}

gboolean
ddd_duration_greater(DddDuration *self, DddDuration* other)
{
    return !ddd_duration_equal(self, other) && !ddd_duration_less(self, other);
}
