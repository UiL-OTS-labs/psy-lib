

#include <CUnit/CUnit.h>
#include <psylib.h>

#include "unit-test-utilities.h"

static void
test_psy_int_to_sample_rate(void)
{
    CU_ASSERT_EQUAL(psy_int_to_sample_rate(PSY_AUDIO_SAMPLE_RATE_22050),
                    PSY_AUDIO_SAMPLE_RATE_22050);
    CU_ASSERT_EQUAL(psy_int_to_sample_rate(PSY_AUDIO_SAMPLE_RATE_24000),
                    PSY_AUDIO_SAMPLE_RATE_24000);
    CU_ASSERT_EQUAL(psy_int_to_sample_rate(PSY_AUDIO_SAMPLE_RATE_32000),
                    PSY_AUDIO_SAMPLE_RATE_32000);
    CU_ASSERT_EQUAL(psy_int_to_sample_rate(PSY_AUDIO_SAMPLE_RATE_44100),
                    PSY_AUDIO_SAMPLE_RATE_44100);
    CU_ASSERT_EQUAL(psy_int_to_sample_rate(PSY_AUDIO_SAMPLE_RATE_48000),
                    PSY_AUDIO_SAMPLE_RATE_48000);
    CU_ASSERT_EQUAL(psy_int_to_sample_rate(PSY_AUDIO_SAMPLE_RATE_88200),
                    PSY_AUDIO_SAMPLE_RATE_88200);
    CU_ASSERT_EQUAL(psy_int_to_sample_rate(PSY_AUDIO_SAMPLE_RATE_96000),
                    PSY_AUDIO_SAMPLE_RATE_96000);
    CU_ASSERT_EQUAL(psy_int_to_sample_rate(PSY_AUDIO_SAMPLE_RATE_192000),
                    PSY_AUDIO_SAMPLE_RATE_192000);

    // Could fail very occasionally
    CU_ASSERT_EQUAL(psy_int_to_sample_rate(random_int_range(0, G_MAXINT)),
                    PSY_AUDIO_SAMPLE_RATE_UNKNOWN);
}

static void
test_psy_num_audio_samples_to_duration(void)
{
    PsyDuration *dur_sample = NULL;
    PsyDuration *dur_second = NULL;

    // Test whether 1 sample is equal to the number of µs with remainder
    // floored.
    dur_sample
        = psy_num_audio_samples_to_duration(1, PSY_AUDIO_SAMPLE_RATE_44100);
    CU_ASSERT_PTR_NOT_NULL_FATAL(dur_sample);
    CU_ASSERT_EQUAL(psy_duration_get_us(dur_sample),
                    (gint64) (1e6 / PSY_AUDIO_SAMPLE_RATE_44100));

    // Test whether one second worth of samples lasts precisely 1 second.
    dur_second = psy_num_audio_samples_to_duration(
        PSY_AUDIO_SAMPLE_RATE_192000, PSY_AUDIO_SAMPLE_RATE_192000);

    CU_ASSERT_PTR_NOT_NULL_FATAL(dur_second);
    CU_ASSERT_EQUAL(psy_duration_get_us(dur_second), 1000000);

    psy_duration_free(dur_sample);
    psy_duration_free(dur_second);
}

static void
test_psy_duration_to_num_audio_samples(void)
{
    PsyDuration *one_s = psy_duration_new_s(1);
    PsyDuration *one_sample_dur;

    CU_ASSERT_EQUAL(
        psy_duration_to_num_audio_frames(one_s, PSY_AUDIO_SAMPLE_RATE_22050),
        PSY_AUDIO_SAMPLE_RATE_22050);
    CU_ASSERT_EQUAL(
        psy_duration_to_num_audio_frames(one_s, PSY_AUDIO_SAMPLE_RATE_44100),
        PSY_AUDIO_SAMPLE_RATE_44100);
    CU_ASSERT_EQUAL(
        psy_duration_to_num_audio_frames(one_s, PSY_AUDIO_SAMPLE_RATE_48000),
        PSY_AUDIO_SAMPLE_RATE_48000);
    CU_ASSERT_EQUAL(
        psy_duration_to_num_audio_frames(one_s, PSY_AUDIO_SAMPLE_RATE_192000),
        PSY_AUDIO_SAMPLE_RATE_192000);

    psy_duration_free(one_s);

    one_sample_dur
        = psy_num_audio_samples_to_duration(1, PSY_AUDIO_SAMPLE_RATE_22050);
    CU_ASSERT_EQUAL(1,
                    psy_duration_to_num_audio_frames(
                        one_sample_dur, PSY_AUDIO_SAMPLE_RATE_22050));
    psy_duration_free(one_sample_dur);

    one_sample_dur
        = psy_num_audio_samples_to_duration(1, PSY_AUDIO_SAMPLE_RATE_44100);
    CU_ASSERT_EQUAL(1,
                    psy_duration_to_num_audio_frames(
                        one_sample_dur, PSY_AUDIO_SAMPLE_RATE_44100));
    psy_duration_free(one_sample_dur);

    one_sample_dur
        = psy_num_audio_samples_to_duration(1, PSY_AUDIO_SAMPLE_RATE_48000);
    CU_ASSERT_EQUAL(1,
                    psy_duration_to_num_audio_frames(
                        one_sample_dur, PSY_AUDIO_SAMPLE_RATE_48000));
    psy_duration_free(one_sample_dur);

    one_sample_dur
        = psy_num_audio_samples_to_duration(1, PSY_AUDIO_SAMPLE_RATE_192000);
    CU_ASSERT_EQUAL(1,
                    psy_duration_to_num_audio_frames(
                        one_sample_dur, PSY_AUDIO_SAMPLE_RATE_192000));
    psy_duration_free(one_sample_dur);
}

int
add_audio_utils_suite(void)
{
    CU_Suite *suite = CU_add_suite("audio utils tests", NULL, NULL);
    CU_Test  *test  = NULL;

    if (!suite)
        return 1;

    test = CU_ADD_TEST(suite, test_psy_int_to_sample_rate);
    if (!test)
        return 1;

    test = CU_ADD_TEST(suite, test_psy_num_audio_samples_to_duration);
    if (!test)
        return 1;

    test = CU_ADD_TEST(suite, test_psy_duration_to_num_audio_samples);
    if (!test)
        return 1;

    return 0;
}
