
#include "psy-duration.h"
#include <psy-clock.h>

static void
test_clock(void)
{
    PsyClock* clock = NULL;
    PsyTimePoint *t1, *t2;
    PsyDuration *dur;
    gint64 us;

    clock = psy_clock_new();
    g_assert(clock);

    for (int i = 0; i < 9; i++) {
        t1 = psy_clock_now(clock);
        t2 = psy_clock_now(clock);
        g_object_unref(t1);
        g_object_unref(t2);
    }

    t1 = psy_clock_now(clock);
    t2 = psy_clock_now(clock);

    g_assert(t1);
    g_assert(t2);

    dur = psy_time_point_subtract(t2, t1);

    g_assert(dur);
    g_object_get(dur,
                 "us", &us,
                 NULL);
    g_assert(us >= 0); // Negative durations would be weird in this case.
    g_object_unref(clock);
    g_object_unref(t1);
    g_object_unref(t2);
    g_object_unref(dur);
}

static void
check_time_point_arithmetic(void)
{
    gint64 us;
    PsyTimePoint *t1 = g_object_new(PSY_TYPE_TIME_POINT, NULL);
    PsyTimePoint *t2 = g_object_new(PSY_TYPE_TIME_POINT, NULL);
    PsyTimePoint *tz = NULL;
    PsyDuration  *onesec = psy_duration_new_s(1);

    PsyDuration *dur = psy_time_point_subtract(t1, t2);
    g_object_get(dur, "us", &us, NULL);
    g_assert(us == 0);
    g_clear_object(&dur);

    dur = psy_time_point_duration_since_start(t1);
    g_object_get(dur, "us", &us, NULL);
    g_assert(us == 0);

    g_clear_object(&dur);
    PsyTimePoint *time = psy_time_point_add(t1, onesec);
    dur = psy_time_point_subtract(time, t1);
    g_assert(psy_duration_equal(dur, onesec));
    tz = psy_time_point_subtract_dur(time, onesec);
    g_assert(psy_time_point_equal(tz, t1));
    g_clear_object(&time);
    g_clear_object(&dur);

    g_object_unref(onesec);
    g_object_unref(t1);
    g_object_unref(t2);
    g_object_unref(tz);
}

static void
check_time_point_comparisons(void) {
    PsyClock *clock = psy_clock_new();
    PsyTimePoint *t2, *t1 = psy_clock_now(clock);
    g_usleep(1000);
    t2 = psy_clock_now(clock);
    g_assert(psy_time_point_less(t1, t2));
    g_assert(psy_time_point_less_equal(t1, t2));
    g_assert(psy_time_point_greater(t2, t1));
    g_assert(psy_time_point_greater_equal(t2, t1));
    g_assert(psy_time_point_equal(t1, t1));
    g_assert(psy_time_point_not_equal(t1, t2));

    g_object_unref(clock);
    g_object_unref(t1);
    g_object_unref(t2);
}

static void
check_time_point_overflow()
{
    PsyTimePoint *t1, *t2, *toverflow;
    PsyDuration *dur_max    = psy_duration_new_us(G_MAXINT64);
    PsyDuration *one_us     = psy_duration_new_us(1);
    PsyDuration *two_us     = psy_duration_new_us(2);
    PsyDuration *n_two_us   = psy_duration_new_us(-2);
    t1 = g_object_new(PSY_TYPE_TIME_POINT, NULL);

    t2 = psy_time_point_add(t1, dur_max);
    g_assert(t2);
    toverflow = psy_time_point_add(t2, one_us);
    g_assert(!toverflow);
    toverflow = psy_time_point_subtract_dur(t2, n_two_us);
    g_assert(!toverflow);

    g_clear_object(&t2);
    t2 = psy_time_point_subtract_dur(t1, dur_max);
    g_assert(t2);
    toverflow = psy_time_point_subtract_dur(t2, two_us);
    g_assert(!toverflow);
    toverflow = psy_time_point_add(t2, n_two_us);
    g_assert(!toverflow);

    g_object_unref(t1);
    g_object_unref(t2);
    g_object_unref(dur_max);
    g_object_unref(one_us);
    g_object_unref(two_us);
    g_object_unref(n_two_us);
}

static void
check_duration_arithmetics(void)
{
    PsyDuration *d_us, *d_ms, *d_s, *d_res, *d_temp, *d_half, *sub_result, *mul_res;

    d_us = psy_duration_new_us(5);
    d_ms = psy_duration_new_ms(5);
    d_s  = psy_duration_new_s(5);

    g_assert (d_us && d_ms && d_s);
    gint64 us, ms, s;
    gdouble seconds;

    d_temp = psy_duration_add(d_us, d_ms);
    g_assert(d_temp);
    d_res = psy_duration_add(d_temp, d_s);
    g_assert(d_res);

    g_object_get(d_res,
                 "us", &us,
                 "ms", &ms,
                 "s", &s,
                 "seconds", &seconds,
                 NULL);

    g_assert(us == 5 + 5 * 1000 + 5 * 1000000);
    g_assert(ms == 5 + 5 * 1000);
    g_assert(s == 5);
    g_assert(seconds == 5.005005);

    d_half = psy_duration_divide_scalar(d_s, 2);
    g_assert(psy_duration_get_ms(d_half) == 2500 &&
             psy_duration_get_us(d_half) == 2500 * 1000
    );
    g_assert(psy_duration_divide(d_s, d_half) == 2);

    sub_result = psy_duration_subtract(d_s, d_half);
    g_assert(sub_result);
    g_assert(psy_duration_equal(sub_result, d_half));

    mul_res = psy_duration_multiply_scalar(d_half, 2);
    g_assert(mul_res);
    g_assert(psy_duration_equal(mul_res, d_s));

    g_object_unref(d_us);
    g_object_unref(d_ms);
    g_object_unref(d_s);
    g_object_unref(d_res);
    g_object_unref(d_temp);
    g_object_unref(d_half);
    g_object_unref(sub_result);
    g_object_unref(mul_res);
}

static void
check_duration_rounded_division(void)
{
    PsyDuration* ten =  psy_duration_new(10);
    PsyDuration* eight =  psy_duration_new(8);
    PsyDuration* six =  psy_duration_new(6);

    gint64 r1 = psy_duration_divide_rounded(ten, eight);
    gint64 r2 = psy_duration_divide_rounded(ten, six);

    g_assert_true(r1 == 1); // 10 / 8 = 1.25
    g_assert_true(r2 == 2); // 10 / 6 = 1.6667

    g_object_unref(ten);
    g_object_unref(eight);
    g_object_unref(six);
}


static void
check_duration_comparisons(void)
{
    PsyDuration *one_s;
    PsyDuration *alittlemore;
    PsyDuration *alittleless;

    one_s = psy_duration_new(1.0);
    alittleless = psy_duration_new_us(999999);
    alittlemore = psy_duration_new_us(999999+2);

    g_assert(psy_duration_not_equal(one_s, alittleless));
    g_assert(psy_duration_not_equal(one_s, alittlemore));

    g_assert(psy_duration_equal(one_s, one_s));
    g_assert(psy_duration_less_equal(one_s, one_s));
    g_assert(psy_duration_greater_equal(one_s, one_s));

    g_assert(psy_duration_less(alittleless, one_s));
    g_assert(psy_duration_less_equal(alittleless, one_s));
    g_assert(!psy_duration_greater_equal(alittleless, one_s));
    g_assert(!psy_duration_greater(alittleless, one_s));

    g_assert(psy_duration_greater(alittlemore, one_s));
    g_assert(psy_duration_greater_equal(alittlemore, one_s));
    g_assert(!psy_duration_less(alittlemore, one_s));
    g_assert(!psy_duration_less_equal(alittlemore, one_s));

}

int
main(int argc, char **argv)
{
    (void) argc, (void) argv;
    test_clock();

    check_time_point_arithmetic();
    check_time_point_comparisons();
    check_time_point_overflow();

    check_duration_arithmetics();
    check_duration_rounded_division();
    check_duration_comparisons();

    return EXIT_SUCCESS;
}


