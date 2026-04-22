
#include <criterion/criterion.h>
#include <criterion/new/assert.h>
#include <psylib.h>

#include "unit-test-utilities.h"

Test(audio_utils, psy_int_to_sample_rate)
{
    cr_expect(eq(psy_int_to_sample_rate(PSY_AUDIO_SAMPLE_RATE_22050),
                 PSY_AUDIO_SAMPLE_RATE_22050));
    cr_expect(eq(psy_int_to_sample_rate(PSY_AUDIO_SAMPLE_RATE_24000),
                 PSY_AUDIO_SAMPLE_RATE_24000));
    cr_expect(eq(psy_int_to_sample_rate(PSY_AUDIO_SAMPLE_RATE_32000),
                 PSY_AUDIO_SAMPLE_RATE_32000));
    cr_expect(eq(psy_int_to_sample_rate(PSY_AUDIO_SAMPLE_RATE_44100),
                 PSY_AUDIO_SAMPLE_RATE_44100));
    cr_expect(eq(psy_int_to_sample_rate(PSY_AUDIO_SAMPLE_RATE_48000),
                 PSY_AUDIO_SAMPLE_RATE_48000));
    cr_expect(eq(psy_int_to_sample_rate(PSY_AUDIO_SAMPLE_RATE_88200),
                 PSY_AUDIO_SAMPLE_RATE_88200));
    cr_expect(eq(psy_int_to_sample_rate(PSY_AUDIO_SAMPLE_RATE_96000),
                 PSY_AUDIO_SAMPLE_RATE_96000));
    cr_expect(eq(psy_int_to_sample_rate(PSY_AUDIO_SAMPLE_RATE_192000),
                 PSY_AUDIO_SAMPLE_RATE_192000));

    // Could fail very occasionally
    cr_expect(eq(psy_int_to_sample_rate(random_int_range(0, G_MAXINT)),
                 PSY_AUDIO_SAMPLE_RATE_UNKNOWN));
}

Test(audio_utils, psy_num_audio_samples_to_duration)
{
    PsyDuration *dur_sample = NULL;
    PsyDuration *dur_second = NULL;

    // Test whether 1 sample is equal to the number of µs with remainder
    // floored.
    dur_sample
        = psy_num_audio_samples_to_duration(1, PSY_AUDIO_SAMPLE_RATE_44100);
    cr_assert(ne(dur_sample, NULL));
    cr_expect(eq(psy_duration_get_us(dur_sample),
                 (gint64) (1e6 / PSY_AUDIO_SAMPLE_RATE_44100)));

    // Test whether one second worth of samples lasts precisely 1 second.
    dur_second = psy_num_audio_samples_to_duration(
        PSY_AUDIO_SAMPLE_RATE_192000, PSY_AUDIO_SAMPLE_RATE_192000);

    cr_assert(ne(dur_second, NULL));
    cr_assert(eq(psy_duration_get_us(dur_second), 1000000));

    psy_duration_free(dur_sample);
    psy_duration_free(dur_second);
}

Test(audio_utils, psy_duration_to_num_audio_samples)
{
    PsyDuration *one_s = psy_duration_new_s(1);
    PsyDuration *one_sample_dur;

    cr_assert(
        eq(psy_duration_to_num_audio_frames(one_s, PSY_AUDIO_SAMPLE_RATE_22050),
           PSY_AUDIO_SAMPLE_RATE_22050));
    cr_assert(
        eq(psy_duration_to_num_audio_frames(one_s, PSY_AUDIO_SAMPLE_RATE_44100),
           PSY_AUDIO_SAMPLE_RATE_44100));
    cr_assert(
        eq(psy_duration_to_num_audio_frames(one_s, PSY_AUDIO_SAMPLE_RATE_48000),
           PSY_AUDIO_SAMPLE_RATE_48000));
    cr_assert(eq(
        psy_duration_to_num_audio_frames(one_s, PSY_AUDIO_SAMPLE_RATE_192000),
        PSY_AUDIO_SAMPLE_RATE_192000));

    psy_duration_free(one_s);

    one_sample_dur
        = psy_num_audio_samples_to_duration(1, PSY_AUDIO_SAMPLE_RATE_22050);
    cr_assert(eq(1,
                 psy_duration_to_num_audio_frames(
                     one_sample_dur, PSY_AUDIO_SAMPLE_RATE_22050)));
    psy_duration_free(one_sample_dur);

    one_sample_dur
        = psy_num_audio_samples_to_duration(1, PSY_AUDIO_SAMPLE_RATE_44100);
    cr_assert(eq(1,
                 psy_duration_to_num_audio_frames(
                     one_sample_dur, PSY_AUDIO_SAMPLE_RATE_44100)));
    psy_duration_free(one_sample_dur);

    one_sample_dur
        = psy_num_audio_samples_to_duration(1, PSY_AUDIO_SAMPLE_RATE_48000);
    cr_assert(eq(1,
                 psy_duration_to_num_audio_frames(
                     one_sample_dur, PSY_AUDIO_SAMPLE_RATE_48000)));
    psy_duration_free(one_sample_dur);

    one_sample_dur
        = psy_num_audio_samples_to_duration(1, PSY_AUDIO_SAMPLE_RATE_192000);
    cr_assert(eq(1,
                 psy_duration_to_num_audio_frames(
                     one_sample_dur, PSY_AUDIO_SAMPLE_RATE_192000)));
    psy_duration_free(one_sample_dur);
}
