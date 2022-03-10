
#include "psy-duration.h"
#include "psy-safe-int-private.h"
#include <math.h>

#define MS_PER_US 1000
#define US_PER_S  1000000

typedef struct _PsyDuration {
    GObject parent;
    gint64  us;
} PsyDuration;

typedef enum {
    PROP_NULL, //
    PROP_US,        // Number of microseconds.
    PROP_MS,        // Number of milliseconds.
    PROP_S,         // Number of seconds.
    PROP_SECONDS,   // Number of seconds with floating point result
    NUM_PROPERTIES
} PsyDurationProperty;

G_DEFINE_TYPE (
        PsyDuration,
        psy_duration,
        G_TYPE_OBJECT
        )

static GParamSpec* obj_properties[NUM_PROPERTIES];

static void
psy_duration_set_property(
        GObject      *object,
        guint         property_id,
        const GValue *value,
        GParamSpec   *pspec
        )
{
    PsyDuration *self = PSY_DURATION(object);

    switch ((PsyDurationProperty) property_id) {
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
psy_duration_get_property(
        GObject    *object,
        guint       property_id,
        GValue     *value,
        GParamSpec *pspec
        )
{
    PsyDuration * self = PSY_DURATION(object);
    switch ((PsyDurationProperty) property_id) {
        case PROP_US:
            g_value_set_int64(value, psy_duration_get_us(self));
            break;
        case PROP_MS:
            g_value_set_int64(value, psy_duration_get_ms(self));
            break;
        case PROP_S:
            g_value_set_int64(value, psy_duration_get_s(self));
            break;
        case PROP_SECONDS:
            g_value_set_double(value, psy_duration_get_seconds(self));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

static void
psy_duration_init(PsyDuration *self)
{
    (void) self;
}

static void
psy_duration_class_init(PsyDurationClass* klass)
{
    GObjectClass *obj_class = G_OBJECT_CLASS(klass);

    obj_class->set_property = psy_duration_set_property;
    obj_class->get_property = psy_duration_get_property;

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

PsyDuration*
psy_duration_new(gdouble seconds)
{
    const double max_dur_us = (gdouble) G_MAXINT64 / US_PER_S;
    const double min_dur_us = (gdouble) G_MININT64 / US_PER_S;
    gdouble us = round(seconds * US_PER_S);

    gboolean overflowed = us < min_dur_us || us > max_dur_us;

    g_return_val_if_fail(!overflowed, NULL);

    gint64 i_us = (gint64) us;

    PsyDuration *dur = g_object_new(
            PSY_TYPE_DURATION,
            "us", i_us,
            NULL
            );

    return dur;
}

PsyDuration*
psy_duration_new_us(gint64 us)
{
    PsyDuration *dur = g_object_new(PSY_TYPE_DURATION, "us", us, NULL);
    return dur;
}

PsyDuration*
psy_duration_new_ms(gint64 ms)
{
    gint64 us;
    gboolean overflows = psy_safe_mul_gint64(ms, 1000, &us);

    g_return_val_if_fail(!overflows, NULL);

    PsyDuration *dur = g_object_new(PSY_TYPE_DURATION, "us", us, NULL);
    return dur;
}

PsyDuration*
psy_duration_new_s(gint64 s)
{
    const gint64 num_us_in_s = 1000000;
    gboolean overflows = (G_MAXINT64 / num_us_in_s < s) ||
                         (G_MININT64 / num_us_in_s > s);

    g_return_val_if_fail(!overflows, NULL);

    gint64 us = s * num_us_in_s;
    PsyDuration *dur = g_object_new(PSY_TYPE_DURATION, "us", us, NULL);
    return dur;
}

gint64
psy_duration_get_us(PsyDuration* self)
{
    g_return_val_if_fail(PSY_IS_DURATION(self), G_MININT64);
    return self->us;
}

gint64
psy_duration_get_ms(PsyDuration* self)
{
    g_return_val_if_fail(PSY_IS_DURATION(self), G_MININT64);
    return self->us / 1000;
}

gint64
psy_duration_get_s(PsyDuration* self)
{
    g_return_val_if_fail(PSY_IS_DURATION(self), G_MININT64);
    return self->us / 1000000;
}

gdouble
psy_duration_get_seconds(PsyDuration* self)
{
    g_return_val_if_fail(PSY_IS_DURATION(self), G_MININT64);
    return ((gdouble) self->us) / 1000000;
}

gint64
psy_duration_divide(PsyDuration* self, PsyDuration *other)
{
    g_return_val_if_fail(PSY_IS_DURATION(self) && PSY_IS_DURATION(other), 0);
    return self->us / other->us;
}

PsyDuration*
psy_duration_divide_scalar(PsyDuration* self, gint64 scalar)
{
    g_return_val_if_fail(PSY_IS_DURATION(self), NULL);

    gint64 us = self->us / scalar;
    return g_object_new(PSY_TYPE_DURATION, "us", us, NULL);
}

PsyDuration*
psy_duration_multiply_scalar(PsyDuration* self, gint64 scalar)
{
    g_return_val_if_fail(PSY_IS_DURATION(self), NULL);
    gint64 us;
    gboolean over_or_under_flows = psy_safe_mul_gint64(self->us, scalar, &us);

    g_return_val_if_fail(!over_or_under_flows, NULL);

    return psy_duration_new_us(us);
}

PsyDuration*
psy_duration_add(PsyDuration* self, PsyDuration* other)
{
    g_return_val_if_fail(PSY_IS_DURATION(self) && PSY_IS_DURATION(other), NULL);
    gint64 us;
    gboolean over_flows_or_under_flows;

    over_flows_or_under_flows = psy_safe_add_gint64(self->us, other->us, &us);
    g_return_val_if_fail(!over_flows_or_under_flows, NULL);

    return g_object_new(PSY_TYPE_DURATION, "us", us, NULL);
}

PsyDuration*
psy_duration_subtract(PsyDuration* self, PsyDuration* other)
{
    g_return_val_if_fail(PSY_IS_DURATION(self) && PSY_IS_DURATION(other), NULL);
    gint64 us;
    gboolean over_flows_or_under_flows;

    over_flows_or_under_flows = psy_safe_sub_gint64(self->us, other->us, &us);
    g_return_val_if_fail(!over_flows_or_under_flows, NULL);

    return g_object_new(PSY_TYPE_DURATION, "us", us, NULL);
}

gboolean
psy_duration_less(PsyDuration* self, PsyDuration* other)
{
    g_return_val_if_fail(PSY_IS_DURATION(self) && PSY_IS_DURATION(other), FALSE);
    return self->us < other->us;
}

gboolean
psy_duration_less_equal(PsyDuration* self, PsyDuration* other)
{
    return psy_duration_less(self, other) || psy_duration_equal(self, other);
}

gboolean
psy_duration_equal(PsyDuration* self, PsyDuration* other)
{
    g_return_val_if_fail(PSY_IS_DURATION(self) && PSY_IS_DURATION(other), FALSE);
    return self->us == other->us;
}

gboolean
psy_duration_not_equal(PsyDuration *self, PsyDuration *other)
{
    return !psy_duration_equal(self, other);
}

gboolean
psy_duration_greater_equal(PsyDuration* self, PsyDuration* other)
{
    return psy_duration_equal(self, other) || !psy_duration_less(self, other);
}

gboolean
psy_duration_greater(PsyDuration *self, PsyDuration* other)
{
    return !psy_duration_equal(self, other) && !psy_duration_less(self, other);
}
