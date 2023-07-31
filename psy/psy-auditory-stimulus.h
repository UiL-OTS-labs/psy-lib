
#pragma once

#include "psy-stimulus.h"

struct PsyAudioDevice;
typedef struct _PsyAudioDevice PsyAudioDevice;

G_BEGIN_DECLS

#define PSY_TYPE_AUDITORY_STIMULUS psy_auditory_stimulus_get_type()
G_DECLARE_DERIVABLE_TYPE(PsyAuditoryStimulus,
                         psy_auditory_stimulus,
                         PSY,
                         AUDITORY_STIMULUS,
                         PsyStimulus)

/**
 * PsyAuditoryStimulusClass:
 * @parent: the parentclass of PsyAuditoryStimulus it derives form PsyStimulus.
 */
typedef struct _PsyAuditoryStimulusClass {
    PsyStimulusClass parent;

    gpointer reserved[12];

} PsyAuditoryStimulusClass;

G_MODULE_EXPORT PsyAudioDevice *
psy_auditory_stimulus_get_audio_device(PsyAuditoryStimulus *self);

G_MODULE_EXPORT void
psy_auditory_stimulus_set_audio_device(PsyAuditoryStimulus *stimulus,
                                       PsyAudioDevice      *audio_device);

G_MODULE_EXPORT gint64
psy_auditory_stimulus_get_num_frames(PsyAuditoryStimulus *self);

G_MODULE_EXPORT gboolean
psy_auditory_stimulus_is_scheduled(PsyAuditoryStimulus *stimulus);

G_MODULE_EXPORT void
psy_auditory_stimulus_set_start_frame(PsyAuditoryStimulus *self,
                                      gint64               frame_num);

G_MODULE_EXPORT gint64
psy_auditory_stimulus_get_start_frame(PsyAuditoryStimulus *self);

G_END_DECLS
