

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

G_DEFINE_FINAL_TYPE(PsyAudioOutputMixer, psy_audio_output_mixer, G_TYPE_OBJECT)

typedef enum { PROP_NULL, NUM_PROPERTIES } PsyAudioOutputMixerProperty;

static GParamSpec *audio_output_mixer_properties[NUM_PROPERTIES];

// static guint       audio_output_mixer_signals[NUM_SIGNALS];

/* ********** virtual(/private) functions ***************** */

static void
psy_audio_output_mixer_set_property(GObject      *object,
                                    guint         prop_id,
                                    const GValue *value,
                                    GParamSpec   *pspec)
{
    PsyAudioOutputMixer *self = PSY_AUDIO_OUTPUT_MIXER(object);
    (void) self;
    (void) value;

    switch ((PsyAudioOutputMixerProperty) prop_id) {
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

static void
psy_audio_output_mixer_get_property(GObject    *object,
                                    guint       prop_id,
                                    GValue     *value,
                                    GParamSpec *pspec)
{
    PsyAudioOutputMixer *self = PSY_AUDIO_OUTPUT_MIXER(object);

    switch ((PsyAudioOutputMixerProperty) prop_id) {
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

static void
psy_audio_output_mixer_init(PsyAudioOutputMixer *self)
{
    self->stimuli
        = g_ptr_array_new_full(DEFAULT_NUM_STIM_CACHE, g_object_unref);
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

    gobject_class->set_property = psy_audio_output_mixer_set_property;
    gobject_class->get_property = psy_audio_output_mixer_get_property;
    gobject_class->finalize     = psy_audio_output_mixer_finalize;
    gobject_class->dispose      = psy_audio_output_mixer_dispose;

    g_object_class_install_properties(
        gobject_class, NUM_PROPERTIES, audio_output_mixer_properties);
}

/* ************ public functions ******************** */

/**
 * psy_audio_output_mixer_new:(constructor)
 *
 * Constructs an audio mixer.
 *
 * Returns: A platform specific instance of [class@PsyAudioOutputMixer]
 */
PsyAudioOutputMixer *
psy_audio_output_mixer_new(PsyAudioDevice *device)
{
    return g_object_new(
        PSY_TYPE_AUDIO_OUTPUT_MIXER, "audio-device", device, NULL);
}

guint
psy_audio_output_mixer_read_samples(PsyAudioOutputMixer *self,
                                    guint                num_samples,
                                    gfloat              *buffer)
{
    g_return_val_if_fail(PSY_IS_AUDIO_OUTPUT_MIXER(self), 0u);
    g_return_val_if_fail(!buffer, 0u);

    // access in a C++ protected fashion.
    PsyAudioQueue *queue = psy_audio_mixer_get_queue(self);

    gsize n = psy_audio_queue_pop_samples(queue, num_samples, buffer);

    return n;
}
