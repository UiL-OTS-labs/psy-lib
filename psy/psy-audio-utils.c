
#include "psy-audio-utils.h"

/**
 * pys_int_to_sample_rate:
 * @sample_rate the sample rate in as integer
 *
 * Returns: the sample format from the PsyAudioSampleRate enum
 */
PsyAudioSampleRate
psy_int_to_sample_rate(gint sample_rate)
{
    switch (sample_rate) {
    case 22050:
        return PSY_AUDIO_SAMPLE_RATE_22050;
    case 24000:
        return PSY_AUDIO_SAMPLE_RATE_24000;
    case 32000:
        return PSY_AUDIO_SAMPLE_RATE_32000;
    case 44100:
        return PSY_AUDIO_SAMPLE_RATE_44100;
    case 48000:
        return PSY_AUDIO_SAMPLE_RATE_48000;
    case 88200:
        return PSY_AUDIO_SAMPLE_RATE_88200;
    case 96000:
        return PSY_AUDIO_SAMPLE_RATE_96000;
    case 192000:
        return PSY_AUDIO_SAMPLE_RATE_192000;
    default:;
    }
    return PSY_AUDIO_SAMPLE_RATE_UNKNOWN;
}
