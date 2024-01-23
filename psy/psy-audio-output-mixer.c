

#include "psy-config.h"

#include "enum-types.h"

#include "psy-audio-output-mixer.h"
#include "psy-enums.h"
#include "psy-queue.h"

/**
 * PsyAudioOutputMixer:(skip)
 *
 * The AudioOutputMixer is considered to be a private to Psylib in practice the
 * AudioDevice will create one instance of this class.
 *
 * A PsyAudioOutputMixer is a mixer that mixes the scheduled PsyAuditoryStimuli
 * together. It maintains an buffer with audio samples, Psylib must make sure
 * that this audio buffer is ready to produce samples for the audio callback.
 * the settings of the PsyAudioOutputMixer should always match the settings
 * for the PsyAudioDevice.
 *
 * This class is used by PsyAudioDevices to buffer audio in such a way
 * that the audio callback can retrieve samples from this buffer very quickly.
 *
 * stability: private
 */

#define DEFAULT_NUM_STIM_CACHE 16

typedef struct _PsyAudioOutputMixer {
    PsyAudioMixer parent;
    GPtrArray    *stimuli;
} PsyAudioOutputMixer;

G_DEFINE_FINAL_TYPE(PsyAudioOutputMixer,
                    psy_audio_output_mixer,
                    PSY_TYPE_AUDIO_MIXER)

// typedef enum {
//     PROP_NULL,
//     NUM_PROPERTIES
// } PsyAudioOutputMixerProperty;

// static GParamSpec *audio_output_mixer_properties[NUM_PROPERTIES];

// static guint       audio_output_mixer_signals[NUM_SIGNALS];

/* ********** virtual(/private) functions ***************** */

// static void
// psy_audio_output_mixer_set_property(GObject      *object,
//                                     guint         prop_id,
//                                     const GValue *value,
//                                     GParamSpec   *pspec)
//{
//     PsyAudioOutputMixer *self = PSY_AUDIO_OUTPUT_MIXER(object);
//
//     switch ((PsyAudioOutputMixerProperty) prop_id) {
//     case PROP_NUM_BUFFER_SAMPLES:
//         self->num_buffer_samples = g_value_get_uint(value);
//         break;
//     default:
//         G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
//     }
// }
//
// static void
// psy_audio_output_mixer_get_property(GObject    *object,
//                                     guint       prop_id,
//                                     GValue     *value,
//                                     GParamSpec *pspec)
//{
//     PsyAudioOutputMixer *self = PSY_AUDIO_OUTPUT_MIXER(object);
//
//     switch ((PsyAudioOutputMixerProperty) prop_id) {
//     case PROP_NUM_BUFFER_SAMPLES:
//         g_value_set_uint(value, self->num_buffer_samples);
//         break;
//     default:
//         G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
//     }
// }

static void
output_mixer_process_audio(PsyAudioMixer *self)
{
    // PsyAudioOutputMixer *out_mixer = PSY_AUDIO_OUTPUT_MIXER(self);
    PsyAudioQueue *queue = psy_audio_mixer_get_queue(PSY_AUDIO_MIXER(self));

    gfloat sample = 0.0f;

    while (psy_audio_queue_push_samples(queue, 1, &sample) == 1)
        ;
}

static void
psy_audio_output_mixer_init(PsyAudioOutputMixer *self)
{
    self->stimuli
        = g_ptr_array_new_full(DEFAULT_NUM_STIM_CACHE, g_object_unref);
}

static void
audio_output_mixer_constructed(GObject *self)
{
    // First chain up in order to create a queue
    G_OBJECT_CLASS(psy_audio_output_mixer_parent_class)->constructed(self);

    // Fill the output buffer
    PsyAudioMixer       *mixer_self        = PSY_AUDIO_MIXER(self);
    PsyAudioOutputMixer *output_mixer_self = PSY_AUDIO_OUTPUT_MIXER(self);

    guint num_samples = psy_audio_mixer_get_num_buffered_samples(mixer_self);

    PsyAudioQueue *queue = psy_audio_mixer_get_queue(mixer_self);

    for (gsize i = 0; i < num_samples; i++) {
        gfloat sample = 0.0f;
        psy_audio_queue_push_samples(queue, 1, &sample);
    }
}

static void
psy_audio_output_mixer_dispose(GObject *object)
{
    PsyAudioOutputMixer *self = PSY_AUDIO_OUTPUT_MIXER(object);

    g_ptr_array_unref(self->stimuli);
    self->stimuli = NULL;

    G_OBJECT_CLASS(psy_audio_output_mixer_parent_class)->dispose(object);
}

static void
psy_audio_output_mixer_finalize(GObject *object)
{
    PsyAudioOutputMixer *self = PSY_AUDIO_OUTPUT_MIXER(object);
    (void) self;

    G_OBJECT_CLASS(psy_audio_output_mixer_parent_class)->finalize(object);
}

static void
psy_audio_output_mixer_class_init(PsyAudioOutputMixerClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

    //    gobject_class->set_property = psy_audio_output_mixer_set_property;
    //    gobject_class->get_property = psy_audio_output_mixer_get_property;
    gobject_class->finalize    = psy_audio_output_mixer_finalize;
    gobject_class->dispose     = psy_audio_output_mixer_dispose;
    gobject_class->constructed = audio_output_mixer_constructed;

    PsyAudioMixerClass *audio_mixer_cls = PSY_AUDIO_MIXER_CLASS(klass);
    audio_mixer_cls->process_audio      = output_mixer_process_audio;

    //    g_object_class_install_properties(
    //        gobject_class, NUM_PROPERTIES, audio_output_mixer_properties);
}

/* ************ public functions ******************** */

/**
 * psy_audio_output_mixer_new:(constructor)
 *
 * Constructs an audio output mixer.
 *
 * Returns: A platform specific instance of [class@PsyAudioOutputMixer]
 */
PsyAudioOutputMixer *
psy_audio_output_mixer_new(PsyAudioDevice *device, guint num_channels)
{
    // clang-format off
    return g_object_new(PSY_TYPE_AUDIO_OUTPUT_MIXER,
                        "audio-device", device,
                        "num-channels", num_channels,
                        NULL);
    // clang-format on
}

/**
 * psy_audio_output_mixer_read_samples:(skip)
 * @self: an instance of [class@AudioOutputMixer].
 * @num_samples: The number of audio samples that should be read.
 * @buffer:(out caller-allocates): The buffer into which the samples should
 *         be written. The size of this buffer should be large enough to hold
 *         num_samples 32 bit floating point numbers. The data is always
 *         written in an interleaved fashion.
 *
 * Generally audio api's have a callback when data should be provided to
 * a internal buffer. These callbacks provide an argument that represents
 * the memory that needs to be written to.
 * This function can be used to write to such a buffer. This function is a
 * lockfree function that writes a number of floats that represents the number
 * of samples, not corrected for the number of output channels, if you have 2
 * channel audios, and need to write 128 samples, you'll have to pass
 * 128 * 2 ( = 256) to this function
 *
 * Returns: the total number of floats written.
 * Stability: private
 */
guint
psy_audio_output_mixer_read_samples(PsyAudioOutputMixer *self,
                                    guint                num_samples,
                                    gfloat              *buffer)
{
    g_return_val_if_fail(PSY_IS_AUDIO_OUTPUT_MIXER(self), 0u);
    g_return_val_if_fail(buffer != NULL, 0u);

    // access in a C++ protected fashion.
    PsyAudioQueue *queue = psy_audio_mixer_get_queue(PSY_AUDIO_MIXER(self));

    return psy_audio_queue_pop_samples(queue, num_samples, buffer);
}
