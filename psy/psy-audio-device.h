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

/**
 * PsyAudioDeviceClass:
 * @param parent_class: the parent of this class
 * @param open: This function will open the device. This means setting up
 *              the sample rate, sample format, number of channels and if this
 *              succeeds the base class will instantiate a PsyAudioMixer
 *              matching these parameters. Finally it will also start the
 *              device.
 * @param start: After the device is opened it may be started, this means the
 *               device will trigger the audio callback likely in another
 *               thread.
 * @param stop: undo's the start.
 * @param close: undo's open.
 * @get_default_name: Gets the default name for a device, this should always
 *                    open a default device, for the backend.
 * @schedule_stimulus: This schedules a stimulus for this audio_device. The
 *                     PsyAuditoryStimulus will be registered with the
 *                     [class@AudioMixer] which will mix it in at the
 *                     appropriate time. TODO may remove as it probably doesn't
 *                     need to be virtual.
 *
 * A base class for handling audio devices.
 */
typedef struct _PsyAudioDeviceClass {
    GObjectClass parent_class;

    void (*open)(PsyAudioDevice *self, GError **error);
    void (*start)(PsyAudioDevice *self, GError **error);
    void (*stop)(PsyAudioDevice *self);
    void (*close)(PsyAudioDevice *self);

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

G_MODULE_EXPORT void
psy_audio_device_start(PsyAudioDevice *self, GError **error);

G_MODULE_EXPORT void
psy_audio_device_stop(PsyAudioDevice *self);

G_MODULE_EXPORT const gchar *
psy_audio_device_get_name(PsyAudioDevice *self);

G_MODULE_EXPORT gboolean
psy_audio_device_set_name(PsyAudioDevice *self, const gchar *name);

G_MODULE_EXPORT gboolean
psy_audio_device_get_is_open(PsyAudioDevice *self);

G_MODULE_EXPORT gboolean
psy_audio_device_get_is_started(PsyAudioDevice *self);

G_MODULE_EXPORT const gchar *
psy_audio_device_get_default_name(PsyAudioDevice *self);

G_MODULE_EXPORT gboolean
psy_audio_device_set_num_input_channels(PsyAudioDevice *self, guint n_channels);
G_MODULE_EXPORT guint
psy_audio_device_get_num_input_channels(PsyAudioDevice *self);

G_MODULE_EXPORT gboolean
psy_audio_device_set_num_output_channels(PsyAudioDevice *self,
                                         guint           n_channels);
G_MODULE_EXPORT guint
psy_audio_device_get_num_output_channels(PsyAudioDevice *self);

G_MODULE_EXPORT gboolean
psy_audio_device_set_sample_rate(PsyAudioDevice    *self,
                                 PsyAudioSampleRate sample_rate);
G_MODULE_EXPORT PsyAudioSampleRate
psy_audio_device_get_sample_rate(PsyAudioDevice *self);

G_MODULE_EXPORT gboolean
psy_audio_device_get_started(PsyAudioDevice *self);

G_MODULE_EXPORT PsyDuration *
psy_audio_device_get_frame_dur(PsyAudioDevice *self);

G_MODULE_EXPORT guint
psy_audio_device_get_num_samples_callback(PsyAudioDevice *self);

void
psy_audio_device_set_num_samples_callback(PsyAudioDevice *self,
                                          guint           num_samples);

void
psy_audio_device_set_started(PsyAudioDevice *self, PsyTimePoint *tp_start);

void
psy_audio_device_schedule_stimulus(PsyAudioDevice      *self,
                                   PsyAuditoryStimulus *stim);

typedef struct _PsyAudioMixer PsyAudioMixer;
PsyAudioMixer *
psy_audio_device_get_mixer(PsyAudioDevice *self);

// TODO
// PsyAudioPlayback*
// psy_audio_device_get_playback(PsyAudioDevice* device);

G_END_DECLS

#endif
