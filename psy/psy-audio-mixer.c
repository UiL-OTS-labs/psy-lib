

#include "psy-audio-mixer.h"
#include "enum-types.h"
#include "psy-audio-device.h"
#include "psy-config.h"
#include "psy-enums.h"

/**
 * PsyAudioMixer:(skip)
 *
 * The AudioMixer is considered to be a private to Psylib in practice the
 * AudioDevice will create one instance of this class.
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
 * stability: private
 */

#define DEFAULT_NUM_STIM_CACHE 16

typedef struct _PsyAudioMixer {
    GObject            parent;
    PsyAudioDevice    *device;
    PsyAudioSampleRate sample_rate;
    guint              num_channels;
    guint              num_buffer_samples;
    GPtrArray         *stimuli;
} PsyAudioMixerPrivate;

G_DEFINE_FINAL_TYPE(PsyAudioMixer, psy_audio_mixer, G_TYPE_OBJECT)

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

static void
psy_audio_mixer_init(PsyAudioMixer *self)
{
    self->device = NULL;
    self->stimuli
        = g_ptr_array_new_full(DEFAULT_NUM_STIM_CACHE, g_object_unref);
}

static void
psy_audio_mixer_dispose(GObject *object)
{
    PsyAudioMixer *self = PSY_AUDIO_MIXER(object);

    g_clear_object(&self->device);
    g_ptr_array_unref(self->stimuli);
    self->stimuli = NULL;

    G_OBJECT_CLASS(psy_audio_mixer_parent_class)->dispose(object);
}

static void
psy_audio_mixer_finalize(GObject *object)
{
    PsyAudioMixer *self = PSY_AUDIO_MIXER(object);

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
                            0,
                            G_MAXUINT,
                            0,
                            G_PARAM_READABLE);

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
 * psy_audio_mixer_new:(constructor)
 *
 * Constructs an audio mixer.
 *
 * Returns: A platform specific instance of [class@PsyAudioMixer]
 */
PsyAudioMixer *
psy_audio_mixer_new(PsyAudioDevice *device)
{
    return g_object_new(PSY_TYPE_AUDIO_MIXER, "audio-device", device, NULL);
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

    g_clear_object(&self->device);

    self->device       = g_object_ref(device);
    self->num_channels = psy_audio_device_get_num_output_channels(device);
    // self->num_buffer_samples = psy_audio_device_num_buffer_samples(device);
    self->sample_rate  = psy_audio_device_get_sample_rate(device);
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

    return self->device;
}

guint
psy_audio_mixer_get_num_channels(PsyAudioMixer *self)
{
    g_return_val_if_fail(PSY_IS_AUDIO_MIXER(self), 0);

    return psy_audio_device_get_num_output_channels(self->device);
}

PsyAudioSampleRate
psy_audio_mixer_get_sample_rate(PsyAudioMixer *self)
{
    g_return_val_if_fail(PSY_IS_AUDIO_MIXER(self), 0);

    return psy_audio_device_get_sample_rate(self->device);
}

guint
psy_audio_mixer_get_num_buffered_samples(PsyAudioMixer *self)
{
    g_return_val_if_fail(PSY_IS_AUDIO_MIXER(self), 0);

    return psy_audio_device_get_num_samples_callback(self->device)
           * psy_audio_device_get_num_output_channels(self->device);
}

///**
// * psy_audio_mixer_set_sample_rate:(skip)
// * @self: an instance of [class@AudioMixer]
// * @sample_rate: The sample rate of configured for the audio pipeline
// *
// * This property may only be set when constructing the mixer. This property
// * should match the [property@Psy.AudioDevice:sample-rate] of the output
// device.
// *
// * Stability: private
// */
// void
// psy_audio_mixer_set_sample_rate(PsyAudioMixer     *self,
//                                PsyAudioSampleRate sample_rate);

///**
// * psy_audio_mixer_set_num_channels:(skip)
// * @self: an instance of [class@AudioMixer]
// * @num_channels: The number of channels of the output device.
// *
// * This property may only be set when constructing the mixer.
// *
// * Stability: private
// */
// void
// psy_audio_mixer_set_num_channels(PsyAudioMixer *self, guint num_channels);

// void
// psy_audio_mixer_set_bufsize(PsyAudioMixer *self, guint bufsize);
