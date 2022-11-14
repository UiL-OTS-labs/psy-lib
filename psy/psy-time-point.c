
#include "psy-time-point.h"
#include "psy-duration.h"
#include "psy-clock.h"
#include "psy-safe-int-private.h"

/**
 * PsyTimePoint:
 *
 * Instances of PsyTimePoint are generally obtained by
 * `psy_clock_now`, which returns the current time as the
 * number of microseconds since the first `PsyClock` is instantiated.
 * common other ways of obtaining PsyTimePoints is the result of
 * a computation between timepoints and durations. One can create a
 * timepoint with the constructor, but than the timepoint isn't really
 * meaning full.
 */

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

    /**
     * PsyTimePoint:num-ticks:
     *
     * This value represents the number of ticks since the first PsyClock
     * is created/since the type PsyClock is registered.
     */
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

/**
 * psy_time_point_new:
 * @monotonic_time:(in): A timevalue obtained with g_get_monotonic_time() or
 *                       some method that in synchronous to that function.
 *
 * Returns: a `PsyTimePoint` obtained from the monotonic clock of glib.
 */
PsyTimePoint*
psy_time_point_new(gint64 monotonic_time)
{
    gint zero_time = psy_clock_get_zero_time();
    gint num_ticks = monotonic_time - zero_time;
    return g_object_new(PSY_TYPE_TIME_POINT,
            "num_ticks", num_ticks,
            NULL);
}

/**
 * psy_time_point_new_copy:(constructor)
 * @other:(transfer none): the `PsyTimePoint` instance of which to make a copy
 *
 * Make a deep copy of a PsyTimePoint instance
 *
 * Returns: a new instance of `PsyTimePoint`
 */
PsyTimePoint*
psy_time_point_new_copy(PsyTimePoint* other)
{
    PsyTimePoint* tp = g_object_new(PSY_TYPE_TIME_POINT, NULL);
    tp->ticks_since_start = other->ticks_since_start;
    return tp;
}

/**
 * psy_time_point_subtract:
 * @self: An instance of `PsyTimePoint`
 * @other: An instance of `PsyTimePoint`
 *
 * Computes the difference/duration between two timepoints
 *
 * Returns:(transfer full): The `PsyDuration` between two `PsyTimePoints`
 *                          Or NULL when the operation overflows.
 */
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

/**
 * psy_time_point_subtract_dur:
 * @self: An instance of `PsyTimePoint`
 * @dur: An instance of `PsyDuration`
 *
 * Computes the the new PsyTimePoint by subtracting a duration from @self
 *
 * Returns:(transfer full): The `PsyTimePoint` that is the result of
 *                          @self - @dur. Or NULL when the operation overflows.
 */
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

/**
 * psy_time_point_add:
 * @self: An instance of `PsyTimePoint`
 * @dur: An instance of `PsyDuration`
 *
 * Computes the the new PsyTimePoint by adding a duration to @self.
 *
 * Returns:(transfer full): The `PsyTimePoint` that is the result of
 *                          @self + @dur Or NULL when the operation
 *                          overflows.
 */
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

/**
 * psy_time_point_duration_since_start:
 * @self: An instance of `PsyTimePoint`
 *
 * A timepoint should refect the time between the timepoint and the start
 * of the PsyClock. The PsyClock start is roughly 0 the PsyClock type is
 * initialized.
 *
 * Returns:(transfer full): a `PsyDuration` that reflects the time when the
 * first clock is loaded.
 */
PsyDuration*
psy_time_point_duration_since_start(PsyTimePoint* self)
{
    g_return_val_if_fail(PSY_IS_TIME_POINT(self), NULL);
    PsyTimePoint *tzero = g_object_new(PSY_TYPE_TIME_POINT, NULL);

    PsyDuration *dur = psy_time_point_subtract(self, tzero);

    g_object_unref(tzero);
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
psy_time_point_less(PsyTimePoint* self, PsyTimePoint* other)
{
    g_return_val_if_fail(PSY_IS_TIME_POINT(self) && PSY_IS_TIME_POINT(other), FALSE);
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
psy_time_point_less_equal(PsyTimePoint* self, PsyTimePoint* other)
{
    return psy_time_point_less(self, other) || psy_time_point_equal(self, other);
}

/**
 * psy_time_point_equal:
 * @self: An instance of #PsyTimePoint.
 * @other: An instance of #PsyTimePoint.
 *
 * Returns: TRUE when @self == @other, FALSE otherwise
 */
gboolean
psy_time_point_equal(PsyTimePoint* self, PsyTimePoint* other)
{
    g_return_val_if_fail(PSY_IS_TIME_POINT(self) && PSY_IS_TIME_POINT(other), FALSE);
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
psy_time_point_not_equal(PsyTimePoint* self, PsyTimePoint* other)
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
psy_time_point_greater_equal(PsyTimePoint* self, PsyTimePoint* other)
{
    return psy_time_point_equal(self, other) || !psy_time_point_less(self, other);
}

/**
 * psy_time_point_greater:
 * @self: An instance of #PsyTimePoint.
 * @other: An instance of #PsyTimePoint.
 *
 * Returns: TRUE when @self > @other, FALSE otherwise
 */
gboolean
psy_time_point_greater(PsyTimePoint *self, PsyTimePoint* other)
{
    return !psy_time_point_equal(self, other) && !psy_time_point_less(self, other);
}
