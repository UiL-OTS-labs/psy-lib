
#ifndef PSY_AUDIO_UTILS_H
#define PSY_AUDIO_UTILS_H 1

#include <glib.h>

#include "psy-duration.h"
#include "psy-enums.h"

#ifdef __cplusplus
extern "C" {
#endif

G_MODULE_EXPORT PsyAudioSampleRate
psy_int_to_sample_rate(gint sample_rate);

G_MODULE_EXPORT PsyDuration *
psy_num_audio_samples_to_duration(gint64 num_samples, PsyAudioSampleRate sr);

G_MODULE_EXPORT gint64
psy_duration_to_num_audio_frames(PsyDuration *dur, PsyAudioSampleRate sr);

#ifdef __cplusplus
}
#endif

#endif
