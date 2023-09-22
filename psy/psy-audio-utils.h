
#ifndef PSY_AUDIO_UTILS_H
#define PSY_AUDIO_UTILS_H 1

#include <glib.h>

#include "psy-enums.h"

#ifdef __cplusplus
extern "C" {
#endif

PsyAudioSampleRate
psy_int_to_sample_rate(gint sample_rate);

#ifdef __cplusplus
}
#endif

#endif
