
#include "psy-duration.h"
#include "psy-safe-int-private.h"
#include <math.h>

/**
 * PsyDuration:
 *
 * PsyDurations specifies the duration between two `PsyTimePoints`
 * A PsyTimePoint can be seen a the number of micro seconds since
 * an arbitrary start point. This duration is the number of microseconds
 * between two of such timepoints.
 * Internally the duration works with 64bit integer precision, The maximum
 * duration should is: 18446744073709551615 µs and the
 * minimum should is: -18446744073709551616 µs.
 *
 * So this spans between roughly -584942 and 584942 years, which should
 * be more than ample for your average psychological experiment.
 */

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

    /**
     * PsyDuration:us:
     *
     * Get/Set the duration in µs.
     */
    obj_properties[PROP_US] = g_param_spec_int64(
            "us",
            "µs",
            "The number of microseconds of this duration.",
            G_MININT64,
            G_MAXINT64,
            0,
            G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY
            );

    /**
     * PsyDuration:ms:
     *
     * Get/Set the duration in ms, when getting the duration
     * if the also contains µs, those will be truncated, hence
     * there is no rounding to the nearest millisecond.
     */
    obj_properties[PROP_MS] = g_param_spec_int64(
            "ms",
            "milliseconds",
            "The number of milliseconds of this duration.",
            G_MININT64 / 1000,
            G_MAXINT64 / 1000,
            0,
            G_PARAM_READABLE
            );

    /**
     * PsyDuration:s:
     *
     * Get/Set the duration in seconds, when the duration
     * also contains ms/µs, those will be truncated, hence
     * there is no rounding to the nearest second.
     * If that would be desired, one could use `PsyDuration:seconds`
     * as that returns the value as a double precision floating point
     * number.
     */
    obj_properties[PROP_S] = g_param_spec_int64(
            "s",
            "seconds",
            "The number of seconds of this duration.",
            G_MININT64,
            G_MAXINT64,
            0,
            G_PARAM_READABLE
            );

    /**
     * PsyDuration:seconds:
     *
     * Provides the duration in seconds of this `PsyDuration` instance
     * The return contains a fractional part.
     */
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

/**
 * psy_duration_new:(constructor)
 * @seconds:the number of seconds of this duration, seconds may contain
 *          a fractional part of a second.
 *
 * Allows to create a duration in seconds. Internally, the number
 * of seconds specified will be stored with µs precision.
 *
 * Returns:A new `PsyDuration` instance
 */
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

/**
 * psy_duration_new_us:(constructor)
 * @us: The number of microseconds of the returned duration
 *
 * Returns: a new `PsyDuration` instance
 */
PsyDuration*
psy_duration_new_us(gint64 us)
{
    PsyDuration *dur = g_object_new(PSY_TYPE_DURATION, "us", us, NULL);
    return dur;
}

/**
 * psy_duration_new_ms:(constructor)
 * @ms: The number of microseconds of the returned duration
 *
 * Returns: a new `PsyDuration` instance or NULL when the number
 * of ms overflows the maximum allowed number of microseconds
 */
PsyDuration*
psy_duration_new_ms(gint64 ms)
{
    gint64 us;
    gboolean overflows = psy_safe_mul_gint64(ms, 1000, &us);

    g_return_val_if_fail(!overflows, NULL);

    PsyDuration *dur = g_object_new(PSY_TYPE_DURATION, "us", us, NULL);
    return dur;
}

/**
 * psy_duration_new_s:(constructor)
 * @s:
 *
 * Returns:
 */
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

/**
 * psy_duration_get_us:
 * @self: a `PsyDuration` instance
 *
 * Returns: The number of microseconds this duration represents
 */
gint64
psy_duration_get_us(PsyDuration* self)
{
    g_return_val_if_fail(PSY_IS_DURATION(self), G_MININT64);
    return self->us;
}

/**
 * psy_duration_get_ms:
 * @self: a `PsyDuration` instance
 *
 * Returns: The number of milliseconds this duration represents
 */
gint64
psy_duration_get_ms(PsyDuration* self)
{
    g_return_val_if_fail(PSY_IS_DURATION(self), G_MININT64);
    return self->us / 1000;
}

/**
 * psy_duration_get_s:
 * @self: a `PsyDuration` instance
 *
 * Obtain the number of seconds this duration represents
 * Note that the fractional part of the number the seconds
 * will be truncated.
 *
 * Returns: The number of seconds this duration represents
 */
gint64
psy_duration_get_s(PsyDuration* self)
{
    g_return_val_if_fail(PSY_IS_DURATION(self), G_MININT64);
    return self->us / 1000000;
}

/**
 * psy_duration_get_seconds:
 * @self: a `PsyDuration` instance
 *
 * Returns: The number of seconds this duration represents in 
 * a floating point format
 */
gdouble
psy_duration_get_seconds(PsyDuration* self)
{
    g_return_val_if_fail(PSY_IS_DURATION(self), G_MININT64);
    return ((gdouble) self->us) / 1000000;
}

/**
 * psy_duration_divide:
 * @self: A `PsyDuration` instance.
 * @other: A `PsyDuration` instance.
 *
 * Divides two `PsyDurations`, note that will strip the
 * unit, So 4ms / 2 ms = 2. As such this function will
 * return a plain integer that is an integer division of
 * the Durations in µs.
 *
 * Returns: the operation of integer division of @self/@other
 */
gint64
psy_duration_divide(PsyDuration* self, PsyDuration *other)
{
    g_return_val_if_fail(PSY_IS_DURATION(self) && PSY_IS_DURATION(other), 0);
    return self->us / other->us;
}

/**
 * psy_duration_divide_scalar:
 * @self: A `PsyDuration` instance
 * @scalar A scalar to divide @self by
 *
 * Returns:(transfer full): the result (a `PsyDuration) of @self/scalar
 */
PsyDuration*
psy_duration_divide_scalar(PsyDuration* self, gint64 scalar)
{
    g_return_val_if_fail(PSY_IS_DURATION(self), NULL);

    gint64 us = self->us / scalar;
    return g_object_new(PSY_TYPE_DURATION, "us", us, NULL);
}

/**
 * psy_duration_multiply_scalar:
 * @self: A `PsyDuration` instance.
 * @scalar: A scalar value
 *
 *
 * Returns:(transfer full): The result of @self * scalar.
 */
PsyDuration*
psy_duration_multiply_scalar(PsyDuration* self, gint64 scalar)
{
    g_return_val_if_fail(PSY_IS_DURATION(self), NULL);
    gint64 us;
    gboolean over_or_under_flows = psy_safe_mul_gint64(self->us, scalar, &us);

    g_return_val_if_fail(!over_or_under_flows, NULL);

    return psy_duration_new_us(us);
}

/**
 * psy_duration_add:
 * @self: A `PsyDuration` instance
 * @other: A `PsyDuration` instance
 *
 * Returns:(transfer full): a new `PsyDuration` that is the result of
 *         @self + @other
 */
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

/**
 * psy_duration_subtract:
 * @self: A `PsyDuration` instance
 * @other: A `PsyDuration` instance
 *
 * Returns:(transfer full): a new `PsyDuration` that is the result of
 *         @self - @other
 */
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

/**
 * psy_duration_less:
 * @self: A `PsyDuration` instance
 * @other: A `PsyDuration` instance
 *
 * Returns:TRUE if @self < @other, FALSE otherwise.
 */
gboolean
psy_duration_less(PsyDuration* self, PsyDuration* other)
{
    g_return_val_if_fail(PSY_IS_DURATION(self) && PSY_IS_DURATION(other), FALSE);
    return self->us < other->us;
}

/**
 * psy_duration_less_equal:
 * @self: A `PsyDuration` instance
 * @other: A `PsyDuration` instance
 *
 * Returns: TRUE if @self <= @other, FALSE otherwise.
 */
gboolean
psy_duration_less_equal(PsyDuration* self, PsyDuration* other)
{
    return psy_duration_less(self, other) || psy_duration_equal(self, other);
}

/**
 * psy_duration_equal:
 * @self: A `PsyDuration` instance
 * @other: A `PsyDuration` instance
 *
 * Returns: TRUE if @self == @other, FALSE otherwise.
 */
gboolean
psy_duration_equal(PsyDuration* self, PsyDuration* other)
{
    g_return_val_if_fail(PSY_IS_DURATION(self) && PSY_IS_DURATION(other), FALSE);
    return self->us == other->us;
}

/**
 * psy_duration_not_equal:
 * @self: A `PsyDuration` instance
 * @other: A `PsyDuration` instance
 *
 * Returns: TRUE if @self != @other, FALSE otherwise.
 */
gboolean
psy_duration_not_equal(PsyDuration *self, PsyDuration *other)
{
    return !psy_duration_equal(self, other);
}

/**
 * psy_duration_greater_equal:
 * @self: A `PsyDuration` instance
 * @other: A `PsyDuration` instance
 *
 * Returns: TRUE if @self >= @other, FALSE otherwise.
 */
gboolean
psy_duration_greater_equal(PsyDuration* self, PsyDuration* other)
{
    return psy_duration_equal(self, other) || !psy_duration_less(self, other);
}

/**
 * psy_duration_greater:
 * @self: A `PsyDuration` instance
 * @other: A `PsyDuration` instance
 *
 * Returns: TRUE if @self > @other, FALSE otherwise.
 */
gboolean
psy_duration_greater(PsyDuration *self, PsyDuration* other)
{
    return !psy_duration_equal(self, other) && !psy_duration_less(self, other);
}

