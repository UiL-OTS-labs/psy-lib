
#include "psy-auditory-stimulus.h"
#include "psy-audio-device.h"
#include "psy-duration.h"

/**
 * PsyAuditoryStimulus:
 *
 * This is the base class for auditory stimuli. A PsyAuditoryStimulus is a
 * stimulus that is going to be presented at an instance of
 * [class@PsyAudioDevice]. The Base class sets up the frame work for when a
 * stimulus is presented and also where. Additionally it supports, some scaling
 * and rotation of a stimulus. This contains all the parameters to setup a Model
 * matrix for this stimulus. A auditory stimulus also supports a color, so that
 * artist know in what color stimuli should be presented.
 *
 * Instances of [class@AuditoryStimulus] are scheduled when the stimulus is
 * played. When, the stimulus is scheduled, the audio_device will call the
 * [method@Psy.AuditoryStimulus.create_artist], which should instantiate the
 * artist. The artist will be responsible for drawing the stimulus. The
 * PsyAuditoryStimulus is merely a dataholder.
 *
 * Derived instances may use the framework setup by [class@AuditoryStimulus]
 * and [class@Artist] to do drawing. The base class PsyAuditoryStimulus and
 * PsyArtist work together, so that deriving class can draw around the origin,
 * PsyAuditoryStimulus and PsyArtist make sure that the stimuli are positioned
 * on the right place.
 */

typedef struct PsyAuditoryStimulusPrivate {
    PsyAudioDevice *audio_device; // The audio_device on which this stimulus
                                  // should be presented
    gint64 nth_frame;
    gint64 num_frames;  // Total number of frames for stimulus duration
    gint64 start_frame; // When the stimulus should start, negative when not
                        // started.
    guint num_channels; // The number of channels, this is fixed, for some
                        // stimuli, such as wav files, the file determines
                        // the format. But it's flexible for others stimuli such
                        // as generated waveform such as Noise or Sine waves.
    PsyAudioChannelMap *channel_map; // The channel map to map source channels
                                     // to the output channels.
    GArray *sample_array;
} PsyAuditoryStimulusPrivate;

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE(PsyAuditoryStimulus,
                                    psy_auditory_stimulus,
                                    PSY_TYPE_STIMULUS)

typedef enum {
    PROP_NULL,         // not used required by GObject
    PROP_AUDIO_DEVICE, // The audio_device on which this stimulus should be
                       // drawn
    PROP_NUM_FRAMES,   // The number of frames the stimulus will be presented
    PROP_NTH_FRAME,    // the frame for which we are rendering
    PROP_START_FRAME,  // the frame at which this object should be first
                       // presented
    PROP_NUM_CHANNELS, // The number of channels of the audio, this is only
                       // writable when PROP_FLEXIBLE_NUM_CHANNELS is set
    PROP_FLEXIBLE_NUM_CHANNELS, // Whether or not NUM_CHANNELS can be set
                                // by the client.
    PROP_CHANNEL_MAP, // The map that maps the channels of the auditory
                      // stimulus to  the channels of the PsyAudioDevice/sink
    NUM_PROPERTIES
} AuditoryStimulusProperty;

typedef enum { SIG_ADD_CHANNEL_MAP, NUM_SIGNALS } AuditoryStimulusSignals;

static GParamSpec *auditory_stimulus_properties[NUM_PROPERTIES] = {0};
static guint       auditory_stimulus_signals[NUM_SIGNALS]       = {0};

static void
psy_auditory_stimulus_set_property(GObject      *object,
                                   guint         property_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
    PsyAuditoryStimulus *self = PSY_AUDITORY_STIMULUS(object);

    switch ((AuditoryStimulusProperty) property_id) {
    case PROP_AUDIO_DEVICE:
        psy_auditory_stimulus_set_audio_device(self, g_value_get_object(value));
        break;
    case PROP_START_FRAME:
        psy_auditory_stimulus_set_start_frame(self, g_value_get_int64(value));
        break;
    case PROP_NUM_CHANNELS:
        psy_auditory_stimulus_set_num_channels(self, g_value_get_uint(value));
        break;
    case PROP_CHANNEL_MAP:
        psy_auditory_stimulus_set_channel_map(self, g_value_get_boxed(value));
        break;
    case PROP_NUM_FRAMES:            // gettable only
    case PROP_NTH_FRAME:             // gettable only
    case PROP_FLEXIBLE_NUM_CHANNELS: // gettable only
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

static void
psy_auditory_stimulus_get_property(GObject    *object,
                                   guint       property_id,
                                   GValue     *value,
                                   GParamSpec *pspec)
{
    PsyAuditoryStimulus        *self = PSY_AUDITORY_STIMULUS(object);
    PsyAuditoryStimulusPrivate *priv
        = psy_auditory_stimulus_get_instance_private(self);

    switch ((AuditoryStimulusProperty) property_id) {
    case PROP_NUM_FRAMES:
        g_value_set_int64(value, priv->num_frames);
        break;
    case PROP_NTH_FRAME:
        g_value_set_int64(value, priv->nth_frame);
        break;
    case PROP_AUDIO_DEVICE:
        g_value_set_object(value, priv->audio_device);
        break;
    case PROP_START_FRAME:
        g_value_set_int64(value, priv->start_frame);
        break;
    case PROP_NUM_CHANNELS:
        g_value_set_uint(value, priv->num_channels);
        break;
    case PROP_FLEXIBLE_NUM_CHANNELS:
        g_value_set_boolean(
            value, psy_auditory_stimulus_get_flexible_num_channels(self));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

static void
psy_auditory_stimulus_dispose(GObject *object)
{
    PsyAuditoryStimulusPrivate *priv
        = psy_auditory_stimulus_get_instance_private(
            PSY_AUDITORY_STIMULUS(object));

    g_clear_object(&priv->audio_device);

    G_OBJECT_CLASS(psy_auditory_stimulus_parent_class)->dispose(object);
}

static void
psy_auditory_stimulus_init(PsyAuditoryStimulus *self)
{
    PsyAuditoryStimulusPrivate *priv
        = psy_auditory_stimulus_get_instance_private(self);
    priv->audio_device = NULL;
    priv->nth_frame    = 0;
    priv->num_frames   = -1;
    priv->start_frame  = -1;
}

static void
auditory_stimulus_play(PsyStimulus *stimulus, PsyTimePoint *start_time)
{
    // We first have to chainup in order to set the start time at the
    // stimulus.
    PSY_STIMULUS_CLASS(psy_auditory_stimulus_parent_class)
        ->play(stimulus, start_time);

    PsyAuditoryStimulus *stim = PSY_AUDITORY_STIMULUS(stimulus);

    PsyAudioDevice *audio_device = psy_auditory_stimulus_get_audio_device(stim);

    psy_audio_device_schedule_stimulus(audio_device, stim);
}

static void
auditory_stimulus_set_duration(PsyStimulus *self, PsyDuration *stim_dur)
{
    PsyAuditoryStimulusPrivate *priv
        = psy_auditory_stimulus_get_instance_private(
            PSY_AUDITORY_STIMULUS(self));

    PsyDuration *frame_dur = psy_audio_device_get_frame_dur(priv->audio_device);

    if (psy_duration_less(stim_dur, frame_dur)) {
        g_warning("Specified duration is less than one frame");
    }

    gint64       num_frames = psy_duration_divide_rounded(stim_dur, frame_dur);
    PsyDuration *corrected_dur
        = psy_duration_multiply_scalar(frame_dur, num_frames);
    priv->num_frames = num_frames;
    PSY_STIMULUS_CLASS(psy_auditory_stimulus_parent_class)
        ->set_duration(self, corrected_dur);
}

static void
psy_auditory_stimulus_class_init(PsyAuditoryStimulusClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    object_class->get_property = psy_auditory_stimulus_get_property;
    object_class->set_property = psy_auditory_stimulus_set_property;
    object_class->dispose      = psy_auditory_stimulus_dispose;

    PsyStimulusClass *stimulus_class = PSY_STIMULUS_CLASS(klass);
    stimulus_class->play             = auditory_stimulus_play;
    stimulus_class->set_duration     = auditory_stimulus_set_duration;

    /**
     * PsyAuditoryStimulus:audio_device:
     *
     * The stimulus on which this stimulus will be played/displays when it
     * is started as stimulus. It should not be NULL, so it should be specified
     * when constructing the stimulus.
     */
    auditory_stimulus_properties[PROP_AUDIO_DEVICE] = g_param_spec_object(
        "audio_device",
        "AudioDevice",
        "The audio_device on which this stimulus will be drawn",
        PSY_TYPE_AUDIO_DEVICE,
        G_PARAM_WRITABLE | G_PARAM_CONSTRUCT);

    /**
     * PsyAuditoryStimulus:num-frames:
     *
     * When a auditory stimulus is played for an amount of time, the stimulus
     * on which it will be drawn will be determine when it will be presented for
     * the first time.
     * This value will be most useful in the update signal handler or in
     * `psy_auditory_stimulus_update`.
     */
    auditory_stimulus_properties[PROP_NUM_FRAMES] = g_param_spec_int64(
        "num-frames",
        "NumFrames",
        "The number of frames that this stimulus will be displayed.",
        -1,
        G_MAXINT64,
        -1,
        G_PARAM_READABLE);

    /**
     * PsyAuditoryStimulus:nth-frame:
     *
     * Everytime the stimulus draws a stimulus, you'll get the time to update
     * it. This value will be most useful in the update signal handler or in
     * `psy_auditory_stimulus_update`.
     */
    auditory_stimulus_properties[PROP_NTH_FRAME]
        = g_param_spec_int64("nth-frame",
                             "NthFrame",
                             "The nth frame of this stimulus.",
                             -1,
                             G_MAXINT64,
                             -1,
                             G_PARAM_READABLE);

    /**
     * PsyAuditoryStimulus:start-frame:
     *
     * When auditory stimuli are scheduled, we have to specify a given frame
     * on which the stimulus will be presented for the first time.
     *
     */
    auditory_stimulus_properties[PROP_START_FRAME] = g_param_spec_int64(
        "start-frame",
        "StartFrame",
        "The number of the frame on which this stimulus should be presented",
        0,
        G_MAXINT64,
        0,
        G_PARAM_READABLE);

    /**
     * PsyAuditoryStimulus:num-channels:
     *
     * The number of channels for the audio stimuli. This parameter is always
     * readable, however, for some stimuli, it is writeable, whereas for
     * others it is ignored. E.g. psylib can decide to create a 1, 2, etc
     * channel sine wave, but it can't/won't assign new channels to existing
     * .wav or other files. So typically, it's true for generated audio,
     * but static for sounds that come from a source outside of psylib.
     * So if you want to know, if
     * [property@PsyAuditoryStimulus:flexible-num-channels] is true, than
     * you should be able to set it manually. Otherwise, the default is 2,
     * so you get a stereo output. You'll can't write to this property,
     * but you can set it using [method@AuditoryStimulus.set_num_channels].
     */
    auditory_stimulus_properties[PROP_NUM_CHANNELS]
        = g_param_spec_uint("num-channels",
                            "NumChannels",
                            "The number of channels for the stimulus",
                            0,
                            G_MAXINT,
                            0,
                            G_PARAM_READWRITE);

    /**
     * PsyAuditoryStimulus:flexible-num-channels:
     *
     * Whether or not the client may set the number of channels for this
     * stimulus.
     */
    auditory_stimulus_properties[PROP_FLEXIBLE_NUM_CHANNELS]
        = g_param_spec_boolean(
            "flexible-num-channels",
            "FlexibleNumChannels",
            "Whether or not the client is able to set the number of channels",
            FALSE,
            G_PARAM_READABLE);

    /**
     * PsyAuditoryStimulus:channel-map:
     *
     * The mapping between the output channels (source pads) of this stimulus
     * and the (input channels) of the PsyAudioDevice.
     */
    auditory_stimulus_properties[PROP_CHANNEL_MAP] = g_param_spec_boxed(
        "channel-map",
        "ChannelMap",
        "The map that maps the StimulusOuptputs(sources) to the input (sinks)"
        "of the PsyAudioDevice/mixer",
        PSY_TYPE_AUDIO_CHANNEL_MAP,
        G_PARAM_READWRITE);

    g_object_class_install_properties(
        object_class, NUM_PROPERTIES, auditory_stimulus_properties);

    /**
     * PsyAuditoryStimulus::add-channel-map:
     * @self: an instance of [class@PsyAuditoryStimulus]
     * @device: an instance of [class@PsyAudioDevice]
     *
     * This signal is emitted when the stimulus is scheduled, by then
     * all parameters should be fixed/known and may a channel map be
     * attached to transfer channels of this stimulus to the channels
     * of the PsyAudioDevice.
     */
    auditory_stimulus_signals[SIG_ADD_CHANNEL_MAP] = g_signal_new(
        "add-channel-map",
        PSY_TYPE_AUDITORY_STIMULUS,
        G_SIGNAL_RUN_LAST,
        G_STRUCT_OFFSET(PsyAuditoryStimulusClass, add_channel_map),
        NULL,
        NULL,
        NULL,
        G_TYPE_NONE,
        1,
        PSY_TYPE_AUDIO_DEVICE);
}

/**
 * psy_auditory_stimulus_get_audio_device:
 * @self: an instance of  [class@Psy.AuditoryStimulus]
 *
 * Get the audio_device on which this stimulus should be drawn.
 *
 * Returns: (nullable) (transfer none): The `PsyAudioDevice` on which this
 * stimulus should be drawn.
 */
PsyAudioDevice *
psy_auditory_stimulus_get_audio_device(PsyAuditoryStimulus *self)
{
    PsyAuditoryStimulusPrivate *priv
        = psy_auditory_stimulus_get_instance_private(self);

    g_return_val_if_fail(PSY_IS_AUDITORY_STIMULUS(self), NULL);

    return priv->audio_device;
}

/**
 * psy_auditory_stimulus_set_audio_device:
 * @stimulus: a `PsyAuditoryStimulus`
 * @audio_device:(transfer full): a `PsyAudioDevice` to draw this stimulus on.
 *
 * Set the audio_device on which this stimulus should be drawn.
 */
void
psy_auditory_stimulus_set_audio_device(PsyAuditoryStimulus *stimulus,
                                       PsyAudioDevice      *audio_device)
{
    PsyAuditoryStimulusPrivate *priv
        = psy_auditory_stimulus_get_instance_private(stimulus);

    g_return_if_fail(PSY_IS_AUDITORY_STIMULUS(stimulus));
    g_return_if_fail(PSY_IS_AUDIO_DEVICE(audio_device));

    g_clear_object(&priv->audio_device);
    priv->audio_device = g_object_ref(audio_device);
}

/**
 * psy_auditory_stimulus_get_num_samples:
 * @self: an instance of `PsyAuditoryStimulus`.
 *
 * This function returns how many samples this stimulus contains/-ed.
 * This may be used to calculate the duration of this stimulus.
 *
 * Returns: An integer reflecting how many frames this stimulus has been
 * presented.
 */
gint64
psy_auditory_stimulus_get_num_frames(PsyAuditoryStimulus *self)
{
    PsyAuditoryStimulusPrivate *priv
        = psy_auditory_stimulus_get_instance_private(self);
    g_return_val_if_fail(PSY_IS_AUDITORY_STIMULUS(self), -1);

    return priv->num_frames;
}

/**
 * psy_auditory_stimulus_get_nth_frame:
 * @self: an instance of `PsyAuditoryStimulus`.
 *
 * This function returns how many times the stimulus has be presented. Notice
 * that this starts at 0 when preparing the first frame.
 *
 * Returns: An integer reflecting how many frames this stimulus has been
 * presented.
 */
gint64
psy_auditory_stimulus_get_nth_frame(PsyAuditoryStimulus *self)
{
    PsyAuditoryStimulusPrivate *priv
        = psy_auditory_stimulus_get_instance_private(self);
    g_return_val_if_fail(PSY_IS_AUDITORY_STIMULUS(self), -1);

    return priv->nth_frame;
}

gboolean
psy_auditory_stimulus_is_scheduled(PsyAuditoryStimulus *self)
{
    PsyAuditoryStimulusPrivate *priv
        = psy_auditory_stimulus_get_instance_private(self);
    g_return_val_if_fail(PSY_IS_AUDITORY_STIMULUS(self), FALSE);

    return priv->start_frame >= 0;
}

/**
 * psy_auditory_stimulus_set_start_frame:
 * @self: an instance of `PsyAuditoryStimulus`
 * @frame_num: the number of the frame on which this stimulus should start
 *
 * Sets the frame number of the frame of a monitor on which this stimulus should
 * start.
 *
 * stability:private
 */
void
psy_auditory_stimulus_set_start_frame(PsyAuditoryStimulus *self,
                                      gint64               frame_num)
{
    PsyAuditoryStimulusPrivate *priv
        = psy_auditory_stimulus_get_instance_private(self);
    g_return_if_fail(PSY_IS_AUDITORY_STIMULUS(self));

    priv->start_frame = frame_num;
}

/**
 * psy_auditory_stimulus_get_start_frame:
 * @self: an instance of `PsyAuditoryStimulus`
 *
 * Gets the frame number of the frame of a monitor on which this stimulus should
 * start.
 *
 * Returns: The number of the frame of the `PsyWindow` that @self should be
 *          first presented.
 */
gint64
psy_auditory_stimulus_get_start_frame(PsyAuditoryStimulus *self)
{
    PsyAuditoryStimulusPrivate *priv
        = psy_auditory_stimulus_get_instance_private(self);
    g_return_val_if_fail(PSY_IS_AUDITORY_STIMULUS(self), -1);

    return priv->start_frame;
}

/**
 * psy_auditory_stimulus_get_flexible_num_channels:
 * @self: an instance [class@AuditoryStimulus]
 *
 * Get whether or not the client may set the number of channels for this
 * stimulus. The deriving classes should dictate whether this is possible.
 */
gboolean
psy_auditory_stimulus_get_flexible_num_channels(PsyAuditoryStimulus *self)
{
    PsyAuditoryStimulusClass *cls = PSY_AUDITORY_STIMULUS_GET_CLASS(self);

    g_return_val_if_fail(PSY_IS_AUDITORY_STIMULUS(self), FALSE);
    g_return_val_if_fail(cls->get_flexible_num_channels != NULL, FALSE);

    return cls->get_flexible_num_channels();
}

/**
 * psy_auditory_stimulus_get_channel_map:
 *
 * Get a copy from the internal channelmap
 *
 * Returns:(transfer full): A copy from the internal channel map or NULL when
 *    no channel map has been set.
 */
PsyAudioChannelMap *
psy_auditory_stimulus_get_channel_map(PsyAuditoryStimulus *self)
{
    PsyAuditoryStimulusPrivate *priv
        = psy_auditory_stimulus_get_instance_private(self);
    g_return_val_if_fail(PSY_IS_AUDITORY_STIMULUS(self), NULL);

    if (priv->channel_map == NULL)
        return NULL;

    return psy_audio_channel_map_dup(priv->channel_map);
}

/**
 * psy_auditory_stimulus_set_channel_map:
 * @self: an instance of PsyAuditoryStimulus
 * @map:(transfer none): A mapping between the output channels (source channels)
 *     of the stimulus and the sink channels of the Opened PsyAudioDevice/Mixer.
 *
 * Set the channel map, The channel map should be appropriate for this device.
 */
void
psy_auditory_stimulus_set_channel_map(PsyAuditoryStimulus *self,
                                      PsyAudioChannelMap  *map)
{
    PsyAuditoryStimulusPrivate *priv
        = psy_auditory_stimulus_get_instance_private(self);
    g_return_if_fail(PSY_IS_AUDITORY_STIMULUS(self));
    g_return_if_fail(map != NULL);

    g_clear_pointer(&priv->channel_map, psy_audio_channel_map_free);
    priv->channel_map = psy_audio_channel_map_dup(map);
}

/**
 * psy_auditory_stimulus_set_num_channels:
 * @self:
 * @num_channels: a value between 1 and G_MAXINT
 *
 * Set the number of channels this is handy for generated waveforms/noise
 * For file like stimuli, the file determines the number of channels
 */
void
psy_auditory_stimulus_set_num_channels(PsyAuditoryStimulus *self,
                                       guint                num_channels)
{
    PsyAuditoryStimulusPrivate *priv
        = psy_auditory_stimulus_get_instance_private(self);

    g_return_if_fail(PSY_IS_AUDITORY_STIMULUS(self));
    g_return_if_fail(num_channels <= G_MAXINT);

    if (!psy_auditory_stimulus_get_flexible_num_channels(self)) {
        g_warning("Unable to change num channels for instance of %s",
                  G_OBJECT_CLASS_NAME(PSY_AUDITORY_STIMULUS_GET_CLASS(self)));
    }

    if (psy_auditory_stimulus_is_scheduled(self)) {
        g_warning("Unable to change num channels when stimulus is active");
        return;
    }

    priv->num_channels = num_channels;
}

/**
 * psy_auditory_stimulus_get_num_channels:
 *
 * Returns the number of channels this device uses. For some devices
 * this might return 0 at first, however, after the pipeline is set to playing
 * the number of channels is known and put in this field.
 *
 * Returns: A positive number greater than 0. or 0 when it isn't yet available
 *     e.g. for files first a bit of the file needs to be read.
 */
guint
psy_auditory_stimulus_get_num_channels(PsyAuditoryStimulus *self)
{
    PsyAuditoryStimulusPrivate *priv
        = psy_auditory_stimulus_get_instance_private(self);

    g_return_val_if_fail(PSY_IS_AUDITORY_STIMULUS(self), 0);

    return priv->num_channels;
}

/**
 * psy_auditory_stimulus_read:
 * @self: an instance of PsyAuditoryStimulus
 * @num_samples: The number of samples that should be read for each channel
 *      of audio.
 * @result:(out caller-allocates): A return location to store the audio,
 *      the output should be large enough to house num_channels * num_samples
 *      samples
 *
 * This function is called by the PsyAudioOutputMixer in order to obtain the
 * data for mixing the final output buffer. It's the responsibility of the
 * mixer to mix the final output for the audiocallback.
 *
 * Returns: The number of samples (for each channel) read this should generally
 * be equal to the num_samples argument, expect when it's exhausted. This is
 * an indication that the stimulus is at the end of its stream.
 */
guint
psy_auditory_stimulus_read(PsyAuditoryStimulus *self,
                           guint                num_samples,
                           gfloat              *result)
{
    g_return_val_if_fail(PSY_IS_AUDITORY_STIMULUS(self), 0);
    g_return_val_if_fail(result != NULL, 0);

    PsyAuditoryStimulusClass *cls = PSY_AUDITORY_STIMULUS_GET_CLASS(self);

    g_return_val_if_fail(cls->read != NULL, 0);

    return cls->read(self, num_samples, result);
}
