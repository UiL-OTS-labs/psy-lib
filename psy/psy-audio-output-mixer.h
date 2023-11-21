#ifndef PSY_AUDIO_OUTPUT_MIXER_H
#define PSY_AUDIO_OUTPUT_MIXER_H

#include <psy-enums.h>

#include <psy-audio-mixer.h>
#include <psy-auditory-stimulus.h>

/*
 * The PsyAudioOutputMixer is considered to be a private object.
 */

G_BEGIN_DECLS

#define PSY_TYPE_AUDIO_OUTPUT_MIXER psy_audio_output_mixer_get_type()
G_DECLARE_FINAL_TYPE(PsyAudioOutputMixer,
                     psy_audio_output_mixer,
                     PSY,
                     AUDIO_OUTPUT_MIXER,
                     PsyAudioMixer)

PsyAudioOutputMixer *
psy_audio_output_mixer_new(PsyAudioDevice *device);

void
psy_audio_output_mixer_schedule_stimulus(PsyAudioOutputMixer *self,
                                         PsyAuditoryStimulus *stimulus);

guint
psy_audio_output_mixer_read_samples(PsyAudioOutputMixer *self,
                                    guint                num_samples,
                                    gfloat              *data);

G_END_DECLS

#endif
