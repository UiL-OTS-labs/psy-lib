
#include "ddd-stimulus.h"

typedef struct DddStimulusPrivate {
    DddTimePoint *start_time;
    DddTimePoint *stop_time;
} DddStimulusPrivate;

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE(DddStimulus, ddd_stimulus, G_TYPE_OBJECT)

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
ddd_stimulus_set_property (GObject      *object,
                           guint         property_id,
                           const GValue *value,
                           GParamSpec   *pspec)
{
    DddStimulus *self = DDD_STIMULUS(object);
    DddStimulusPrivate *priv = ddd_stimulus_get_instance_private(self);

    switch ((StimulusProperty) property_id) {
        case PROP_START_TIME:
            ddd_stimulus_play(self, g_value_get_object(value));
            break;
        case PROP_STOP_TIME:
            ddd_stimulus_play_until(self,
                                    priv->start_time,
                                    g_value_get_object(value));
            break;
        case PROP_DURATION:
            ddd_stimulus_play_for(self,
                                  priv->start_time,
                                  g_value_get_object(value));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
            break;
    }
}

static void
ddd_stimulus_get_property (GObject      *object,
                           guint         property_id,
                           GValue       *value,
                           GParamSpec   *pspec)
{
    DddStimulus *self = DDD_STIMULUS(object);
    DddStimulusPrivate * priv = ddd_stimulus_get_instance_private(self);
    switch ((StimulusProperty) property_id) {
        case PROP_START_TIME:
            g_value_set_object(value, priv->start_time);
            break;
        case PROP_STOP_TIME:
            g_value_set_object(value, priv->stop_time);
            break;
        case PROP_DURATION:
            {
                DddDuration *dur = ddd_time_point_subtract (
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
ddd_stimulus_init(DddStimulus* self)
{
    (void) self;
}

static void
ddd_stimulus_class_init(DddStimulusClass* klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    object_class->set_property = ddd_stimulus_set_property;
    object_class->get_property = ddd_stimulus_get_property;

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
            DDD_TYPE_TIME_POINT,
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
            DDD_TYPE_TIME_POINT,
            G_PARAM_READWRITE
            );

    /**
     * Stimulus:duration:
     *
     * This is the duration between the #DddStimulus:stop-time and
     * #DddStimulus:start-time. You can use this property to specify the
     * #DddStimulus:stop-time, when you set it. In contrast when you read it,
     * it will be calculated as #DddStimulus:stop-time - #DddStimulus:start-time.
     */
    stimulus_properties[PROP_DURATION] = g_param_spec_object(
            "duration",
            "Duration",
            "The desired duration of the stimulus",
            DDD_TYPE_DURATION,
            G_PARAM_READWRITE
            );

    g_object_class_install_properties(
            object_class, NUM_PROPERTIES, stimulus_properties
            );

    /**
     * Stimulus::started:
     * @stimulus: The stimulus that received the signal.
     * @start:(transfer None):The start time of the stimulus
     */
    stimulus_signals [SIG_STARTED] = g_signal_new (
            "started",
            DDD_TYPE_STIMULUS,
            G_SIGNAL_RUN_FIRST,
            G_STRUCT_OFFSET(struct _DddStimulusClass, started),
            NULL,
            NULL,
            NULL,
            G_TYPE_NONE,
            1,
            DDD_TYPE_TIME_POINT
    );

    /**
     * DddStimulus::stopped:
     * @stimulus: the #DddStimulus that received the signal.
     * @stop_time:(transfer none): A #DddTimePoint at which the stimulus stopped.
     */
    stimulus_signals [SIG_STOPPED] = g_signal_new (
            "stopped",
            DDD_TYPE_STIMULUS,
            G_SIGNAL_RUN_LAST,
            G_STRUCT_OFFSET(DddStimulusClass , stopped),
            NULL,
            NULL,
            NULL,
            G_TYPE_NONE,
            1,
            G_TYPE_UINT64
    );
}

/**
 * ddd_stimulus_play:
 * @self: A #DddStimulus instance.
 * @desired_start_time:(transfer none): A #DddTimePoint at which the stimulus
 *                     should start.
 *
 * This method schedules a stimulus at a given start time. It is well possible
 * that the stimulus can not be started precisly at that time. You can
 * examine the #DddStimulus:start_time property to see the when the stimulus
 * is going to start.
 */
void
ddd_stimulus_play(DddStimulus* self, DddTimePoint* desired_start_time)
{
    g_return_if_fail(DDD_IS_STIMULUS(self));
    g_return_if_fail(DDD_IS_TIME_POINT(desired_start_time));

    DddStimulusClass *klass = DDD_STIMULUS_GET_CLASS(self);

    g_return_if_fail(klass->play);

    klass->play(self, desired_start_time);
}

/**
 * ddd_stimulus_play_for:
 * @self: A #DddStimulus instance
 * @param:(transfer none):start_time The intended start time of the stimulus
 * @param:(transfer none):dur        The intended duration of the stimulus
 *
 * This is short hand for ddd_stimulus_play(self, start_time) and
 * ddd_stimulus_stop(self, start_time + dur);
 */
void
ddd_stimulus_play_for(DddStimulus  *self,
                      DddTimePoint *start_time,
                      DddDuration  *dur
                      )
{
    g_return_if_fail(DDD_IS_STIMULUS(self));
    g_return_if_fail(DDD_IS_TIME_POINT(start_time));
    g_return_if_fail(DDD_IS_DURATION(dur));

    DddTimePoint* end_time = ddd_time_point_add(start_time, dur);
    ddd_stimulus_play(self, start_time);
    ddd_stimulus_stop(self, end_time);

    g_object_unref(end_time);
}

/**
 * ddd_stimulus_play:
 * @self: A #DddStimulus instance.
 * @start_time A #DddTimePoint instance specifying a time point in the future
 *             when the stimulus in desired to start.
 * @stop_time  A #DddTimePoint instance specifying a time point in the future
 *             when the stimulus in desired to stop.
 *
 * this is short hand for doing:
 * ddd_stimulus_play(self, start_time);
 * ddd_stimulus_stop(self, stop_time);
 */
void
ddd_stimulus_play_until(DddStimulus     *self,
                        DddTimePoint    *start_time,
                        DddTimePoint    *stop_time)
{
    g_return_if_fail(DDD_IS_STIMULUS(self));
    g_return_if_fail(DDD_IS_TIME_POINT(start_time));
    g_return_if_fail(DDD_IS_TIME_POINT(stop_time));

    ddd_stimulus_play(self, start_time);
    ddd_stimulus_stop(self, stop_time);
}

/**
 * ddd_stimulus_stop:
 * @self:
 * @desired_stop_time:
 *
 * Set the stop time of the stimulus.
 */
void
ddd_stimulus_stop(DddStimulus *self, DddTimePoint *desired_stop_time)
{
    g_return_if_fail(DDD_IS_STIMULUS(self));
    g_return_if_fail(DDD_IS_TIME_POINT(desired_stop_time));

    DddStimulusClass *klass = DDD_STIMULUS_GET_CLASS(self);

    g_return_if_fail(klass->play);

    klass->stop(self, desired_stop_time);
}

/**
 * ddd_stimulus_get_start_time:
 * @self: A #DddStimulus instance
 *
 * Get the best estimation of when a stimulus is about to start or has started.
 *
 * @return:: An #DddTimePoint describing when the stimulus will start or
 *           was started if it is in the past. Or NULL when the start time
 *           is still not decided on.
 */
DddTimePoint*
ddd_stimulus_get_start_time(DddStimulus *self)
{
    g_return_val_if_fail(DDD_IS_STIMULUS(self), NULL);

    DddStimulusPrivate *priv = ddd_stimulus_get_instance_private(self);
    return priv->start_time;
}

/**
 * ddd_stimulus_get_stop_time:
 * @self: A #DddStimulus instance
 *
 * Get the best estimation of when a stimulus is about to stop or has stopped.
 *
 * @return:: An #DddTimePoint describing when the stimulus will stop or
 *           was stopped if it is in the past. Or NULL when the stop time
 *           is still not decided on.
 */
DddTimePoint*
ddd_stimulus_get_stop_time(DddStimulus *self)
{
    g_return_val_if_fail(DDD_IS_STIMULUS(self), NULL);

    DddStimulusPrivate *priv = ddd_stimulus_get_instance_private(self);
    return priv->stop_time;
}

/**
 * ddd_stimulus_set_duration:
 * @self: A #DddStimulus instance.
 * @dur: A #DddDuration instance that tells the framework for how long
 *        this stimulus should be presented.
 *
 * Sets the duration for the stimulus, in order to succeed the starting
 * time should be known.
 */
void
ddd_stimulus_set_duration(DddStimulus* self, DddDuration* dur)
{
    g_return_if_fail(DDD_IS_STIMULUS(self));

    DddStimulusPrivate* priv = ddd_stimulus_get_instance_private(self);
    g_return_if_fail(priv->start_time != NULL);
    DddTimePoint *end_time = ddd_time_point_add(priv->start_time, dur);
    ddd_stimulus_stop(self, end_time);
    g_object_unref(end_time);
}

/**
 * ddd_stimulus_get_duration:
 * @self : a DddStimulus instance.
 *
 * A duration of a stimulus is a "virtual" property, its the differnce
 * between the start and end time. Hence, both must be known, otherwise
 * NULL is returned.
 *
 * return: A #DddDuration* that represents the duration of stop_time - start_time.
 *         or NULL, when no stop time is known.
 */
DddDuration*
ddd_stimulus_get_duration(DddStimulus* self)
{
    g_return_val_if_fail(DDD_IS_STIMULUS(self), NULL);

    DddStimulusPrivate* priv = ddd_stimulus_get_instance_private(self);
    if (priv->start_time && priv->stop_time)
        return ddd_time_point_subtract(priv->stop_time, priv->start_time);
    return NULL;
}