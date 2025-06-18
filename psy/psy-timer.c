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

void
fire_data_free(FireData *data)
{
    psy_time_point_free(data->fire_time);
    g_free(data);
}

/* forward declarations */

guint
psy_timer_get_source_id(PsyTimer *self);

typedef struct _PsyTimer {
    GObject       parent;
    GMainContext *context;
    PsyTimePoint *fire_time;

    GAsyncQueue *queue;

    PsyTimerAsyncCb callback;
    gpointer        callback_data;

    guint  source_id;
    GMutex mutex; // protects source_id;

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
    g_debug("Timer %p, %s", (void *) self, __func__);
    self->context = g_main_context_get_thread_default();
    self->queue   = g_async_queue_new();
}

static void
timer_dispose(GObject *obj)
{
    PsyTimer *self = PSY_TIMER(obj);
    g_debug("Timer %p, %s", (void *) self, __func__);

    psy_timer_cancel(self);

    guint source_id = psy_timer_get_source_id(self);
    if (source_id) {
        GSource *source
            = g_main_context_find_source_by_id(self->context, source_id);
        g_source_destroy(source);
    }

    G_OBJECT_CLASS(psy_timer_parent_class)->dispose(obj);
}

static void
timer_finalize(GObject *self)
{
    g_debug("Timer %p, %s", (void *) self, __func__);
    PsyTimer *timer_self = PSY_TIMER(self);

    g_clear_pointer(&timer_self->fire_time, psy_time_point_free);
    g_clear_pointer(&timer_self->queue, g_async_queue_unref);

    g_mutex_clear(&timer_self->mutex);

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
    g_debug("Timer %p: %s", (void *) self, __func__);
    g_return_if_fail(PSY_IS_TIMER(self));

    g_clear_pointer(&self->fire_time, psy_time_point_free);

    g_signal_emit(self, timer_signals[SIG_FIRED], 0, tp);
}

static gboolean
thread_default_fire(FireData *data)
{
    g_debug("Timer %p FireData %p: %s",
            (void *) data->timer,
            (void *) data,
            __func__);
    psy_timer_emit_fire(data->timer, data->fire_time);

    psy_timer_set_source_id(data->timer, 0);

    return G_SOURCE_REMOVE;
}

static void
psy_timer_class_init(PsyTimerClass *klass)
{
    GObjectClass *obj_class = G_OBJECT_CLASS(klass);

    obj_class->get_property = timer_get_property;
    obj_class->set_property = timer_set_property;
    obj_class->dispose      = timer_dispose;
    obj_class->finalize     = timer_finalize;

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
        g_debug("Timer: %p, %s: canceling self", (void *) self, __func__);
        psy_timer_cancel(self);
    }

    if (tp) {
        self->fire_time = psy_time_point_copy(tp);
        g_debug("Timer: %p, %s: add timer to thread", (void *) self, __func__);
        timer_private_add_timer(self);
    }
    else {
        g_debug("Timer: %p, %s: clearing fire_time", (void *) self, __func__);
        self->fire_time = NULL;
    }
}

/**
 * psy_timer_get_fire_time:
 * @self: an instance of [class@Timer]
 *
 * Gets the timepoint when this timer is set
 *
 * Returns:(nullable)(transfer full):The time for when this timer is/was
 * set.
 */
PsyTimePoint *
psy_timer_get_fire_time(PsyTimer *self)
{
    g_return_val_if_fail(PSY_IS_TIMER((PsyTimer *) self), NULL);

    return self->fire_time != NULL ? psy_time_point_copy(self->fire_time)
                                   : NULL;
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

    g_debug("Timer %p,%s: request thread to cancel timer, source_id = %u",
            (void *) self,
            __func__,
            self->source_id);
    timer_private_cancel_timer(self);
    // Check whether the thread has already issued a fire
    g_clear_handle_id(&self->source_id, g_source_remove);
    g_clear_pointer(&self->fire_time, psy_time_point_free);
}

/**
 * psy_timer_set_async_fire_cb:
 * @cb:(nullable)(closure data)(scope forever): a callback to be called
 * @data:(nullable): the data passed to the callback
 *
 * You may only set this member when the fire-time is not yet set.
 * This callback is called from the timer thread, hence, you must take care not
 * to run in any thread related issues when operation on/with data. You should
 * only call this function when its fire time isn't set yet, as that could
 * complicate stuff.
 * The thread that monitors the timers will first undertake the steps to emit
 * the fired signal, as that is guaranteed pretty quickly. Only then it will
 * call this callback asynchronously. It is advised that this callback is non
 * blocking, as a blocking callback will mess with other timer scheduled .
 *
 * Returns: TRUE when the callback was successfully set.
 */
gboolean
psy_timer_set_async_fire_cb(PsyTimer *self, PsyTimerAsyncCb cb, gpointer data)
{
    g_return_val_if_fail(PSY_IS_TIMER(self), FALSE);

    if (G_UNLIKELY(self->fire_time)) { // cancel ongoing operations first
        g_warning("Unable to set callback when timer is already scheduled.");
        return FALSE;
    }

    self->callback      = cb;
    self->callback_data = data;

    return TRUE;
}

/**
 * psy_timer_fire:(skip)
 * @self, the timer to fire
 * @tp: The timepoint at which the timer should be fired
 *
 * Fire the timer in the thread default context at the time the timer was
 * created.
 *
 * Stability: private
 */
void
psy_timer_fire(PsyTimer *self, PsyTimePoint *tp)
{
    FireData *data = g_new(FireData, 1);
    g_debug(
        "Timer %p, %s, FireData %p", (void *) self, __func__, (void *) data);

    data->fire_time = psy_time_point_copy(tp);
    data->timer     = self;

    // This doesn't work as we need to be able to destroy the source id when the
    // timer is canceled.
    //
    //    g_main_context_invoke_full(self->context,
    //                               G_PRIORITY_DEFAULT,
    //                               G_SOURCE_FUNC(thread_default_fire),
    //                               data,
    //                               (GDestroyNotify) fire_data_free);

    GSource *source = g_idle_source_new();
    g_source_set_callback(source,
                          G_SOURCE_FUNC(thread_default_fire),
                          data,
                          (GDestroyNotify) fire_data_free);
    self->source_id = g_source_attach(source, self->context);

    g_source_unref(source);
}

/**
 * psy_timer_fire_async_cb:(skip)
 *
 * fires the async callback
 */
void
psy_timer_fire_async_cb(PsyTimer *self, PsyTimePoint *tp)
{
    g_return_if_fail(PSY_IS_TIMER(self));

    if (self->callback) {
        self->callback(tp, self->callback_data);
    }
}

/**
 * psy_timer_get_queue:(skip)
 * @self: the timer
 *
 * Returns: the internal [struct@GLib.AsyncQueue]
 * Stability: private
 */
GAsyncQueue *
psy_timer_get_queue(PsyTimer *self)
{
    g_return_val_if_fail(PSY_IS_TIMER(self), NULL);

    return self->queue;
}

/**
 * psy_timer_set_source_id:(skip)
 * @self: the timer on which the source must be deleted.
 * @source_id: the id to be removed from the mainloop.
 *
 * For internal use only. If the timer thread has dispached a source to the
 * timer it must be freed when the timer is disposed.
 */
void
psy_timer_set_source_id(PsyTimer *self, guint source_id)
{
    g_return_if_fail(PSY_IS_TIMER(self));

    g_mutex_lock(&self->mutex);
    self->source_id = source_id;
    g_mutex_unlock(&self->mutex);
}

/**
 * psy_timer_get_source_id:(skip)
 * @self: the timer on which the source must be deleted.
 * @source_id: the id to be removed from the mainloop.
 *
 * For internal use only. If the timer thread has dispatched a source to the
 * timer it must be freed when the timer is disposed.
 */
guint
psy_timer_get_source_id(PsyTimer *self)
{
    g_return_val_if_fail(PSY_IS_TIMER(self), 0);
    guint ret;
    g_mutex_lock(&self->mutex);
    ret = self->source_id;
    g_mutex_unlock(&self->mutex);
    return ret;
}
