
#include "psy-stimulus.h"
#include "psy-time-point.h"
#include "psy-timer.h"

typedef struct PsyStimulusPrivate {
    PsyTimePoint *start_time;
    PsyDuration  *duration;
    PsyTimer     *timer;
    guint         is_started  : 1;
    guint         is_finished : 1;
} PsyStimulusPrivate;

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE(PsyStimulus, psy_stimulus, G_TYPE_OBJECT)

typedef enum {
    PROP_NULL, // Unused required by gobject
    PROP_START_TIME,
    PROP_STOP_TIME,
    PROP_DURATION,
    PROP_IS_STARTED,
    PROP_IS_FINISHED,
    NUM_PROPERTIES
} StimulusProperty;

typedef enum { SIG_STARTED, SIG_STOPPED, NUM_SIGNALS } StimulusSignals;

static GParamSpec *stimulus_properties[NUM_PROPERTIES] = {0};
static guint       stimulus_signals[NUM_SIGNALS]       = {0};

static void
psy_stimulus_dispose(GObject *object)
{
    PsyStimulusPrivate *priv
        = psy_stimulus_get_instance_private(PSY_STIMULUS(object));

    g_clear_object(&priv->timer);

    G_OBJECT_CLASS(psy_stimulus_parent_class)->dispose(object);
}

static void
psy_stimulus_finalize(GObject *self)
{
    PsyStimulusPrivate *priv
        = psy_stimulus_get_instance_private(PSY_STIMULUS(self));

    psy_duration_free(priv->duration);
    psy_time_point_free(priv->start_time);
}

static void
psy_stimulus_timer_fired(PsyTimer *timer, PsyTimePoint *tp, gpointer data)
{
    (void) timer;
    PsyStimulus *stimulus   = PSY_STIMULUS(data);
    gboolean     is_started = psy_stimulus_get_is_started(stimulus);

    if (!is_started) {
        psy_stimulus_set_is_started(stimulus, tp);
    }
    else {
        psy_stimulus_set_is_finished(stimulus, tp);
    }
}

static void
psy_stimulus_set_property(GObject      *object,
                          guint         property_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
    PsyStimulus *self = PSY_STIMULUS(object);

    switch ((StimulusProperty) property_id) {
    case PROP_START_TIME:
        psy_stimulus_play(self, g_value_get_boxed(value));
        break;
    case PROP_DURATION:
        psy_stimulus_set_duration(self, g_value_get_boxed(value));
        break;
    case PROP_IS_STARTED: // Only get properties
    case PROP_IS_FINISHED:
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static void
psy_stimulus_get_property(GObject    *object,
                          guint       property_id,
                          GValue     *value,
                          GParamSpec *pspec)
{
    PsyStimulus        *self = PSY_STIMULUS(object);
    PsyStimulusPrivate *priv = psy_stimulus_get_instance_private(self);
    switch ((StimulusProperty) property_id) {
    case PROP_START_TIME:
        g_value_set_boxed(value, priv->start_time);
        break;
    case PROP_STOP_TIME:
        g_value_set_boxed(value, psy_stimulus_get_stop_time(self));
        break;
    case PROP_DURATION:
        g_value_set_boxed(value, priv->duration);
        break;
    case PROP_IS_STARTED:
        g_value_set_boolean(value, priv->is_started);
        break;
    case PROP_IS_FINISHED:
        g_value_set_boolean(value, priv->is_finished);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static void
psy_stimulus_init(PsyStimulus *self)
{
    PsyStimulusPrivate *priv = psy_stimulus_get_instance_private(self);
    priv->timer              = psy_timer_new();

    g_signal_connect(
        priv->timer, "fired", G_CALLBACK(psy_stimulus_timer_fired), self);
}

static void
stimulus_play(PsyStimulus *stim, PsyTimePoint *tp)
{
    PsyStimulusPrivate *priv = psy_stimulus_get_instance_private(stim);

    if (priv->start_time)
        psy_time_point_free(priv->start_time);

    priv->start_time = psy_time_point_copy(tp);

    psy_timer_set_fire_time(priv->timer, priv->start_time);
}

static void
stimulus_set_duration(PsyStimulus *stim, PsyDuration *dur)
{
    PsyStimulusPrivate *priv = psy_stimulus_get_instance_private(stim);
    if (priv->duration)
        psy_duration_free(priv->duration);
    priv->duration = psy_duration_copy(dur);
}

static void
psy_stimulus_class_init(PsyStimulusClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    object_class->set_property = psy_stimulus_set_property;
    object_class->get_property = psy_stimulus_get_property;
    object_class->dispose      = psy_stimulus_dispose;
    object_class->finalize     = psy_stimulus_finalize;

    klass->play         = stimulus_play;
    klass->set_duration = stimulus_set_duration;

    /**
     * Stimulus:start-time:
     *
     * This specifies when the stimulus is going to be presented. In case
     * of most stimuli, the framework will try to make a small adjustment
     * to the start time. Eg When the frame rate of a window/monitor is
     * 60 Hz a fresh screen can be presented every 16.666.. ms. Hence,
     * the start time will be reset to the nearest vblanking interval.
     */
    stimulus_properties[PROP_START_TIME]
        = g_param_spec_boxed("start-time",
                             "StartTime",
                             "The time at which the stimulus started.",
                             PSY_TYPE_TIME_POINT,
                             G_PARAM_READABLE);

    /**
     * Stimulus:stop-time:
     *
     * This specifies stimulus will stop or has stopped. Similar to the start
     * time, the stop time might be adjusted by the framework in order to
     * meet match the physical properties of the stimulus device e.g. sound
     * card or monitor.
     */
    stimulus_properties[PROP_STOP_TIME]
        = g_param_spec_boxed("stop-time",
                             "StopTime",
                             "The time at which the stimulus stopped.",
                             PSY_TYPE_TIME_POINT,
                             G_PARAM_READABLE);

    /**
     * Stimulus:duration:
     *
     * This is the duration between the #PsyStimulus:stop-time and
     * #PsyStimulus:start-time. You can use this property to specify the
     * #PsyStimulus:stop-time, when you set it. In contrast when you read it,
     * it will be calculated as #PsyStimulus:stop-time -
     * #PsyStimulus:start-time.
     */
    stimulus_properties[PROP_DURATION]
        = g_param_spec_boxed("duration",
                             "Duration",
                             "The desired duration of the stimulus",
                             PSY_TYPE_DURATION,
                             G_PARAM_READWRITE);

    /**
     * PsyStimulus:is-started:
     *
     * A boolean that is set once the stimulus has started.
     */
    stimulus_properties[PROP_IS_STARTED]
        = g_param_spec_boolean("is-started",
                               "started",
                               "whether the stimulus has begun",
                               FALSE,
                               G_PARAM_READABLE);

    /**
     * PsyStimulus:is-finished:
     *
     * A boolean that is set once the stimulus has finished.
     */
    stimulus_properties[PROP_IS_FINISHED]
        = g_param_spec_boolean("finished",
                               "Finished",
                               "Whether the stimulus has finished.",
                               FALSE,
                               G_PARAM_READABLE);

    g_object_class_install_properties(
        object_class, NUM_PROPERTIES, stimulus_properties);

    /**
     * Stimulus::started:
     *
     * stimulus: The stimulus that received the signal.
     * start:(transfer None):The start time of the stimulus
     */
    stimulus_signals[SIG_STARTED]
        = g_signal_new("started",
                       PSY_TYPE_STIMULUS,
                       G_SIGNAL_RUN_FIRST,
                       G_STRUCT_OFFSET(struct _PsyStimulusClass, started),
                       NULL,
                       NULL,
                       NULL,
                       G_TYPE_NONE,
                       1,
                       PSY_TYPE_TIME_POINT);

    /**
     * PsyStimulus::stopped:
     * @stimulus: the [class@Stimulus] that received the signal.
     * @stop_time: (transfer none): A [struct@TimePoint] at which the stimulus
     * stopped.
     *
     * The stopped signal is emitted when the stimulus has just stopped. The
     * stop time is the deduced time when the stimulus has actually stopped,
     * this might be a little before or after the stop signal is actually
     * emitted.
     */
    stimulus_signals[SIG_STOPPED]
        = g_signal_new("stopped",
                       PSY_TYPE_STIMULUS,
                       G_SIGNAL_RUN_LAST,
                       G_STRUCT_OFFSET(PsyStimulusClass, stopped),
                       NULL,
                       NULL,
                       NULL,
                       G_TYPE_NONE,
                       1,
                       PSY_TYPE_TIME_POINT);
}

/**
 * psy_stimulus_play:
 * @self: A #PsyStimulus instance.
 * @start_time:(transfer none): A [struct@TimePoint] at which the stimulus
 *                              should start.
 *
 * This method schedules a stimulus at a given start time. It is well possible
 * that the stimulus can not be started precisely at that time. You can
 * examine the #PsyStimulus:start_time property to see the when the stimulus
 * is going to start.
 */
void
psy_stimulus_play(PsyStimulus *self, PsyTimePoint *start_time)
{
    g_return_if_fail(PSY_IS_STIMULUS(self));
    g_return_if_fail(start_time != NULL);

    PsyStimulusClass *klass = PSY_STIMULUS_GET_CLASS(self);

    g_return_if_fail(klass->play);

    klass->play(self, start_time);
}

/**
 * psy_stimulus_play_for:
 * @self: A [class@Stimulus] instance
 * @start_time:(transfer none): The intended start time of the stimulus
 * @duration(transfer none):   The intended duration of the stimulus
 *
 * This is short hand for psy_stimulus_play(self, start_time) and
 * psy_stimulus_stop(self, start_time + dur);
 *
 */
void
psy_stimulus_play_for(PsyStimulus  *self,
                      PsyTimePoint *start_time,
                      PsyDuration  *dur)
{
    g_return_if_fail(PSY_IS_STIMULUS(self));
    g_return_if_fail(start_time != NULL);
    g_return_if_fail(dur != NULL);

    psy_stimulus_set_duration(self, dur);
    psy_stimulus_play(self, start_time);
}

/**
 * psy_stimulus_play_until:
 * @self: A #PsyStimulus instance.
 * @start_time: A #PsyTimePoint instance specifying a time point in the future
 *              when the stimulus in desired to start.
 * @stop_time:  A #PsyTimePoint instance specifying a time point in the future
 *              when the stimulus in desired to stop.
 *
 * this is short hand for doing:
 * psy_stimulus_play(self, start_time);
 * psy_stimulus_stop(self, stop_time);
 */
void
psy_stimulus_play_until(PsyStimulus  *self,
                        PsyTimePoint *start_time,
                        PsyTimePoint *stop_time)
{
    g_return_if_fail(PSY_IS_STIMULUS(self));
    g_return_if_fail(start_time != NULL);
    g_return_if_fail(stop_time != NULL);

    psy_stimulus_stop(self, stop_time);
    psy_stimulus_play(self, start_time);
}

/**
 * psy_stimulus_stop:
 * @self: The stimulus that should stop.
 * @stop_time: A timepoint at which the stimulus should stop.
 *
 * Set the stop time of the stimulus.
 */
void
psy_stimulus_stop(PsyStimulus *self, PsyTimePoint *stop_time)
{
    g_return_if_fail(PSY_IS_STIMULUS(self));
    g_return_if_fail(stop_time != NULL);
    PsyStimulusClass *klass = PSY_STIMULUS_GET_CLASS(self);
    g_return_if_fail(klass->set_duration);

    PsyStimulusPrivate *priv = psy_stimulus_get_instance_private(self);
    if (psy_time_point_less(stop_time, priv->start_time)) {
        g_warning("Specified stop time is before the start_time.");
        return;
    }
    PsyDuration *dur = psy_time_point_subtract(stop_time, priv->start_time);
    klass->set_duration(self, dur);
    psy_duration_free(dur);
}

/**
 * psy_stimulus_get_start_time:
 * @self: A #PsyStimulus instance
 *
 * Get the best estimation of when a stimulus is about to start or has started.
 *
 * Returns:(transfer none): A #PsyTimePoint describing when the stimulus will
 *     start or was started if it is in the past. Or NULL when the start time
 *     is still not decided on.
 */
PsyTimePoint *
psy_stimulus_get_start_time(PsyStimulus *self)
{
    g_return_val_if_fail(PSY_IS_STIMULUS(self), NULL);

    PsyStimulusPrivate *priv = psy_stimulus_get_instance_private(self);
    return priv->start_time;
}

/**
 * psy_stimulus_get_stop_time:
 * @self: A #PsyStimulus instance
 *
 * Get the best estimation of when a stimulus is about to stop or has stopped.
 *
 * Returns:(transfer full)(nullable): An [struct@TimePoint] describing when the
 * stimulus will stop or was stopped if it is in the past. Or NULL when the stop
 * time is still not decided on.
 */
PsyTimePoint *
psy_stimulus_get_stop_time(PsyStimulus *self)
{
    g_return_val_if_fail(PSY_IS_STIMULUS(self), NULL);
    PsyStimulusPrivate *priv = psy_stimulus_get_instance_private(self);

    if (!priv->duration || priv->start_time == NULL)
        return NULL;

    return psy_time_point_add(priv->start_time, priv->duration);
}

/**
 * psy_stimulus_set_duration:
 * @self: A `PsyStimulus` instance.
 * @duration:(transfer none): An [struct@Duration] instance that tells the
 *            framework for how long this stimulus should be presented.
 *
 * Sets the duration for the stimulus, in order to succeed the starting
 * time should be known.
 */
void
psy_stimulus_set_duration(PsyStimulus *self, PsyDuration *duration)
{
    g_return_if_fail(PSY_IS_STIMULUS(self));
    g_return_if_fail(psy_duration_get_us(duration) > 0);

    PsyStimulusClass *klass = PSY_STIMULUS_GET_CLASS(self);
    g_return_if_fail(klass->set_duration);

    klass->set_duration(self, duration);
}

/**
 * psy_stimulus_get_duration:
 * @self : a PsyStimulus instance.
 *
 * A duration of a stimulus is a "virtual" property, its the differnce
 * between the start and end time. Hence, both must be known, otherwise
 * NULL is returned.
 *
 * Returns:(transfer none): An instance of `PsyDuration` or NULL when the
 *                          duration isn't set.
 */
PsyDuration *
psy_stimulus_get_duration(PsyStimulus *self)
{
    g_return_val_if_fail(PSY_IS_STIMULUS(self), NULL);

    PsyStimulusPrivate *priv = psy_stimulus_get_instance_private(self);
    return priv->duration;
}

/**
 * psy_stimulus_get_is_started:
 *
 * Returns: #TRUE if the stimulus is started #FALSE otherwise.
 */
gboolean
psy_stimulus_get_is_started(PsyStimulus *self)
{
    PsyStimulusPrivate *priv = psy_stimulus_get_instance_private(self);
    g_return_val_if_fail(PSY_IS_STIMULUS(self), FALSE);

    return priv->is_started;
}

/**
 * psy_stimulus_set_is_started:
 * @self: An instance of `PsyStimulus` being started.
 * @start_time: An instance of [struct@PsyTimePoint] when the stimulus should be
 * visible.
 *
 * Emits the [signal@Stimulus::started] signal, It sets the
 * [property@Stimulus:is-started] and clears the
 * [property@Stimulus:finished] property.
 *
 * If an end duration is specified, the timer will be set to fire at the
 * predicted end time of the stimulus.
 *
 * stability:private
 */
void
psy_stimulus_set_is_started(PsyStimulus *self, PsyTimePoint *start_time)
{
    g_return_if_fail(PSY_IS_STIMULUS(self));

    PsyStimulusPrivate *priv = psy_stimulus_get_instance_private(self);

    priv->is_started  = 1;
    priv->is_finished = 0;

    g_signal_emit(self, stimulus_signals[SIG_STARTED], 0, start_time);

    PsyTimePoint *stop_time = NULL; // owned

    if (priv->duration) {
        stop_time = psy_time_point_add(start_time, priv->duration);
    }

    if (stop_time) {
        psy_timer_set_fire_time(priv->timer, stop_time);
        psy_time_point_free(stop_time);
    }
}

/**
 * psy_stimulus_get_is_finished:
 */
gboolean
psy_stimulus_get_is_finished(PsyStimulus *self)
{
    PsyStimulusPrivate *priv = psy_stimulus_get_instance_private(self);
    g_return_val_if_fail(PSY_IS_STIMULUS(self), FALSE);

    return priv->is_finished;
}

/**
 * psy_stimulus_set_is_finished:
 * @self: An instance of [class@Stimulus] that is about to finish.
 * @stop_time:(transfer none): An instance of [struct@TimePoint] when the
 *            stimulus should be done.
 *
 * stability:private
 */
void
psy_stimulus_set_is_finished(PsyStimulus *self, PsyTimePoint *stop_time)
{
    g_return_if_fail(PSY_IS_STIMULUS(self));

    PsyStimulusPrivate *priv = psy_stimulus_get_instance_private(self);
    priv->is_finished        = 1;

    g_signal_emit(self, stimulus_signals[SIG_STOPPED], 0, stop_time);
}
