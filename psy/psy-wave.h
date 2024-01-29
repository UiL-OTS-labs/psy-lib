
#pragma once

#include "psy-gst-stimulus.h"

#include "psy-enums.h"

G_BEGIN_DECLS

#define PSY_TYPE_WAVE psy_wave_get_type()

G_DECLARE_FINAL_TYPE(PsyWave, psy_wave, PSY, WAVE, PsyGstStimulus)

G_MODULE_EXPORT PsyWave *
psy_wave_new(PsyAudioDevice *device);

G_MODULE_EXPORT PsyWave *
psy_wave_new_volume(PsyAudioDevice *device, gdouble volume);

G_MODULE_EXPORT PsyWave *
psy_wave_tone_new(PsyAudioDevice *device, gdouble hz, gdouble volume);

G_MODULE_EXPORT void
psy_wave_free(PsyWave *self);

G_MODULE_EXPORT gdouble
psy_wave_get_volume(PsyWave *self);

G_MODULE_EXPORT void
psy_wave_set_volume(PsyWave *self, gdouble volume);

G_MODULE_EXPORT gdouble
psy_wave_get_freq(PsyWave *self);

G_MODULE_EXPORT void
psy_wave_set_freq(PsyWave *self, gdouble freq);

G_MODULE_EXPORT PsyWaveForm
psy_wave_get_form(PsyWave *self);

G_MODULE_EXPORT void
psy_wave_set_form(PsyWave *self, PsyWaveForm form);

G_END_DECLS
