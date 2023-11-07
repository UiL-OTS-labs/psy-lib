#ifndef PSY_PA_DEVICE_H
#define PSY_PA_DEVICE_H

#include "../psy-audio-device.h"

G_BEGIN_DECLS

#define PSY_TYPE_PA_DEVICE psy_pa_device_get_type()
G_DECLARE_FINAL_TYPE(PsyPADevice, psy_pa_device, PSY, PA_DEVICE, PsyAudioDevice)

PsyAudioDevice *
psy_pa_device_new(void);

G_END_DECLS

#endif
