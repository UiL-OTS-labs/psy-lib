
#include <math.h>

#include "psy-audio-utils.h"
#include "psy-safe-int-private.h"

/**
 * psy_int_to_sample_rate:
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
    default:
        return PSY_AUDIO_SAMPLE_RATE_UNKNOWN;
    }
}

/**
 * psy_num_audio_samples_to_duration:
 * @num_samples: the number of samples to convert to a duration
 * @sr: The sampling rate of the audio
 *
 * Calculates the duration of a number of samples given a sampling rate.
 * This is a much more accurate way to calculate the duration of for a number
 * of audio samples than using [method@Duration.multiply_scalar] as
 * the resolution of [struct@Duration] is in microseconds and 1.0/44100 e.g.
 * isn't a whole number of micro seconds. So the multiplication with a large
 * number of samples loses quite some time.
 *
 * To do the reverse operation see [method@Duration.to_num_audio_frames]
 *
 * Returns:(transfer full)(nullable): a duration for that number of samples. The
 * result might be null if an overflow is detected.
 */
PsyDuration *
psy_num_audio_samples_to_duration(gint64 num_samples, PsyAudioSampleRate sr)
{
    const gint64 us_per_s = 1000000;

    gint64 us;
    gint64 sr_internal = sr;

    gboolean overflows = psy_safe_mul_gint64(us_per_s, num_samples, &us);

    g_return_val_if_fail(!overflows, NULL);

    us = us_per_s * num_samples / sr_internal;

    return psy_duration_new_us(us);
}

/**
 * psy_duration_to_num_audio_frames:
 * @dur:(transfer none): An duration to be converted in a number of frames.
 * @sr: the sample rate used for the conversion
 *
 * Converts a duration to a number of samples. You'll get a rounded result to
 * the nearest frame.
 *
 * Returns: The number of frames that represents this duration the most
 * accurately
 */
gint64
psy_duration_to_num_audio_frames(PsyDuration *dur, PsyAudioSampleRate sr)
{
    return (gint64) round(psy_duration_get_seconds(dur) * sr);
}
