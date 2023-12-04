

#include "psy-config.h"

#include "enum-types.h"

#include "psy-audio-device.h"
#include "psy-audio-mixer.h"
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
    PsyAudioDevice    *device;
    PsyAudioQueue     *queue;
    PsyAudioSampleRate sample_rate;
    guint              num_channels;
    guint              num_buffer_samples;
    GMainContext      *context;
    GPtrArray         *stimuli;
    guint              process_callback_id;
} PsyAudioMixerPrivate;

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE(PsyAudioMixer,
                                    psy_audio_mixer,
                                    G_TYPE_OBJECT)

typedef enum {
    PROP_NULL,
    PROP_AUDIO_DEVICE,
    PROP_SAMPLE_RATE,
    PROP_NUM_CHANNELS,
    PROP_NUM_BUFFERED_SAMPLES,
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
    PsyAudioMixer *self = PSY_AUDIO_MIXER(object);
    (void) self;
    (void) value;

    switch ((PsyAudioMixerProperty) prop_id) {
    case PROP_AUDIO_DEVICE:
        psy_audio_mixer_set_audio_device(self, g_value_get_object(value));
        break;
    case PROP_NUM_CHANNELS:
        psy_audio_mixer_set_num_channels(self, g_value_get_uint(value));
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
    PsyAudioMixer *self = PSY_AUDIO_MIXER(object);

    switch ((PsyAudioMixerProperty) prop_id) {
    case PROP_SAMPLE_RATE:
        g_value_set_enum(value, psy_audio_mixer_get_sample_rate(self));
        break;
    case PROP_NUM_CHANNELS:
        g_value_set_uint(value, psy_audio_mixer_get_num_channels(self));
        break;
    case PROP_NUM_BUFFERED_SAMPLES:
        g_value_set_uint(value, psy_audio_mixer_get_num_buffered_samples(self));
        break;
    case PROP_AUDIO_DEVICE:
        g_value_set_object(value, psy_audio_mixer_get_audio_device(self));
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
    // construct the queue here as only after initalization has finished
    // the number of buffered samples is known.
    guint num_buffered_samples
        = psy_audio_mixer_get_num_buffered_samples(PSY_AUDIO_MIXER(self));

    g_return_if_fail(num_buffered_samples <= G_MAXINT);

    PsyAudioMixerPrivate *priv
        = psy_audio_mixer_get_instance_private(PSY_AUDIO_MIXER(self));
    priv->queue = psy_audio_queue_new((int) num_buffered_samples);

    G_OBJECT_CLASS(psy_audio_mixer_parent_class)->constructed(self);
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

    psy_audio_queue_free(priv->queue);
    priv->queue = NULL;

    G_OBJECT_CLASS(psy_audio_mixer_parent_class)->finalize(object);
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

    audio_mixer_properties[PROP_NUM_CHANNELS]
        = g_param_spec_uint("num-channels",
                            "NumChannels",
                            "The number of channels for the mixer.",
                            1,
                            G_MAXUINT,
                            1,
                            G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

    audio_mixer_properties[PROP_NUM_BUFFERED_SAMPLES] = g_param_spec_uint(
        "num-buffered-samples",
        "NumBufferedSamples`",
        "The total number of samples that are buffered in the mixer",
        0,
        G_MAXUINT,
        0,
        G_PARAM_READABLE);

    g_object_class_install_properties(
        gobject_class, NUM_PROPERTIES, audio_mixer_properties);
}

/* ************ public functions ******************** */

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

    priv->device       = g_object_ref(device);
    priv->num_channels = psy_audio_device_get_num_output_channels(device);
    // self->num_buffer_samples = psy_audio_device_num_buffer_samples(device);
    priv->sample_rate  = psy_audio_device_get_sample_rate(device);
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

/**
 * psy_audio_mixer_get_audio_queue:
 *
 * Returns the audio queue from the mixer. The audio queue is used to buffer
 * audio between the audio callback and the parts of psylib that generate
 * audio for presentation.
 *
 * Returns:(transfer none): the audio queue
 */
PsyAudioQueue *
psy_audio_mixer_get_queue(PsyAudioMixer *self)
{
    g_return_val_if_fail(PSY_IS_AUDIO_MIXER(self), NULL);

    PsyAudioMixerPrivate *priv = psy_audio_mixer_get_instance_private(self);

    return priv->queue;
}

guint
psy_audio_mixer_get_num_channels(PsyAudioMixer *self)
{
    g_return_val_if_fail(PSY_IS_AUDIO_MIXER(self), 0);
    PsyAudioMixerPrivate *priv = psy_audio_mixer_get_instance_private(self);

    return priv->num_channels;
}

void
psy_audio_mixer_set_num_channels(PsyAudioMixer *self, guint num_channels)
{
    g_return_if_fail(PSY_IS_AUDIO_MIXER(self));
    PsyAudioMixerPrivate *priv = psy_audio_mixer_get_instance_private(self);

    priv->num_channels = num_channels;
}

PsyAudioSampleRate
psy_audio_mixer_get_sample_rate(PsyAudioMixer *self)
{
    g_return_val_if_fail(PSY_IS_AUDIO_MIXER(self), 0);
    PsyAudioMixerPrivate *priv = psy_audio_mixer_get_instance_private(self);

    return psy_audio_device_get_sample_rate(priv->device);
}

guint
psy_audio_mixer_get_num_buffered_samples(PsyAudioMixer *self)
{
    g_return_val_if_fail(PSY_IS_AUDIO_MIXER(self), 0);
    PsyAudioMixerPrivate *priv = psy_audio_mixer_get_instance_private(self);

    return psy_audio_device_get_num_samples_buffer(priv->device)
           * priv->num_channels;
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
