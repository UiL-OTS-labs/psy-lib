

#include "psy-config.h"

#include "enum-types.h"

#include "psy-audio-device.h"
#include "psy-audio-mixer.h"
#include "psy-duration.h"
#include "psy-enums.h"
#include "psy-queue.h"

/**
 * PsyAudioMixer:
 *
 * The AudioMixer is considered to be a private to Psylib in practice the
 * AudioDevice will create one instance of this class for the in and output.
 *
 * A PsyAudioMixer is a mixer that mixes the scheduled PsyAuditoryStimuli
 * together. It maintains an buffer with audio samples, Psylib must make sure
 * that this audio buffer is ready to produce samples for the audio callback.
 * the settings of the PsyAudioMixer should always match the settings
 * for the PsyAudioDevice.
 *
 * This class is used by PsyAudioDevices to buffer audio in such a way
 * that the audio callback can retrieve samples from this buffer very quickly.
 *
 * The class has a pure virtual function process_audio that should be
 * implemented by deriving classes. The function is installed as a
 * callback function that is called every roughly every ms. In the case of
 * an output mixer, the callback should ensure there is enough data to be read
 * by the audio callback. In case of an input, there should be enough space
 * in the input buffer so that the audio callback can write all its samples.
 *
 * Stability: private
 */

#define DEFAULT_NUM_STIM_CACHE 16

typedef struct _PsyAudioMixerPrivate {

    PsyAudioDevice *device;

    PsyAudioQueue *in_queue;  // AudioCallback writes to input queue.
    PsyAudioQueue *out_queue; // AudioCallback reads from output queue.

    GMainContext *context;
    GPtrArray    *stimuli;
    guint         process_callback_id;

    gboolean is_output;

} PsyAudioMixerPrivate;

G_DEFINE_TYPE_WITH_PRIVATE(PsyAudioMixer, psy_audio_mixer, G_TYPE_OBJECT)

typedef enum {
    PROP_NULL,
    PROP_AUDIO_DEVICE,
    PROP_SAMPLE_RATE,
    PROP_NUM_IN_CHANNELS,
    PROP_NUM_OUT_CHANNELS,
    PROP_IS_OUTPUT,
    PROP_IS_INPUT,
    NUM_PROPERTIES
} PsyAudioMixerProperty;

// typedef enum { STARTED, NUM_SIGNALS } PsyAudioMixerSignals;

static GParamSpec *audio_mixer_properties[NUM_PROPERTIES];

// static guint       audio_mixer_signals[NUM_SIGNALS];

/* ********** virtual(/private) functions ***************** */

static void
psy_audio_mixer_set_property(GObject      *object,
                             guint         prop_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
    PsyAudioMixer        *self = PSY_AUDIO_MIXER(object);
    PsyAudioMixerPrivate *priv = psy_audio_mixer_get_instance_private(self);

    switch ((PsyAudioMixerProperty) prop_id) {
    case PROP_AUDIO_DEVICE:
        psy_audio_mixer_set_audio_device(self, g_value_get_object(value));
        break;
    case PROP_IS_OUTPUT:
        priv->is_output = g_value_get_boolean(value);
        break;
    case PROP_IS_INPUT:
        priv->is_output = !g_value_get_boolean(value);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

static void
psy_audio_mixer_get_property(GObject    *object,
                             guint       prop_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
    PsyAudioMixer        *self = PSY_AUDIO_MIXER(object);
    PsyAudioMixerPrivate *priv = psy_audio_mixer_get_instance_private(self);

    switch ((PsyAudioMixerProperty) prop_id) {
    case PROP_SAMPLE_RATE:
        g_value_set_enum(value, psy_audio_mixer_get_sample_rate(self));
        break;
    case PROP_NUM_IN_CHANNELS:
        g_value_set_uint(value, psy_audio_mixer_get_num_in_channels(self));
        break;
    case PROP_NUM_OUT_CHANNELS:
        g_value_set_uint(value, psy_audio_mixer_get_num_out_channels(self));
        break;
    case PROP_AUDIO_DEVICE:
        g_value_set_object(value, psy_audio_mixer_get_audio_device(self));
        break;
    case PROP_IS_OUTPUT:
        g_value_set_boolean(value, priv->is_output);
        break;
    case PROP_IS_INPUT:
        g_value_set_boolean(value, !priv->is_output);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

static int
audio_mixer_call_process(gpointer data)
{
    g_assert(PSY_IS_AUDIO_MIXER(data));
    PsyAudioMixer *self = data;

    psy_audio_mixer_process_audio(self);

    return G_SOURCE_CONTINUE;
}

static void
psy_audio_mixer_init(PsyAudioMixer *self)
{
    PsyAudioMixerPrivate *priv = psy_audio_mixer_get_instance_private(self);
    priv->device               = NULL;
    priv->stimuli
        = g_ptr_array_new_full(DEFAULT_NUM_STIM_CACHE, g_object_unref);

    priv->process_callback_id = g_timeout_add_full(
        G_PRIORITY_HIGH, 1, audio_mixer_call_process, self, NULL);
}

static void
audio_mixer_constructed(GObject *self)
{
    G_OBJECT_CLASS(psy_audio_mixer_parent_class)->constructed(self);

    PsyAudioMixerPrivate *priv
        = psy_audio_mixer_get_instance_private(PSY_AUDIO_MIXER(self));

    guint n_in_chan
        = psy_audio_mixer_get_num_in_channels(PSY_AUDIO_MIXER(self));
    guint n_out_chan
        = psy_audio_mixer_get_num_out_channels(PSY_AUDIO_MIXER(self));

    guint  num_samples = psy_audio_device_get_num_samples_buffer(priv->device);
    gint64 num_in_samples  = (gint64) num_samples * n_in_chan;
    gint64 num_out_samples = (gint64) num_samples * n_out_chan;

    g_return_if_fail(num_in_samples < G_MAXUINT && num_out_samples < G_MAXUINT);

    priv->in_queue  = psy_audio_queue_new(num_in_samples);
    priv->out_queue = psy_audio_queue_new(num_out_samples);

    // fill output queue, otherwise audio callback can't fetch data and will log
    // errors.
    if (priv->is_output) {
        psy_audio_mixer_process_audio(PSY_AUDIO_MIXER(self));
    }
}

static void
psy_audio_mixer_dispose(GObject *object)
{
    PsyAudioMixer *self = PSY_AUDIO_MIXER(object);

    PsyAudioMixerPrivate *priv = psy_audio_mixer_get_instance_private(self);

    g_clear_object(&priv->device);

    if (priv->stimuli) {
        g_ptr_array_unref(priv->stimuli);
        priv->stimuli = NULL;
    }

    if (priv->process_callback_id != 0) {
        g_source_remove(priv->process_callback_id);
        priv->process_callback_id = 0;
    }

    G_OBJECT_CLASS(psy_audio_mixer_parent_class)->dispose(object);
}

static void
psy_audio_mixer_finalize(GObject *object)
{
    PsyAudioMixer        *self = PSY_AUDIO_MIXER(object);
    PsyAudioMixerPrivate *priv = psy_audio_mixer_get_instance_private(self);

    psy_audio_queue_free(priv->in_queue);
    psy_audio_queue_free(priv->out_queue);
    priv->in_queue  = NULL;
    priv->out_queue = NULL;

    G_OBJECT_CLASS(psy_audio_mixer_parent_class)->finalize(object);
}

static void
audio_mixer_process_audio(PsyAudioMixer *self)
{
    PsyAudioMixerPrivate *priv = psy_audio_mixer_get_instance_private(self);

    gfloat sample = 0.0f;

    while (psy_audio_queue_push_samples(priv->out_queue, 1, &sample) == 1)
        ;
}

static void
psy_audio_mixer_class_init(PsyAudioMixerClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

    gobject_class->set_property = psy_audio_mixer_set_property;
    gobject_class->get_property = psy_audio_mixer_get_property;
    gobject_class->finalize     = psy_audio_mixer_finalize;
    gobject_class->dispose      = psy_audio_mixer_dispose;
    gobject_class->constructed  = audio_mixer_constructed;

    klass->process_audio = audio_mixer_process_audio;

    audio_mixer_properties[PROP_AUDIO_DEVICE]
        = g_param_spec_object("audio-device",
                              "AudioDevice",
                              "The Audio device \"attached\" to this mixer",
                              PSY_TYPE_AUDIO_DEVICE,
                              G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

    audio_mixer_properties[PROP_SAMPLE_RATE]
        = g_param_spec_enum("sample-rate",
                            "SampleRate",
                            "The configured sample rate",
                            PSY_TYPE_AUDIO_SAMPLE_RATE,
                            PSY_AUDIO_SAMPLE_RATE_48000,
                            G_PARAM_READABLE);

    audio_mixer_properties[PROP_NUM_IN_CHANNELS]
        = g_param_spec_uint("num-in-channels",
                            "NumInChannels",
                            "The number of input channels for the mixer.",
                            0,
                            G_MAXUINT,
                            1,
                            G_PARAM_READABLE);

    audio_mixer_properties[PROP_NUM_OUT_CHANNELS]
        = g_param_spec_uint("num-out-channels",
                            "NumOutChannels",
                            "The number of output channels for the mixer.",
                            0,
                            G_MAXUINT,
                            1,
                            G_PARAM_READABLE);

    audio_mixer_properties[PROP_IS_OUTPUT] = g_param_spec_boolean(
        "is-output",
        "IsOutput",
        "Whether or not this mixer is configured for output",
        TRUE,
        G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);

    audio_mixer_properties[PROP_IS_INPUT] = g_param_spec_boolean(
        "is-input",
        "IsInput",
        "Whether or not this mixer is configured as input",
        FALSE,
        G_PARAM_READABLE);

    g_object_class_install_properties(
        gobject_class, NUM_PROPERTIES, audio_mixer_properties);
}

/* ************ public functions ******************** */

/**
 * psy_audio_mixer_new:(skip)(constructor)
 * @device: An instance of PsyAudioDevice, that is connected to this mixer.
 * @is_output: Whether or not this mixer is configured as output.
 *
 * Configures a new in- or output audio mixer. If true is passed to the
 * is output, one may only read from the audio mixer. Otherwise, one can
 * only write to the output mixer.
 *
 * Construct a new instance of [class@PsyAudioMixer]
 */
PsyAudioMixer *
psy_audio_mixer_new(PsyAudioDevice *device, gboolean is_output)
{
    // clang-format off
    return g_object_new(PSY_TYPE_AUDIO_MIXER,
                        "audio-device", device,
                        "is-output", is_output,
                        NULL);
    // clang-format on
}

void
psy_audio_mixer_schedule_stimulus(PsyAudioMixer       *self,
                                  PsyAuditoryStimulus *stimulus)
{
    PsyAudioMixerPrivate *priv = psy_audio_mixer_get_instance_private(self);

    g_return_if_fail(PSY_IS_AUDIO_MIXER(self)
                     && PSY_IS_AUDITORY_STIMULUS(stimulus));

    if (psy_auditory_stimulus_is_scheduled(stimulus))
        return;

    PsyTimePoint *tp_sample = NULL;
    gint64        nth_sample;

    if (!psy_audio_device_get_last_known_frame(
            priv->device, &nth_sample, NULL, &tp_sample)) {
        g_critical("%s: The audio device doesn't seem to be running.",
                   __func__);
        return;
    }

    PsyDuration *buffer_duration // owned by the device
        = psy_audio_device_get_buffer_duration(priv->device);

    PsyTimePoint *tp_start // owned by the stimulus
        = psy_stimulus_get_start_time(PSY_STIMULUS(stimulus));

    PsyDuration *onset_dur = psy_time_point_subtract(tp_start, tp_sample);

    if (psy_duration_less(onset_dur, buffer_duration)) {
        g_warning("scheduling an auditory stimulus within %lf seconds from "
                  "last frame is to soon or in the past, presenting it as "
                  "quickly as possible.",
                  psy_duration_get_seconds(onset_dur));
    }

    psy_duration_destroy(onset_dur);
    psy_time_point_destroy(tp_sample);
}

/**
 * psy_audio_mixer_set_audio_device:(skip):
 *
 * This function should only be during the construction of an audio mixer.
 *
 * stability:private:
 */
void
psy_audio_mixer_set_audio_device(PsyAudioMixer *self, PsyAudioDevice *device)
{
    g_return_if_fail(PSY_IS_AUDIO_MIXER(self) && PSY_IS_AUDIO_DEVICE(device));

    PsyAudioMixerPrivate *priv = psy_audio_mixer_get_instance_private(self);

    g_clear_object(&priv->device);

    priv->device = g_object_ref(device);
}

/**
 * psy_audio_mixer_get_audio_device:
 *
 * Returns:(transfer none): The AudioDevice that created this mixer.
 */
PsyAudioDevice *
psy_audio_mixer_get_audio_device(PsyAudioMixer *self)
{
    g_return_val_if_fail(PSY_IS_AUDIO_MIXER(self), NULL);
    PsyAudioMixerPrivate *priv = psy_audio_mixer_get_instance_private(self);

    return priv->device;
}

guint
psy_audio_mixer_get_num_in_channels(PsyAudioMixer *self)
{
    g_return_val_if_fail(PSY_IS_AUDIO_MIXER(self), 0);

    PsyAudioMixerPrivate *priv = psy_audio_mixer_get_instance_private(self);

    g_return_val_if_fail(PSY_IS_AUDIO_DEVICE(priv->device), 0);

    return psy_audio_device_get_num_input_channels(priv->device);
}

guint
psy_audio_mixer_get_num_out_channels(PsyAudioMixer *self)
{
    g_return_val_if_fail(PSY_IS_AUDIO_MIXER(self), 0);

    PsyAudioMixerPrivate *priv = psy_audio_mixer_get_instance_private(self);

    g_return_val_if_fail(PSY_IS_AUDIO_DEVICE(priv->device), 0);

    return psy_audio_device_get_num_output_channels(priv->device);
}

guint
psy_audio_mixer_read_frames(PsyAudioMixer *self, guint num_frames, gfloat *data)
{
    PsyAudioMixerPrivate *priv = psy_audio_mixer_get_instance_private(self);
    g_return_val_if_fail(PSY_IS_AUDIO_MIXER(self), 0);
    g_return_val_if_fail(data != NULL, 0);

    guint num_out_channels = psy_audio_mixer_get_num_out_channels(self);
    guint num_samples      = num_frames * num_out_channels;

    guint num_popped
        = psy_audio_queue_pop_samples(priv->out_queue, num_samples, data);
    return num_popped;
}

PsyAudioSampleRate
psy_audio_mixer_get_sample_rate(PsyAudioMixer *self)
{
    g_return_val_if_fail(PSY_IS_AUDIO_MIXER(self), 0);

    PsyAudioMixerPrivate *priv = psy_audio_mixer_get_instance_private(self);

    g_return_val_if_fail(PSY_IS_AUDIO_DEVICE(priv->device), 0);

    return psy_audio_device_get_sample_rate(priv->device);
}

/**
 * psy_audio_mixer_process_audio:
 * @self: An instance of [class@PsyAudioMixer]
 *
 * virtual function that should be implemented in the deriving classes
 * That maintains the buffer strategy of the mixer.
 */
void
psy_audio_mixer_process_audio(PsyAudioMixer *self)
{
    g_return_if_fail(PSY_IS_AUDIO_MIXER(self));

    PsyAudioMixerClass *cls = PSY_AUDIO_MIXER_GET_CLASS(self);

    g_return_if_fail(cls->process_audio != NULL);

    cls->process_audio(self);
}
