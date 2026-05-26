
#include <psylib.h>

#include "unit-test-utilities.h"

static void
audio_utils_psy_int_to_sample_rate(void)
{
    g_assert_cmpint(psy_int_to_sample_rate(PSY_AUDIO_SAMPLE_RATE_22050),
                    ==,
                    PSY_AUDIO_SAMPLE_RATE_22050);
    g_assert_cmpint(psy_int_to_sample_rate(PSY_AUDIO_SAMPLE_RATE_24000),
                    ==,
                    PSY_AUDIO_SAMPLE_RATE_24000);
    g_assert_cmpint(psy_int_to_sample_rate(PSY_AUDIO_SAMPLE_RATE_32000),
                    ==,
                    PSY_AUDIO_SAMPLE_RATE_32000);
    g_assert_cmpint(psy_int_to_sample_rate(PSY_AUDIO_SAMPLE_RATE_44100),
                    ==,
                    PSY_AUDIO_SAMPLE_RATE_44100);
    g_assert_cmpint(psy_int_to_sample_rate(PSY_AUDIO_SAMPLE_RATE_48000),
                    ==,
                    PSY_AUDIO_SAMPLE_RATE_48000);
    g_assert_cmpint(psy_int_to_sample_rate(PSY_AUDIO_SAMPLE_RATE_88200),
                    ==,
                    PSY_AUDIO_SAMPLE_RATE_88200);
    g_assert_cmpint(psy_int_to_sample_rate(PSY_AUDIO_SAMPLE_RATE_96000),
                    ==,
                    PSY_AUDIO_SAMPLE_RATE_96000);
    g_assert_cmpint(psy_int_to_sample_rate(PSY_AUDIO_SAMPLE_RATE_192000),
                    ==,
                    PSY_AUDIO_SAMPLE_RATE_192000);

    // Could fail very occasionally
    g_assert_cmpint(psy_int_to_sample_rate(g_test_rand_int_range(0, G_MAXINT)),
                    ==,
                    PSY_AUDIO_SAMPLE_RATE_UNKNOWN);
}

static void
audio_utils_psy_num_audio_samples_to_duration(void)
{
    PsyDuration *dur_sample = NULL;
    PsyDuration *dur_second = NULL;

    // Test whether 1 sample is equal to the number of µs with remainder
    // floored.
    dur_sample
        = psy_num_audio_samples_to_duration(1, PSY_AUDIO_SAMPLE_RATE_44100);
    g_assert_nonnull(dur_sample);
    g_assert_cmpint(psy_duration_get_us(dur_sample),
                    ==,
                    (gint64) (1e6 / PSY_AUDIO_SAMPLE_RATE_44100));

    // Test whether one second worth of samples lasts precisely 1 second.
    dur_second = psy_num_audio_samples_to_duration(
        PSY_AUDIO_SAMPLE_RATE_192000, PSY_AUDIO_SAMPLE_RATE_192000);

    g_assert_nonnull(dur_second);
    g_assert_cmpint(psy_duration_get_us(dur_second), ==, 1000000);

    psy_duration_free(dur_sample);
    psy_duration_free(dur_second);
}

static void
audio_utils_psy_duration_to_num_audio_samples(void)
{
    PsyDuration *one_s = psy_duration_new_s(1);
    PsyDuration *one_sample_dur;

    g_assert_cmpint(
        psy_duration_to_num_audio_frames(one_s, PSY_AUDIO_SAMPLE_RATE_22050),
        ==,
        PSY_AUDIO_SAMPLE_RATE_22050);
    g_assert_cmpint(
        psy_duration_to_num_audio_frames(one_s, PSY_AUDIO_SAMPLE_RATE_44100),
        ==,
        PSY_AUDIO_SAMPLE_RATE_44100);
    g_assert_cmpint(
        psy_duration_to_num_audio_frames(one_s, PSY_AUDIO_SAMPLE_RATE_48000),
        ==,
        PSY_AUDIO_SAMPLE_RATE_48000);
    g_assert_cmpint(
        psy_duration_to_num_audio_frames(one_s, PSY_AUDIO_SAMPLE_RATE_192000),
        ==,
        PSY_AUDIO_SAMPLE_RATE_192000);

    psy_duration_free(one_s);

    one_sample_dur
        = psy_num_audio_samples_to_duration(1, PSY_AUDIO_SAMPLE_RATE_22050);
    g_assert_cmpint(1,
                    ==,
                    psy_duration_to_num_audio_frames(
                        one_sample_dur, PSY_AUDIO_SAMPLE_RATE_22050));
    psy_duration_free(one_sample_dur);

    one_sample_dur
        = psy_num_audio_samples_to_duration(1, PSY_AUDIO_SAMPLE_RATE_44100);
    g_assert_cmpint(1,
                    ==,
                    psy_duration_to_num_audio_frames(
                        one_sample_dur, PSY_AUDIO_SAMPLE_RATE_44100));
    psy_duration_free(one_sample_dur);

    one_sample_dur
        = psy_num_audio_samples_to_duration(1, PSY_AUDIO_SAMPLE_RATE_48000);
    g_assert_cmpint(1,
                    ==,
                    psy_duration_to_num_audio_frames(
                        one_sample_dur, PSY_AUDIO_SAMPLE_RATE_48000));
    psy_duration_free(one_sample_dur);

    one_sample_dur
        = psy_num_audio_samples_to_duration(1, PSY_AUDIO_SAMPLE_RATE_192000);
    g_assert_cmpint(1,
                    ==,
                    psy_duration_to_num_audio_frames(
                        one_sample_dur, PSY_AUDIO_SAMPLE_RATE_192000));
    psy_duration_free(one_sample_dur);
}

int
main(int argc, char **argv)
{
    g_test_init(&argc, &argv, NULL);

    g_test_add_func("/audio-utils/psy-int-to-sample-rate",
                    audio_utils_psy_int_to_sample_rate);
    g_test_add_func("/audio-utils/psy-num-audio-samples-to-duration",
                    audio_utils_psy_num_audio_samples_to_duration);
    g_test_add_func("/audio-utils/psy-duration-to-num-audio-samples",
                    audio_utils_psy_duration_to_num_audio_samples);

    int ret = g_test_run();

    return ret;
}
