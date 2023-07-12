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

PsyAudioDevice *
psy_jack_audio_device_new(void);

G_END_DECLS

#endif
