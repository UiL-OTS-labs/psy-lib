
#ifndef PSY_AUDIO_UTILS_H
#define PSY_AUDIO_UTILS_H 1

#include <glib.h>

#include "psy-duration.h"
#include "psy-enums.h"

#ifdef __cplusplus
extern "C" {
#endif

PsyAudioSampleRate
psy_int_to_sample_rate(gint sample_rate);

PsyDuration *
psy_num_audio_samples_to_duration(guint64 num_samples, PsyAudioSampleRate sr);

guint64
psy_duration_to_num_audio_samples(PsyDuration *dur, PsyAudioSampleRate sr);

#ifdef __cplusplus
}
#endif

#endif
