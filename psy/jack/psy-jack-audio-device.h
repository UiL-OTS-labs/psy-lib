#ifndef PSY_JACK_AUDIO_DEVICE_H
#define PSY_JACK_AUDIO_DEVICE_H

#include "../psy-audio-device.h"

G_BEGIN_DECLS

#define PSY_TYPE_JACK_AUDIO_DEVICE psy_jack_audio_device_get_type()
G_DECLARE_FINAL_TYPE(PsyJackAudioDevice,
                     psy_jack_audio_device,
                     PSY,
                     JACK_AUDIO_DEVICE,
                     PsyAudioDevice)

G_MODULE_EXPORT PsyJackAudioDevice *
psy_jack_audio_device_new(void);

G_MODULE_EXPORT void
psy_jack_audio_device_free(PsyJackAudioDevice *self);

G_END_DECLS

#endif
