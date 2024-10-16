/**
 * PsyTimer:
 *
 * A timer can be set for a specific timepoint. At this timepoint the
 * timer will be fired.
 * Timer instances are tied to a specific GMainContext. The main context will
 * receive a message for this TimePoint, at that time the Timer will emit its
 * fire signal in the GMainContext that the timer was created. So the main
 * purpose of this timer is to get a signal at a very specific time, so that the
 * caller can do something at this time.
 */

#include "psy-timer.h"
#include "psy-time-point.h"
#include "psy-timer-private.h"

typedef struct FireData {
    PsyTimer     *timer;
    PsyTimePoint *fire_time;
} FireData;

typedef struct _PsyTimer {
    GObject       parent;
    GMainContext *context;
    PsyTimePoint *fire_time;
    guint         fire_callback_id;
} PsyTimer;

typedef enum {
    PROP_NULL,
    PROP_CONTEXT,
    PROP_FIRE_TIME,
    NUM_PROPERIES
} PsyTimerProperty;

typedef enum { SIG_FIRED, NUM_SIGNALS } PsyTimerSignal;

G_DEFINE_TYPE(PsyTimer, psy_timer, G_TYPE_OBJECT)

static GParamSpec *timer_properties[NUM_PROPERIES];
static guint       timer_signals[NUM_SIGNALS];

static void
psy_timer_init(PsyTimer *self)
{
    self->context = g_main_context_get_thread_default();

    // Make sure that a timer thread is running.
    timer_private_start_timer_thread();
}

static void
timer_finalize(GObject *self)
{
    PsyTimer *timer_self = PSY_TIMER(self);

    g_clear_pointer(&timer_self->fire_time, psy_time_point_free);

    if (timer_self->fire_callback_id) {
        GSource *source = g_main_context_find_source_by_id(
            PSY_TIMER(self)->context, timer_self->fire_callback_id);
        if (source) {
            g_source_destroy(source);
        }
    }

    // closes timer thread if we are the last timer standing
    timer_private_stop_timer_thread();

    // chainup to parent.
    G_OBJECT_CLASS(psy_timer_parent_class)->finalize(self);
}

static void
timer_get_property(GObject    *object,
                   guint       property_id,
                   GValue     *value,
                   GParamSpec *pspec)
{
    PsyTimer *self = PSY_TIMER(object);

    switch ((PsyTimerProperty) property_id) {
    case PROP_FIRE_TIME:
        g_value_set_boxed(value, self->fire_time);
        break;
    case PROP_CONTEXT:
        g_value_set_boxed(value, self->context);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

static void
timer_set_property(GObject      *object,
                   guint         property_id,
                   const GValue *value,
                   GParamSpec   *pspec)
{
    PsyTimer *self = PSY_TIMER(object);

    switch ((PsyTimerProperty) property_id) {
    case PROP_FIRE_TIME:
        psy_timer_set_fire_time(self, g_value_get_boxed(value));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

void
psy_timer_emit_fire(PsyTimer *self, PsyTimePoint *tp)
{
    g_return_if_fail(PSY_IS_TIMER(self));

    g_clear_pointer(&self->fire_time, psy_time_point_free);

    g_signal_emit(self, timer_signals[SIG_FIRED], 0, tp);
}

static gboolean
thread_default_fire(FireData *data)
{
    psy_timer_emit_fire(data->timer, data->fire_time);
    psy_time_point_free(data->fire_time);

    g_assert(data->timer->fire_callback_id != 0);
    data->timer->fire_callback_id = 0;

    return G_SOURCE_REMOVE;
}

void
psy_timer_fire(PsyTimer *self, PsyTimePoint *tp)
{
    FireData *data = g_new(FireData, 1);

    data->fire_time = psy_time_point_copy(tp);
    data->timer     = self;

    GSource *source = g_idle_source_new();
    g_source_set_callback(
        source, G_SOURCE_FUNC(thread_default_fire), data, g_free);
    self->fire_callback_id = g_source_attach(source, self->context);
    g_source_unref(source);
}

static void
psy_timer_class_init(PsyTimerClass *klass)
{
    GObjectClass *obj_class = G_OBJECT_CLASS(klass);

    obj_class->get_property = timer_get_property;
    obj_class->set_property = timer_set_property;

    obj_class->finalize = timer_finalize;

    /**
     * PsyTimer:fire-time:
     *
     * An instance of [struct@TimePoint] that is the time that describes when
     * this timer should fire. Setting this property to a non NULL value, will
     * disable ongoing timers, and set a new time at which this property should
     * fire.
     */
    timer_properties[PROP_FIRE_TIME]
        = g_param_spec_boxed("fire-time",
                             "FireTime",
                             "The time at which this object should be fired.",
                             PSY_TYPE_TIME_POINT,
                             G_PARAM_READWRITE);

    /**
     * PsyTimer:context:
     *
     * An instance of [struct@Glib.MainContext], that was current when the
     * timer was created.
     */
    timer_properties[PROP_CONTEXT]
        = g_param_spec_boxed("context",
                             "Context",
                             "The thread default context at creation time of "
                             "the instance of [class@Timer]",
                             G_TYPE_MAIN_CONTEXT,
                             G_PARAM_READABLE);

    g_object_class_install_properties(
        obj_class, NUM_PROPERIES, timer_properties);

    /**
     * PsyTimer::fire:
     *
     * This signal is called in the [struct@Glib.MainContext] that was the
     * thread current context when the timer is created.
     */
    timer_signals[SIG_FIRED] = g_signal_new("fired",
                                            PSY_TYPE_TIMER,
                                            G_SIGNAL_RUN_LAST,
                                            0,
                                            NULL,
                                            NULL,
                                            NULL,
                                            G_TYPE_NONE,
                                            1,
                                            PSY_TYPE_TIME_POINT);
}

/**
 * psy_timer_new:(constructor)
 *
 * Create a new unarmed timer
 */
PsyTimer *
psy_timer_new(void)
{
    PsyTimer *timer = g_object_new(PSY_TYPE_TIMER, NULL);
    return timer;
}

/**
 * psy_timer_free:
 *
 * Frees a previously timer created with psy_timer_new
 */
void
psy_timer_free(PsyTimer *self)
{
    g_return_if_fail(PSY_IS_TIMER(self));

    g_object_unref(self);
}

/**
 * psy_timer_set_fire_time:
 * @self: an instance of [class@Timer], the timer to arm
 * @tp:(transfer none)(nullable): an instance of [struct@TimePoint], the
 * time point at which this timer should fire.
 *
 * This function sets a new time at which timer should be fired. if @tp ==
 * NULL means that you want to disable the timer.
 */
void
psy_timer_set_fire_time(PsyTimer *self, PsyTimePoint *tp)
{
    g_return_if_fail(PSY_IS_TIMER(self));

    if (self->fire_time) {
        psy_timer_cancel(self);
    }

    if (tp) {
        self->fire_time = psy_time_point_copy(tp);
        timer_private_set_timer(self);
    }
}

/**
 * psy_timer_get_fire_time:
 * @self: an instance of [class@Timer]
 *
 * Gets the timepoint when this timer is set
 *
 * Returns:(nullable)(transfer none):The time for when this timer is/was
 * set.
 */
PsyTimePoint *
psy_timer_get_fire_time(PsyTimer *self)
{
    g_return_val_if_fail(PSY_IS_TIMER((PsyTimer *) self), NULL);

    return self->fire_time;
}

/**
 * psy_timer_cancel:
 *
 * Cancels the arming of the timer
 */
void
psy_timer_cancel(PsyTimer *self)
{
    g_return_if_fail(PSY_IS_TIMER(self));

    if (!self->fire_time)
        return;

    timer_private_cancel_timer(self);
    g_clear_pointer(&self->fire_time, psy_time_point_free);
}
