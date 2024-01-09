
#ifndef PSY_AUDITORY_STIMULUS_H
#define PSY_AUDITORY_STIMULUS_H

#include "psy-audio-channel-map.h"
#include "psy-stimulus.h"

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
 * @has_flexible_num_channels: abstract method that needs to be implemented in
 *     the child. It returns a boolean to indicate what the result is for the
 *     [property@AuditoryStimulus:flexible_num_channels]
 * @add_channel_map: a method that adds a ChannelMap to the stimulus when is
 *     ready to be presented.
 */
typedef struct _PsyAuditoryStimulusClass {
    PsyStimulusClass parent;

    gboolean (*get_flexible_num_channels)(void);
    void (*add_channel_map)(PsyAuditoryStimulus *self,
                            PsyAudioDevice      *device,
                            gpointer             data);

    guint (*read)(PsyAuditoryStimulus *self, guint num_samples, gfloat *result);

    gpointer reserved[12];

} PsyAuditoryStimulusClass;

G_MODULE_EXPORT PsyAudioDevice *
psy_auditory_stimulus_get_audio_device(PsyAuditoryStimulus *self);

G_MODULE_EXPORT void
psy_auditory_stimulus_set_audio_device(PsyAuditoryStimulus *self,
                                       PsyAudioDevice      *audio_device);

G_MODULE_EXPORT gint64
psy_auditory_stimulus_get_num_frames(PsyAuditoryStimulus *self);

G_MODULE_EXPORT gint64
psy_auditory_stimulus_get_num_frames_presented(PsyAuditoryStimulus *self);

G_MODULE_EXPORT gboolean
psy_auditory_stimulus_is_scheduled(PsyAuditoryStimulus *self);

void
psy_auditory_stimulus_set_start_frame(PsyAuditoryStimulus *self,
                                      gint64               frame_num);

G_MODULE_EXPORT gint64
psy_auditory_stimulus_get_start_frame(PsyAuditoryStimulus *self);

G_MODULE_EXPORT gboolean
psy_auditory_stimulus_get_flexible_num_channels(PsyAuditoryStimulus *self);

G_MODULE_EXPORT PsyAudioChannelMap *
psy_auditory_stimulus_get_channel_map(PsyAuditoryStimulus *self);

G_MODULE_EXPORT void
psy_auditory_stimulus_set_channel_map(PsyAuditoryStimulus *self,
                                      PsyAudioChannelMap  *map);

G_MODULE_EXPORT guint
psy_auditory_stimulus_get_num_channels(PsyAuditoryStimulus *self);

G_MODULE_EXPORT void
psy_auditory_stimulus_set_num_channels(PsyAuditoryStimulus *self,
                                       guint                num_channels);

G_MODULE_EXPORT guint
psy_auditory_stimulus_read(PsyAuditoryStimulus *self,
                           guint                num_samples,
                           gfloat              *result);

G_END_DECLS

#endif
