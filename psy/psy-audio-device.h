#ifndef PSY_AUDIO_DEVICE_H
#define PSY_AUDIO_DEVICE_H

#include <gio/gio.h>
#include <psy-enums.h>

#include <psy-auditory-stimulus.h>

G_BEGIN_DECLS

#define PSY_AUDIO_DEVICE_ERROR psy_audio_device_error_quark()
G_MODULE_EXPORT GQuark
psy_audio_device_error_quark(void);

#define PSY_TYPE_AUDIO_DEVICE psy_audio_device_get_type()
G_DECLARE_DERIVABLE_TYPE(
    PsyAudioDevice, psy_audio_device, PSY, AUDIO_DEVICE, GObject)

typedef struct _PsyAudioDeviceClass {
    GObjectClass parent_class;

    void (*open)(PsyAudioDevice *self, GError **error);
    void (*close)(PsyAudioDevice *self);
    void (*set_name)(PsyAudioDevice *self, const gchar *name);
    void (*set_sample_rate)(PsyAudioDevice *self, guint sample_rate);

    const gchar *(*get_default_name)(PsyAudioDevice *self);

    void (*schedule_stimulus)(PsyAudioDevice *self, PsyAuditoryStimulus *stim);

    gpointer extensions[16];

} PsyAudioDeviceClass;

G_MODULE_EXPORT PsyAudioDevice *
psy_audio_device_new(void);

G_MODULE_EXPORT void
psy_schedule_stimulus(PsyAudioDevice *self, PsyAuditoryStimulus *stimulus);

G_MODULE_EXPORT void
psy_audio_device_open(PsyAudioDevice *self, GError **error);

G_MODULE_EXPORT void
psy_audio_device_close(PsyAudioDevice *self);

G_MODULE_EXPORT const gchar *
psy_audio_device_get_name(PsyAudioDevice *self);

G_MODULE_EXPORT void
psy_audio_device_set_name(PsyAudioDevice *self, const gchar *name);

G_MODULE_EXPORT gboolean
psy_audio_device_get_is_open(PsyAudioDevice *self);

G_MODULE_EXPORT const gchar *
psy_audio_device_get_default_name(PsyAudioDevice *self);

G_MODULE_EXPORT PsyAudioSampleRate
psy_audio_device_get_sample_rate(PsyAudioDevice *self);

G_MODULE_EXPORT gboolean
psy_audio_device_set_sample_rate(PsyAudioDevice    *self,
                                 PsyAudioSampleRate sample_rate,
                                 GError           **error);

G_MODULE_EXPORT gboolean
psy_audio_device_get_started(PsyAudioDevice *self);

void
psy_audio_device_set_started(PsyAudioDevice *self, PsyTimePoint *tp_start);

G_MODULE_EXPORT PsyDuration *
psy_audio_device_get_frame_dur(PsyAudioDevice *self);

void
psy_audio_device_schedule_stimulus(PsyAudioDevice      *self,
                                   PsyAuditoryStimulus *stim);

// TODO
// PsyAudioPlayback*
// psy_audio_device_get_playback(PsyAudioDevice* device);

G_END_DECLS

#endif
