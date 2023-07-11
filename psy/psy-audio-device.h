#ifndef PSY_AUDIO_DEVICE_H
#define PSY_AUDIO_DEVICE_H

#include <gio/gio.h>

#include <psy-matrix4.h>
#include <psy-program.h>
#include <psy-texture.h>
#include <psy-vbuffer.h>

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

    gpointer extensions[16];

} PsyAudioDeviceClass;

PsyAudioDevice *
psy_audio_device_new(void);

void
psy_audio_device_open(PsyAudioDevice *self, GError **error);

const gchar *
psy_audio_device_get_name(PsyAudioDevice *self);

void
psy_audio_device_set_name(PsyAudioDevice *self, const gchar *name);

guint
psy_audio_get_sample_rate(PsyAudioDevice *self);

guint
psy_audio_set_sample_rate(PsyAudioDevice *self, guint sample_rate);

gboolean
psy_audio_device_get_is_open(PsyAudioDevice *self);

const gchar *
psy_audio_device_get_default_name(PsyAudioDevice *self);

// TODO
// PsyAudioPlayback*
// psy_audio_device_get_playback(PsyAudioDevice* device);

G_END_DECLS

#endif
