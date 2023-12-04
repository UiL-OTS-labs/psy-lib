#ifndef PSY_AUDIO_MIXER_H
#define PSY_AUDIO_MIXER_H

#include <gio/gio.h>
#include <psy-enums.h>

#include <psy-auditory-stimulus.h>
#include <psy-queue.h>

/*
 * The PsyAudioMixer is considered to be a private object.
 */

G_BEGIN_DECLS

#define PSY_TYPE_AUDIO_MIXER psy_audio_mixer_get_type()
G_DECLARE_DERIVABLE_TYPE(
    PsyAudioMixer, psy_audio_mixer, PSY, AUDIO_MIXER, GObject)

/**
 * PsyAudioMixerClass:
 * @parent_class: The parent class
 * @process_audio: A function that is called repeatedly. For outputs it
 *                 prepares the AudioOutputMixer that there is enough
 *                 data ready to be read by the audio_callback so it can fetch a
 *                 the samples it needs. For inputs it empties the mixer so that
 *                 the audio callback can write to the
 *                 PsyAudioInputMixer
 *
 * The psy mixer class is mainly a holder for a PsyAudioQueue. It is ment
 * as a parent for PsyMixerIn and -outputs.
 */

typedef struct _PsyAudioMixerClass {
    GObjectClass parent_class;

    void (*process_audio)(PsyAudioMixer *self);

    /*< private >*/

    gpointer padding[12];
} PsyAudioMixerClass;

G_MODULE_EXPORT PsyAudioMixer *
psy_audio_mixer_new(PsyAudioDevice *device);

G_MODULE_EXPORT void
psy_audio_mixer_schedule_stimulus(PsyAudioMixer       *self,
                                  PsyAuditoryStimulus *stimulus);

G_MODULE_EXPORT void
psy_audio_mixer_set_audio_device(PsyAudioMixer *self, PsyAudioDevice *device);

G_MODULE_EXPORT PsyAudioDevice *
psy_audio_mixer_get_audio_device(PsyAudioMixer *self);

G_MODULE_EXPORT PsyAudioSampleRate
psy_audio_mixer_get_sample_rate(PsyAudioMixer *self);

G_MODULE_EXPORT void
psy_audio_mixer_set_sample_rate(PsyAudioMixer     *self,
                                PsyAudioSampleRate sample_rate);

G_MODULE_EXPORT guint
psy_audio_mixer_get_num_channels(PsyAudioMixer *self);

G_MODULE_EXPORT void
psy_audio_mixer_set_num_channels(PsyAudioMixer *self, guint num_channels);

G_MODULE_EXPORT guint
psy_audio_mixer_get_num_buffered_samples(PsyAudioMixer *self);

G_MODULE_EXPORT void
psy_audio_mixer_set_num_buffered_samples(PsyAudioMixer *self,
                                         guint          num_samples);

G_MODULE_EXPORT guint
psy_audio_mixer_read_samples(PsyAudioMixer *self,
                             guint          num_samples,
                             gfloat        *data);

G_MODULE_EXPORT PsyAudioQueue *
psy_audio_mixer_get_queue(PsyAudioMixer *self);

G_MODULE_EXPORT void
psy_audio_mixer_process_audio(PsyAudioMixer *self);

G_END_DECLS

#endif