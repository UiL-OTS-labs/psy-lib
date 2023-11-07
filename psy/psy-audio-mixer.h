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

/**
 * PsyMixerClass:
 * @parent_class: The parent class
 *
 * The psy mixer class is mainly a holder for a PsyAudioQueue. It is ment
 * as a parent for PsyMixerIn and -outputs.
 */

typedef struct _PsyAudioMixerClass {
    GObjectClass parent_class;

    /*< private >*/

    gpointer padding[12];
} PsyAudioMixerClass;

#define PSY_TYPE_AUDIO_MIXER psy_audio_mixer_get_type()
G_DECLARE_DERIVABLE_TYPE(
    PsyAudioMixer, psy_audio_mixer, PSY, AUDIO_MIXER, GObject)

PsyAudioMixer *
psy_audio_mixer_new(PsyAudioDevice *device);

void
psy_audio_mixer_schedule_stimulus(PsyAudioMixer       *self,
                                  PsyAuditoryStimulus *stimulus);

void
psy_audio_mixer_set_audio_device(PsyAudioMixer *self, PsyAudioDevice *device);

PsyAudioDevice *
psy_audio_mixer_get_audio_device(PsyAudioMixer *self);

PsyAudioSampleRate
psy_audio_mixer_get_sample_rate(PsyAudioMixer *self);

void
psy_audio_mixer_set_sample_rate(PsyAudioMixer     *self,
                                PsyAudioSampleRate sample_rate);

guint
psy_audio_mixer_get_num_channels(PsyAudioMixer *self);

void
psy_audio_mixer_set_num_channels(PsyAudioMixer *self, guint num_channels);

guint
psy_audio_mixer_get_num_buffered_samples(PsyAudioMixer *self);

void
psy_audio_mixer_set_num_buffered_samples(PsyAudioMixer *self,
                                         guint          num_samples);

guint
psy_audio_mixer_read_samples(PsyAudioMixer *self,
                             guint          num_samples,
                             gfloat        *data);

PsyAudioQueue *
psy_audio_mixer_get_queue(PsyAudioMixer *self);

G_END_DECLS

#endif
