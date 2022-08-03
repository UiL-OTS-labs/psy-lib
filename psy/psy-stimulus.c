
#include "psy-stimulus.h"

typedef struct PsyStimulusPrivate {
    PsyTimePoint *start_time;
    PsyTimePoint *stop_time;
} PsyStimulusPrivate;

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE(PsyStimulus, psy_stimulus, G_TYPE_OBJECT)

typedef enum {
    PROP_NULL,  // Unused required by gobject
    PROP_START_TIME,
    PROP_STOP_TIME,
    PROP_DURATION,
    NUM_PROPERTIES
} StimulusProperty;

typedef enum {
    SIG_STARTED,
    SIG_STOPPED,
    NUM_SIGNALS
}StimulusSignals;

static GParamSpec  *stimulus_properties[NUM_PROPERTIES] = {0};
static guint        stimulus_signals[NUM_SIGNALS]       = {0};

static void
psy_stimulus_set_property (GObject      *object,
                           guint         property_id,
                           const GValue *value,
                           GParamSpec   *pspec)
{
    PsyStimulus *self = PSY_STIMULUS(object);
    PsyStimulusPrivate *priv = psy_stimulus_get_instance_private(self);

    switch ((StimulusProperty) property_id) {
        case PROP_START_TIME:
            psy_stimulus_play(self, g_value_get_object(value));
            break;
        case PROP_STOP_TIME:
            psy_stimulus_play_until(self,
                                    priv->start_time,
                                    g_value_get_object(value));
            break;
        case PROP_DURATION:
            psy_stimulus_play_for(self,
                                  priv->start_time,
                                  g_value_get_object(value));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
            break;
    }
}

static void
psy_stimulus_get_property (GObject      *object,
                           guint         property_id,
                           GValue       *value,
                           GParamSpec   *pspec)
{
    PsyStimulus *self = PSY_STIMULUS(object);
    PsyStimulusPrivate * priv = psy_stimulus_get_instance_private(self);
    switch ((StimulusProperty) property_id) {
        case PROP_START_TIME:
            g_value_set_object(value, priv->start_time);
            break;
        case PROP_STOP_TIME:
            g_value_set_object(value, priv->stop_time);
            break;
        case PROP_DURATION:
            {
                PsyDuration *dur = psy_time_point_subtract (
                        priv->stop_time,
                        priv->start_time
                        );
                g_value_set_object(value, dur);
                g_object_unref(dur);
            }
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
            break;
    }
}

static void
psy_stimulus_init(PsyStimulus* self)
{
    (void) self;
}

static void
psy_stimulus_class_init(PsyStimulusClass* klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    object_class->set_property = psy_stimulus_set_property;
    object_class->get_property = psy_stimulus_get_property;

    /**
     * Stimulus:start-time:
     *
     * This specifies when the stimulus is going to be presented. In case
     * of most stimuli, the framework will try to make a small adjustment
     * to the start time. Eg When the frame rate of a window/monitor is
     * 60 Hz a fresh screen can be presented every 16.666.. ms. Hence,
     * the start time will be reset to the nearest vblanking interval.
     */
    stimulus_properties[PROP_START_TIME] = g_param_spec_object (
            "start-time",
            "StartTime",
            "The time at which the stimulus started.",
            PSY_TYPE_TIME_POINT,
            G_PARAM_READABLE
            );

    /**
     * Stimulus:stop-time:
     *
     * This specifies stimulus will stop or has stopped. Similar to the start
     * time, the stop time might be adjusted by the framework in order to
     * meet match the physical properties of the stimulus device e.g. sound
     * card or monitor.
     */
    stimulus_properties[PROP_STOP_TIME] = g_param_spec_object (
            "stop-time",
            "StopTime",
            "The time at which the stimulus stopped.",
            PSY_TYPE_TIME_POINT,
            G_PARAM_READWRITE
            );

    /**
     * Stimulus:duration:
     *
     * This is the duration between the #PsyStimulus:stop-time and
     * #PsyStimulus:start-time. You can use this property to specify the
     * #PsyStimulus:stop-time, when you set it. In contrast when you read it,
     * it will be calculated as #PsyStimulus:stop-time - #PsyStimulus:start-time.
     */
    stimulus_properties[PROP_DURATION] = g_param_spec_object(
            "duration",
            "Duration",
            "The desired duration of the stimulus",
            PSY_TYPE_DURATION,
            G_PARAM_READWRITE
            );

    g_object_class_install_properties(
            object_class, NUM_PROPERTIES, stimulus_properties
            );

    /**
     * Stimulus::started:
     *
     * stimulus: The stimulus that received the signal.
     * start:(transfer None):The start time of the stimulus
     */
    stimulus_signals [SIG_STARTED] = g_signal_new (
            "started",
            PSY_TYPE_STIMULUS,
            G_SIGNAL_RUN_FIRST,
            G_STRUCT_OFFSET(struct _PsyStimulusClass, started),
            NULL,
            NULL,
            NULL,
            G_TYPE_NONE,
            1,
            PSY_TYPE_TIME_POINT
    );

    /**
     * PsyStimulus::stopped:
     * @stimulus: the `PsyStimulus` that received the signal.
     * @stop_time: (transfer none): A #PsyTimePoint at which the stimulus stopped.
     *
     * The stopped signal is emitted when the stimulus has just stopped. The
     * stop time is the deduced time when the stimulus has actually stopped,
     * this might be a little before or after the stop signal is actually
     * emitted.
     */
    stimulus_signals [SIG_STOPPED] = g_signal_new (
            "stopped",
            PSY_TYPE_STIMULUS,
            G_SIGNAL_RUN_LAST,
            G_STRUCT_OFFSET(PsyStimulusClass , stopped),
            NULL,
            NULL,
            NULL,
            G_TYPE_NONE,
            1,
            PSY_TYPE_TIME_POINT
    );
}

/**
 * psy_stimulus_play:
 * @self: A #PsyStimulus instance.
 * @start_time:(transfer none): A #PsyTimePoint at which the stimulus
 *                              should start.
 *
 * This method schedules a stimulus at a given start time. It is well possible
 * that the stimulus can not be started precisely at that time. You can
 * examine the #PsyStimulus:start_time property to see the when the stimulus
 * is going to start.
 */
void
psy_stimulus_play(PsyStimulus* self, PsyTimePoint* start_time)
{
    g_return_if_fail(PSY_IS_STIMULUS(self));
    g_return_if_fail(PSY_IS_TIME_POINT(start_time));

    PsyStimulusClass *klass = PSY_STIMULUS_GET_CLASS(self);

    g_return_if_fail(klass->play);

    klass->play(self, start_time);
}

/**
 * psy_stimulus_play_for:
 *
 * This is short hand for psy_stimulus_play(self, start_time) and
 * psy_stimulus_stop(self, start_time + dur);
 *
 * self: A #PsyStimulus instance
 * start_time:(transfer none): The intended start time of the stimulus
 * dur:(transfer none):        The intended duration of the stimulus
 */
void
psy_stimulus_play_for(PsyStimulus  *self,
                      PsyTimePoint *start_time,
                      PsyDuration  *dur
                      )
{
    g_return_if_fail(PSY_IS_STIMULUS(self));
    g_return_if_fail(PSY_IS_TIME_POINT(start_time));
    g_return_if_fail(PSY_IS_DURATION(dur));

    PsyTimePoint* end_time = psy_time_point_add(start_time, dur);
    psy_stimulus_play(self, start_time);
    psy_stimulus_stop(self, end_time);

    g_object_unref(end_time);
}

/**
 * psy_stimulus_play_until:
 *
 * self: A #PsyStimulus instance.
 * start_time: A #PsyTimePoint instance specifying a time point in the future
 *            when the stimulus in desired to start.
 * stop_time:  A #PsyTimePoint instance specifying a time point in the future
 *             when the stimulus in desired to stop.
 *
 * this is short hand for doing:
 * psy_stimulus_play(self, start_time);
 * psy_stimulus_stop(self, stop_time);
 */
void
psy_stimulus_play_until(PsyStimulus     *self,
                        PsyTimePoint    *start_time,
                        PsyTimePoint    *stop_time)
{
    g_return_if_fail(PSY_IS_STIMULUS(self));
    g_return_if_fail(PSY_IS_TIME_POINT(start_time));
    g_return_if_fail(PSY_IS_TIME_POINT(stop_time));

    psy_stimulus_play(self, start_time);
    psy_stimulus_stop(self, stop_time);
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
    g_return_if_fail(PSY_IS_TIME_POINT(stop_time));

    PsyStimulusClass *klass = PSY_STIMULUS_GET_CLASS(self);

    g_return_if_fail(klass->play);

    klass->stop(self, stop_time);
}

/**
 * psy_stimulus_get_start_time:
 *
 * Get the best estimation of when a stimulus is about to start or has started.
 *
 * self: A #PsyStimulus instance
 *
 * Returns:  An #PsyTimePoint describing when the stimulus will start or
 *           was started if it is in the past. Or NULL when the start time
 *           is still not decided on.
 */
PsyTimePoint*
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
 * Returns:: An #PsyTimePoint describing when the stimulus will stop or
 *           was stopped if it is in the past. Or NULL when the stop time
 *           is still not decided on.
 */
PsyTimePoint*
psy_stimulus_get_stop_time(PsyStimulus *self)
{
    g_return_val_if_fail(PSY_IS_STIMULUS(self), NULL);

    PsyStimulusPrivate *priv = psy_stimulus_get_instance_private(self);
    return priv->stop_time;
}

/**
 * psy_stimulus_set_duration:
 * @self: A #PsyStimulus instance.
 * @duration: A #PsyDuration instance that tells the framework for how long
 *        this stimulus should be presented.
 *
 * Sets the duration for the stimulus, in order to succeed the starting
 * time should be known.
 */
void
psy_stimulus_set_duration(PsyStimulus* self, PsyDuration* duration)
{
    g_return_if_fail(PSY_IS_STIMULUS(self));

    PsyStimulusPrivate* priv = psy_stimulus_get_instance_private(self);
    g_return_if_fail(priv->start_time != NULL);
    PsyTimePoint *end_time = psy_time_point_add(priv->start_time, duration);
    psy_stimulus_stop(self, end_time);
    g_object_unref(end_time);
}

/**
 * psy_stimulus_get_duration:
 * @self : a PsyStimulus instance.
 *
 * A duration of a stimulus is a "virtual" property, its the differnce
 * between the start and end time. Hence, both must be known, otherwise
 * NULL is returned.
 *
 * return: A #PsyDuration* that represents the duration of stop_time - start_time.
 *         or NULL, when no stop time is known.
 */
PsyDuration*
psy_stimulus_get_duration(PsyStimulus* self)
{
    g_return_val_if_fail(PSY_IS_STIMULUS(self), NULL);

    PsyStimulusPrivate* priv = psy_stimulus_get_instance_private(self);
    if (priv->start_time && priv->stop_time)
        return psy_time_point_subtract(priv->stop_time, priv->start_time);
    return NULL;
}
