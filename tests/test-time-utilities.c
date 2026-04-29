
#include <criterion/criterion.h>
#include <criterion/new/assert.h>

#include <psy-clock.h>
#include <psy-duration.h>

Test(clock, create)
{
    PsyClock     *clock = NULL;
    PsyTimePoint *t1, *t2;
    PsyDuration  *dur = NULL;
    gint64        us;

    clock = psy_clock_new();
    cr_assert(ne(clock, NULL), "Clocks can be created");

    for (int i = 0; i < 9; i++) {
        t1 = psy_clock_now(clock);
        t2 = psy_clock_now(clock);
        cr_expect(eq(psy_time_point_less_equal(t1, t2), TRUE),
                  "Timepoints sampled from the clock only increase");
        psy_time_point_free(t1);
        psy_time_point_free(t2);
    }
    t1 = NULL;
    t2 = NULL;

    t1 = psy_clock_now(clock);
    g_object_get(clock, "now", &t2, NULL);

    cr_assert(ne(t1, NULL));
    cr_assert(ne(t2, NULL));

    dur = psy_time_point_subtract(t2, t1);

    g_assert(dur);
    us = psy_duration_get_us(dur);
    cr_assert(ge(us, 0)); // Negative durations would be weird in this case.

    psy_clock_free(clock);
    psy_time_point_free(t1);
    psy_time_point_free(t2);
    psy_duration_free(dur);
}

Test(time_point, arithmetic)
{
    gint64        us;
    PsyTimePoint *t1     = psy_time_point_new();
    PsyTimePoint *t2     = psy_time_point_new();
    PsyTimePoint *tz     = NULL;
    PsyDuration  *onesec = psy_duration_new_s(1);

    PsyDuration *dur = psy_time_point_subtract(t1, t2);
    us               = psy_duration_get_us(dur);
    cr_expect(eq(us, 0));
    g_clear_pointer(&dur, psy_duration_free);

    dur = psy_time_point_duration_since_start(t1);
    us  = psy_duration_get_us(dur);
    cr_expect(eq(us, 0));

    g_clear_pointer(&dur, psy_duration_free);
    PsyTimePoint *time = psy_time_point_add(t1, onesec);
    dur                = psy_time_point_subtract(time, t1);
    cr_assert(eq(psy_duration_equal(dur, onesec), TRUE));
    tz = psy_time_point_subtract_dur(time, onesec);
    cr_assert(eq(psy_time_point_equal(tz, t1), TRUE));
    g_clear_pointer(&time, psy_time_point_free);
    g_clear_pointer(&dur, psy_duration_free);

    psy_duration_free(onesec);
    psy_time_point_free(t1);
    psy_time_point_free(t2);
    psy_time_point_free(tz);
}

Test(time_point, copy)
{
    PsyClock     *clk = psy_clock_new();
    PsyTimePoint *now = psy_clock_now(clk);
    PsyTimePoint *dup = psy_time_point_copy(now);

    cr_assert(eq(psy_time_point_equal(dup, now), TRUE),
              "A copy should equal the original");

    g_object_unref(clk);
    psy_time_point_free(now);
    psy_time_point_free(dup);
}

Test(time_point, comparisons)
{
    PsyClock     *clock = psy_clock_new();
    PsyTimePoint *t2, *t1 = psy_clock_now(clock);
    g_usleep(1000);
    t2 = psy_clock_now(clock);

    cr_assert(eq(psy_time_point_less(t1, t2), TRUE),
              "t1 should be smaller than t2");
    cr_assert(eq(psy_time_point_less_equal(t1, t2), TRUE), "t1 <= t2");
    cr_assert(eq(psy_time_point_greater(t2, t1), TRUE), "t2 > t1");
    cr_assert(eq(psy_time_point_greater_equal(t2, t1), TRUE), " t2 >= t1");
    cr_assert(eq(psy_time_point_equal(t1, t1), TRUE), "t1 == t1");
    cr_assert(eq(psy_time_point_not_equal(t1, t2), TRUE), "t1 != t2");

    g_object_unref(clock);
    psy_time_point_free(t1);
    psy_time_point_free(t2);
}

Test(duration, check_arithmetics)
{
    PsyDuration *d_us = NULL, *d_ms = NULL, *d_s = NULL, *d_res = NULL,
                *d_temp = NULL, *d_half = NULL, *sub_result = NULL,
                *mul_res = NULL;

    gdouble epsilon = 1e-9;

    d_us = psy_duration_new_us(5);
    d_ms = psy_duration_new_ms(5);
    d_s  = psy_duration_new_s(5);

    cr_assert(d_us && d_ms && d_s, "Durations can be created");
    gint64  us, ms, s;
    gdouble seconds;

    d_temp = psy_duration_add(d_us, d_ms);
    cr_assert(d_temp != NULL);
    d_res = psy_duration_add(d_temp, d_s);
    cr_assert(d_res != NULL);

    us      = psy_duration_get_us(d_res);
    ms      = psy_duration_get_ms(d_res);
    s       = psy_duration_get_s(d_res);
    seconds = psy_duration_get_seconds(d_res);

    cr_expect(eq(us, 5 + 5 * 1000 + 5 * 1000000),
              "5s + 5ms + 5µs == 5005005 µs");
    cr_expect(eq(ms, 5 + 5 * 1000), "5s + 5ms + 5µs == 5005 ms");
    cr_expect(eq(s, 5), "5s + 5ms + 5µs == 5s");
    cr_expect(epsilon_eq(seconds, 5.005005, epsilon),
              "5s + 5ms + 5µs ~ 5.005005s");

    d_half = psy_duration_divide_scalar(d_s, 2);
    cr_assert(eq(psy_duration_get_ms(d_half), 2500), "5s / 2 = 2500ms");
    cr_assert(eq(psy_duration_get_us(d_half), 2500 * 1000),
              "5s / 2 = 2500000µs");
    cr_assert(eq(psy_duration_divide(d_s, d_half), 2), "5.0s / 2.5s = 2");

    sub_result = psy_duration_subtract(d_s, d_half);
    cr_assert(ne(sub_result, NULL), "It's possible to subtract durations");

    cr_assert(eq(psy_duration_equal(sub_result, d_half), TRUE),
              "5.0 / 2 = 2.5s");

    mul_res = psy_duration_multiply_scalar(d_half, 2);
    cr_assert(ne(mul_res, NULL));
    cr_expect(eq(psy_duration_equal(mul_res, d_s), TRUE));

    psy_duration_free(d_us);
    psy_duration_free(d_ms);
    psy_duration_free(d_s);
    psy_duration_free(d_res);
    psy_duration_free(d_temp);
    psy_duration_free(d_half);
    psy_duration_free(sub_result);
    psy_duration_free(mul_res);
}

Test(duration, rounded_division)
{
    PsyDuration *ten   = psy_duration_new(10);
    PsyDuration *eight = psy_duration_new(8);
    PsyDuration *six   = psy_duration_new(6);

    gint64 r1 = psy_duration_divide_rounded(ten, eight);
    gint64 r2 = psy_duration_divide_rounded(ten, six);

    cr_assert(eq(r1, 1)); // 10 / 8 = 1.25
    cr_assert(eq(r2, 2)); // 10 / 6 = 1.6667

    psy_duration_free(ten);
    psy_duration_free(eight);
    psy_duration_free(six);
}

Test(duration, comparisons)
{
    PsyDuration *one_s;
    PsyDuration *alittlemore;
    PsyDuration *alittleless;

    one_s       = psy_duration_new(1.0);
    alittleless = psy_duration_new_us(999999);
    alittlemore = psy_duration_new_us(999999 + 2);

    cr_expect(eq(psy_duration_not_equal(one_s, alittleless), TRUE));
    cr_expect(eq(psy_duration_not_equal(one_s, alittlemore), TRUE));

    cr_expect(eq(psy_duration_equal(one_s, one_s), TRUE));
    cr_expect(eq(psy_duration_less_equal(one_s, one_s), TRUE));
    cr_expect(eq(psy_duration_greater_equal(one_s, one_s), TRUE));

    cr_expect(eq(psy_duration_less(alittleless, one_s), TRUE));
    cr_expect(eq(psy_duration_less_equal(alittleless, one_s), TRUE));
    cr_expect(eq(psy_duration_greater_equal(alittleless, one_s), FALSE));
    cr_expect(eq(psy_duration_greater(alittleless, one_s), FALSE));

    cr_expect(eq(psy_duration_greater(alittlemore, one_s), TRUE));
    cr_expect(eq(psy_duration_greater_equal(alittlemore, one_s), TRUE));
    cr_expect(eq(psy_duration_less(alittlemore, one_s), FALSE));
    cr_expect(eq(psy_duration_less_equal(alittlemore, one_s), FALSE));

    psy_duration_free(one_s);
    psy_duration_free(alittlemore);
    psy_duration_free(alittleless);
}

Test(clock, zero_time)
{
    PsyClock *clock = psy_clock_new();

    gint64 zero_time = psy_clock_get_zero_time();
    cr_assert(ge(zero_time, 0));

    psy_clock_free(clock);
}
