
#include "psy-time-point.h"
#include "psy-clock.h"
#include "psy-duration.h"
#include "psy-safe-int-private.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"

G_DEFINE_BOXED_TYPE(PsyTimePoint,
                    psy_time_point,
                    psy_time_point_copy,
                    psy_time_point_free);

#pragma GCC diagnostic pop

/**
 * PsyTimePoint:
 *
 * Instances of [struct@TimePoint] are generally obtained by
 * [method@Psy.Clock.now], which returns the current time as the
 * number of microseconds since the first instance of [class@Clock] is
 * instantiated. Common other ways of obtaining PsyTimePoints is the result of
 * a computation between timepoints and durations. One can create a
 * timepoint with the [struct@TimePoint.new], but than the timepoint isn't
 * really meaning full. It would reflect the timepoint at which the clock
 * would start ticking. You can however, create a timepoint from a result of
 * [method@GLib.get_monotonic_time], these timepoint are then shifted so that
 * they are comparable as if they were returned from our own [class@Clock]
 */

/**
 * psy_time_point_new:(constructor)
 *
 * Creates a timepoint that reflects the time that the first clock was created.
 * You should take care that if no clock has been created. It only makes sense
 * after an instance of [class@Clock] is created.
 *
 * Returns:An instance of [class@TimePoint] that reflects the start of the
 *         experiment.
 */
PsyTimePoint *
psy_time_point_new(void)
{
    PsyTimePoint *new = g_new0(PsyTimePoint, 1);
    return new;
}

/**
 * psy_time_point_new_monotonic:(constructor)
 * @monotonic_time:(in): A timevalue obtained with g_get_monotonic_time() or
 *                       some method that in synchronous to that function.
 *
 * GLib e.g. returns monotonic time with [func@GLib.get_monotonic_time]. You
 * can use this function to get a timepoint that is equivalent to psylib's
 * time. The time by psylib starts at 0 when the first clock is Instantiated.
 *
 * Returns: a [struct@imePoint] obtained from the monotonic clock of glib.
 */
PsyTimePoint *
psy_time_point_new_monotonic(gint64 monotonic_time)
{
    PsyTimePoint *new      = g_new0(PsyTimePoint, 1);
    gint64 zero_time       = psy_clock_get_zero_time();
    new->ticks_since_start = monotonic_time - zero_time;
    return new;
}

/**
 * psy_time_point_free:
 * @self: the instance of [struct@TimePoint] to destroy
 *
 * Frees timepoint that have previously created with psy_time_point_new(_x)
 * or have been retured by e.g. [method@Clock.now]
 */
void
psy_time_point_free(PsyTimePoint *self)
{
    g_free(self);
}

/**
 * psy_time_point_copy:
 * @self: An instance of [struct@TimePoint]
 *
 * Creates a new copy of self
 *
 * Returns:(transfer full): a new copy of @self.
 */
PsyTimePoint *
psy_time_point_copy(PsyTimePoint *self)
{
    PsyTimePoint *copy      = g_new(PsyTimePoint, 1);
    copy->ticks_since_start = self->ticks_since_start;
    return copy;
}

/**
 * psy_time_point_subtract:
 * @self: An instance of [struct@TimePoint]
 * @other: An instance of [struct@TimePoint]
 *
 * Computes the difference/duration between two timepoints
 *
 * Returns:(transfer full): The [struct@Duration] between two `PsyTimePoints`
 *                          Or NULL when the operation overflows.
 */
PsyDuration *
psy_time_point_subtract(PsyTimePoint *self, PsyTimePoint *other)
{
    g_return_val_if_fail(self != NULL, NULL);
    g_return_val_if_fail(other != NULL, NULL);

    gint64   us_result;
    gboolean over_or_underflows = psy_safe_sub_gint64(
        self->ticks_since_start, other->ticks_since_start, &us_result);
    g_return_val_if_fail(!over_or_underflows, NULL);

    return psy_duration_new_us(us_result);
}

/**
 * psy_time_point_subtract_dur:
 * @self: An instance of [struct@TimePoint]
 * @dur: An instance of [struct@Duration]
 *
 * Computes the new PsyTimePoint by subtracting a duration from @self
 *
 * Returns:(transfer full): The [struct@TimePoint] that is the result of
 *                          @self - @dur. Or NULL when the operation overflows.
 */
PsyTimePoint *
psy_time_point_subtract_dur(PsyTimePoint *self, PsyDuration *dur)
{
    g_return_val_if_fail(self != NULL, NULL);
    g_return_val_if_fail(dur != NULL, NULL);

    gint64 new_ticks, ticks, us;
    us    = psy_duration_get_us(dur);
    ticks = self->ticks_since_start;

    gboolean over_or_under_flows;
    over_or_under_flows = psy_safe_sub_gint64(ticks, us, &new_ticks);

    g_return_val_if_fail(!over_or_under_flows, NULL);
    PsyTimePoint *tret      = g_new(PsyTimePoint, 1);
    tret->ticks_since_start = new_ticks;

    return tret;
}

/**
 * psy_time_point_add:
 * @self: An instance of [struct@TimePoint]
 * @dur: An instance of [struct@Duration]
 *
 * Computes the the new PsyTimePoint by adding a duration to @self.
 *
 * Returns:(transfer full): The [struct@TimePoint] that is the result of
 *                          @self + @dur Or NULL when the operation
 *                          overflows.
 */
PsyTimePoint *
psy_time_point_add(PsyTimePoint *self, PsyDuration *dur)
{
    g_return_val_if_fail(self != NULL, NULL);
    g_return_val_if_fail(dur != NULL, NULL);

    gint64 us        = psy_duration_get_us(dur);
    gint64 new_ticks = 0;

    gboolean over_or_under_flows
        = psy_safe_add_gint64(self->ticks_since_start, us, &new_ticks);
    g_return_val_if_fail(!over_or_under_flows, NULL);

    PsyTimePoint *tp      = g_new(PsyTimePoint, 1);
    tp->ticks_since_start = new_ticks;
    return tp;
}

/**
 * psy_time_point_duration_since_start:
 * @self: An instance of [struct@TimePoint]
 *
 * A timepoint should refect the time between the timepoint and the start
 * of the PsyClock. The PsyClock start is roughly 0 the PsyClock type is
 * initialized.
 *
 * Returns:(transfer full): a [struct@Duration] that reflects the time when the
 * first clock is loaded.
 */
PsyDuration *
psy_time_point_duration_since_start(PsyTimePoint *self)
{
    g_return_val_if_fail(self != NULL, NULL);
    PsyTimePoint *tzero = psy_time_point_new();

    PsyDuration *dur = psy_time_point_subtract(self, tzero);

    psy_time_point_free(tzero);
    return dur;
}

/**
 * psy_time_point_less:
 * @self: An instance of #PsyTimePoint.
 * @other: An instance of #PsyTimePoint.
 *
 * Returns: TRUE when @self < @other, FALSE otherwise
 */
gboolean
psy_time_point_less(PsyTimePoint *self, PsyTimePoint *other)
{
    g_return_val_if_fail(self != NULL && other != NULL, FALSE);
    return self->ticks_since_start < other->ticks_since_start;
}

/**
 * psy_time_point_less_equal:
 * @self: An instance of #PsyTimePoint.
 * @other: An instance of #PsyTimePoint.
 *
 * Returns: TRUE when @self <= @other, FALSE otherwise
 */
gboolean
psy_time_point_less_equal(PsyTimePoint *self, PsyTimePoint *other)
{
    return psy_time_point_less(self, other)
           || psy_time_point_equal(self, other);
}

/**
 * psy_time_point_equal:
 * @self: An instance of #PsyTimePoint.
 * @other: An instance of #PsyTimePoint.
 *
 * Returns: TRUE when @self == @other, FALSE otherwise
 */
gboolean
psy_time_point_equal(PsyTimePoint *self, PsyTimePoint *other)
{
    g_return_val_if_fail(self != NULL && other != NULL, FALSE);
    return self->ticks_since_start == other->ticks_since_start;
}

/**
 * psy_time_point_not_equal:
 * @self: An instance of #PsyTimePoint.
 * @other: An instance of #PsyTimePoint.
 *
 * Returns: TRUE when @self != @other, FALSE otherwise
 */
gboolean
psy_time_point_not_equal(PsyTimePoint *self, PsyTimePoint *other)
{
    return !psy_time_point_equal(self, other);
}

/**
 * psy_time_point_greater_equal:
 * @self: An instance of #PsyTimePoint.
 * @other: An instance of #PsyTimePoint.
 *
 * Returns: TRUE when @self >= @other, FALSE otherwise
 */
gboolean
psy_time_point_greater_equal(PsyTimePoint *self, PsyTimePoint *other)
{
    return psy_time_point_equal(self, other)
           || !psy_time_point_less(self, other);
}

/**
 * psy_time_point_greater:
 * @self: An instance of #PsyTimePoint.
 * @other: An instance of #PsyTimePoint.
 *
 * Returns: TRUE when @self > @other, FALSE otherwise
 */
gboolean
psy_time_point_greater(PsyTimePoint *self, PsyTimePoint *other)
{
    return !psy_time_point_equal(self, other)
           && !psy_time_point_less(self, other);
}
