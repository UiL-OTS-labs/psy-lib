
#include "psy-duration.h"
#include "psy-safe-int-private.h"
#include <math.h>

/**
 * PsyDuration:
 *
 * PsyDurations specifies the duration between two instances of
 * [struct@TimePoint]. A time point can be seen as the number of micro seconds
 * since an arbitrary start point. This duration is the number of microseconds
 * between two of such time points.
 * Internally the duration works with 64bit integer precision, The maximum
 * duration should is: 18446744073709551615 µs and the
 * minimum should is: -18446744073709551616 µs.
 *
 * So this spans between roughly -584942 and 584942 years, which should
 * be more than ample for your average psychological experiment.
 */

#define MS_PER_US 1000
#define US_PER_S 1000000

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"

G_DEFINE_BOXED_TYPE(PsyDuration,
                    psy_duration,
                    psy_duration_copy,
                    psy_duration_free);

#pragma GCC diagnostic pop

struct PsyDuration {
    gint64 us;
};

/**
 * psy_duration_new:(constructor)
 * @seconds:the number of seconds of this duration, seconds may contain
 *          a fractional part of a second.
 *
 * Allows to create a duration in seconds. Internally, the number
 * of seconds specified will be stored with µs precision.
 *
 * Objects created this way should be freed with [struct@Duration.free]
 *
 * Returns:A new `PsyDuration` instance free with psy_duration_free
 */
PsyDuration *
psy_duration_new(gdouble seconds)
{
    const double max_dur_us = (gdouble) G_MAXINT64 / US_PER_S;
    const double min_dur_us = (gdouble) G_MININT64 / US_PER_S;
    gdouble      us         = round(seconds * US_PER_S);

    gboolean overflowed = us < min_dur_us || us > max_dur_us;

    g_return_val_if_fail(!overflowed, NULL);

    gint64 i_us = (gint64) us;

    PsyDuration *dur = psy_duration_new_us(i_us);

    return dur;
}

/**
 * psy_duration_new_us:(constructor)
 * @us: The number of microseconds of the returned duration
 *
 * Returns: a new `PsyDuration` instance
 */
PsyDuration *
psy_duration_new_us(gint64 us)
{
    PsyDuration *dur = g_new(PsyDuration, 1);
    dur->us          = us;
    return dur;
}

/**
 * psy_duration_new_ms:(constructor)
 * @ms: The number of microseconds of the returned duration
 *
 * Returns: a new `PsyDuration` instance or NULL when the number
 * of ms overflows the maximum allowed number of microseconds
 */
PsyDuration *
psy_duration_new_ms(gint64 ms)
{
    gint64   us;
    gboolean overflows = psy_safe_mul_gint64(ms, 1000, &us);

    g_return_val_if_fail(!overflows, NULL);

    PsyDuration *dur = psy_duration_new_us(us);
    return dur;
}

/**
 * psy_duration_new_s:(constructor)
 * @s:
 *
 * Returns:
 */
PsyDuration *
psy_duration_new_s(gint64 s)
{
    const gint64 num_us_in_s = 1000000;
    gboolean     overflows
        = (G_MAXINT64 / num_us_in_s < s) || (G_MININT64 / num_us_in_s > s);

    g_return_val_if_fail(!overflows, NULL);

    gint64       us  = s * num_us_in_s;
    PsyDuration *dur = psy_duration_new_us(us);
    return dur;
}

/**
 * psy_duration_free:
 * @self: an instance of [struct@Duration] to free/destroy
 *
 * Destroys instances previously created with psy_duration_new* or
 * psy_duration_copy functions.
 */
void
psy_duration_free(PsyDuration *self)
{
    g_return_if_fail(self != NULL);
    g_free(self);
}

/**
 * psy_duration_copy:
 * @self: an instace of [struct@Duration]
 *
 * Copies a duration, the copy should be freed with [struct@Duration.free]
 *
 * Returns:(transfer full): a new copy of `self` free with
 * [struct@Duration.free]
 */
PsyDuration *
psy_duration_copy(PsyDuration *self)
{
    g_return_val_if_fail(self != NULL, NULL);

    PsyDuration *dup = psy_duration_new_us(self->us);

    return dup;
}

/**
 * psy_duration_get_us:
 * @self: a `PsyDuration` instance
 *
 * Returns: The number of microseconds this duration represents
 */
gint64
psy_duration_get_us(PsyDuration *self)
{
    g_return_val_if_fail(self != NULL, G_MININT64);
    return self->us;
}

/**
 * psy_duration_get_ms:
 * @self: a `PsyDuration` instance
 *
 * Returns: The number of milliseconds this duration represents
 */
gint64
psy_duration_get_ms(PsyDuration *self)
{
    g_return_val_if_fail(self != NULL, G_MININT64);
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
psy_duration_get_s(PsyDuration *self)
{
    g_return_val_if_fail(self != NULL, G_MININT64);
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
psy_duration_get_seconds(PsyDuration *self)
{
    g_return_val_if_fail(self != NULL, G_MININT64);
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
psy_duration_divide(PsyDuration *self, PsyDuration *other)
{
    g_return_val_if_fail(self != NULL && other != NULL, 0);
    return self->us / other->us;
}

/**
 * psy_duration_divide_rounded:
 * @self: a `PsyDuration` instance
 * @other: another `PsyDuration` instance
 *
 * Since durations are based on `gint64` µs standard division is truncating.
 * This method returns the result rounded to the nearest `gint64`.
 *
 * Returns: a division of durations that returns the result of the
 *          division rounded to the nearest integer.
 */
gint64
psy_duration_divide_rounded(PsyDuration *self, PsyDuration *other)
{
    g_return_val_if_fail(self != NULL && other != NULL, 0);

    // Thanks to:  https://stackoverflow.com/a/18067292/2082884

    gint64 n = self->us;
    gint64 d = other->us;

    return ((n < 0) ^ (d < 0)) ? ((n - d / 2) / d) : ((n + d / 2) / d);
}

/**
 * psy_duration_divide_scalar:
 * @self: A `PsyDuration` instance
 * @scalar A scalar to divide @self by
 *
 * Returns:(transfer full): the result (a `PsyDuration) of @self/scalar
 */
PsyDuration *
psy_duration_divide_scalar(PsyDuration *self, gint64 scalar)
{
    g_return_val_if_fail(self != NULL, NULL);

    gint64 us = self->us / scalar;
    return psy_duration_new_us(us);
}

/**
 * psy_duration_multiply_scalar:
 * @self: A `PsyDuration` instance.
 * @scalar: A scalar value
 *
 * This multiplies a duration a number of times, so 3s * 3 = 9s. This
 * works very nice for typical durations in an experiment like a whole number of
 * milliseconds or second or even µseconds, however, it does not work so well
 * for computing the duration of a number of audio sample rates. As 1 sample
 * sampled at 44100 Hz is roughly 22.68 µs. Since a duration has internally
 * the resolution of 1 µs you'll lose .68 µs for every sample.
 *
 * So if you would like to compute time based on audio sample rates use
 * [func@Psy.num_audio_samples_to_duration] instead.
 *
 * Returns:(transfer full): The result of @self * scalar.
 */
PsyDuration *
psy_duration_multiply_scalar(PsyDuration *self, gint64 scalar)
{
    g_return_val_if_fail(self != NULL, NULL);
    gint64   us;
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
PsyDuration *
psy_duration_add(PsyDuration *self, PsyDuration *other)
{
    g_return_val_if_fail(self != NULL && other != NULL, NULL);
    gint64   us;
    gboolean over_flows_or_under_flows;

    over_flows_or_under_flows = psy_safe_add_gint64(self->us, other->us, &us);
    g_return_val_if_fail(!over_flows_or_under_flows, NULL);

    return psy_duration_new_us(us);
}

/**
 * psy_duration_subtract:
 * @self: A `PsyDuration` instance
 * @other: A `PsyDuration` instance
 *
 * Returns:(transfer full): a new `PsyDuration` that is the result of
 *         @self - @other
 */
PsyDuration *
psy_duration_subtract(PsyDuration *self, PsyDuration *other)
{
    g_return_val_if_fail(self != NULL && other != NULL, NULL);
    gint64   us;
    gboolean over_flows_or_under_flows;

    over_flows_or_under_flows = psy_safe_sub_gint64(self->us, other->us, &us);
    g_return_val_if_fail(!over_flows_or_under_flows, NULL);

    return psy_duration_new_us(us);
}

/**
 * psy_duration_lless:
 * @self: A `PsyDuration` instance
 * @other: A `PsyDuration` instance
 *
 * Returns:TRUE if @self < @other, FALSE otherwise.
 */
gboolean
psy_duration_less(PsyDuration *self, PsyDuration *other)
{
    g_return_val_if_fail(self != NULL && other != NULL, FALSE);
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
psy_duration_less_equal(PsyDuration *self, PsyDuration *other)
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
psy_duration_equal(PsyDuration *self, PsyDuration *other)
{
    g_return_val_if_fail(self != NULL && other != NULL, FALSE);
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
psy_duration_greater_equal(PsyDuration *self, PsyDuration *other)
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
psy_duration_greater(PsyDuration *self, PsyDuration *other)
{
    return !psy_duration_equal(self, other) && !psy_duration_less(self, other);
}
