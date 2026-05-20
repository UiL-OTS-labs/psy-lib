
#include <psylib.h>

static void
test_clock_create(void)
{
    PsyClock     *clock = NULL;
    PsyTimePoint *t1, *t2;
    PsyDuration  *dur = NULL;
    gint64        us;

    clock = psy_clock_new();
    g_assert_nonnull(clock); // Clocks can be created

    for (int i = 0; i < 9; i++) {
        t1 = psy_clock_now(clock);
        t2 = psy_clock_now(clock);
        // Timepoints sampled from the clock only increase
        g_assert_true(psy_time_point_less_equal(t1, t2));
        psy_time_point_free(t1);
        psy_time_point_free(t2);
    }
    t1 = NULL;
    t2 = NULL;

    t1 = psy_clock_now(clock);
    g_object_get(clock, "now", &t2, NULL);

    g_assert_nonnull(t1);
    g_assert_nonnull(t2);

    dur = psy_time_point_subtract(t2, t1);

    g_assert_nonnull(dur);
    us = psy_duration_get_us(dur);

    // Negative durations would be weird in this case.
    g_assert_cmpint(us, >=, 0);

    psy_clock_free(clock);
    psy_time_point_free(t1);
    psy_time_point_free(t2);
    psy_duration_free(dur);
}

static void
test_time_point_arithmetic(void)
{
    gint64        us;
    PsyTimePoint *t1     = psy_time_point_new();
    PsyTimePoint *t2     = psy_time_point_new();
    PsyTimePoint *tz     = NULL;
    PsyDuration  *onesec = psy_duration_new_s(1);

    PsyDuration *dur = psy_time_point_subtract(t1, t2);
    us               = psy_duration_get_us(dur);
    g_assert_cmpint(us, ==, 0);
    g_clear_pointer(&dur, psy_duration_free);

    dur = psy_time_point_duration_since_start(t1);
    us  = psy_duration_get_us(dur);
    g_assert_cmpint(us, ==, 0);
    g_clear_pointer(&dur, psy_duration_free);

    PsyTimePoint *time = psy_time_point_add(t1, onesec);
    dur                = psy_time_point_subtract(time, t1);
    g_assert_true(psy_duration_equal(dur, onesec));
    tz = psy_time_point_subtract_dur(time, onesec);
    g_assert_true(psy_time_point_equal(tz, t1));
    g_clear_pointer(&time, psy_time_point_free);
    g_clear_pointer(&dur, psy_duration_free);

    psy_duration_free(onesec);
    psy_time_point_free(t1);
    psy_time_point_free(t2);
    psy_time_point_free(tz);
}

static void
test_time_point_copy(void)
{
    PsyClock     *clk = psy_clock_new();
    PsyTimePoint *now = psy_clock_now(clk);
    PsyTimePoint *dup = psy_time_point_copy(now);

    // A copy should equal the original
    g_assert_true(psy_time_point_equal(dup, now));

    // It should be a deep copy
    g_assert_cmphex((uintptr_t) now, !=, (uintptr_t) dup);

    g_object_unref(clk);
    psy_time_point_free(now);
    psy_time_point_free(dup);
}

static void
test_time_point_comparisons(void)
{
    PsyClock     *clock = psy_clock_new();
    PsyTimePoint *t2, *t1 = psy_clock_now(clock);
    g_usleep(1000);
    t2 = psy_clock_now(clock);

    g_assert_true(psy_time_point_less(t1, t2)); // t1 should be smaller than t2
    g_assert_true(psy_time_point_less_equal(t1, t2));    // t1 <= t2
    g_assert_true(psy_time_point_greater(t2, t1));       // t2 > t1
    g_assert_true(psy_time_point_greater_equal(t2, t1)); // t2 >= t1
    g_assert_true(psy_time_point_equal(t1, t1));         // t1 == t1
    g_assert_true(psy_time_point_not_equal(t1, t2));     // t1 != t2

    g_object_unref(clock);
    psy_time_point_free(t1);
    psy_time_point_free(t2);
}

static void
test_duration_check_arithmetics(void)
{
    PsyDuration *d_us = NULL, *d_ms = NULL, *d_s = NULL, *d_res = NULL,
                *d_temp = NULL, *d_half = NULL, *sub_result = NULL,
                *mul_res = NULL;

    gdouble epsilon = 1e-9;

    d_us = psy_duration_new_us(5);
    d_ms = psy_duration_new_ms(5);
    d_s  = psy_duration_new_s(5);

    g_assert_true(d_us && d_ms && d_s); // Durations can be created
    gint64  us, ms, s;
    gdouble seconds;

    d_temp = psy_duration_add(d_us, d_ms);
    g_assert_nonnull(d_temp);
    d_res = psy_duration_add(d_temp, d_s);
    g_assert_nonnull(d_res);

    us      = psy_duration_get_us(d_res);
    ms      = psy_duration_get_ms(d_res);
    s       = psy_duration_get_s(d_res);
    seconds = psy_duration_get_seconds(d_res);

    // 5s + 5ms + 5µs == 5005005 µs
    g_assert_cmpint(us, ==, 5 + 5 * 1000 + 5 * 1000000);
    g_assert_cmpint(ms, ==, 5 + 5 * 1000); // 5s + 5ms + 5µs == 5005 ms
    g_assert_cmpint(s, ==, 5);             // "5s + 5ms + 5µs == 5s"
    // 5s + 5ms + 5µs ~ 5.005005s
    g_assert_cmpfloat_with_epsilon(seconds, 5.005005, epsilon);

    d_half = psy_duration_divide_scalar(d_s, 2);
    g_assert_cmpint(
        psy_duration_get_ms(d_half), ==, 2500); // "5s / 2 = 2500ms";
    g_assert_cmpint(
        psy_duration_get_us(d_half), ==, 2500lu * 1000); // 5s / 2 = 2500000µs
    g_assert_cmpint(psy_duration_divide(d_s, d_half), ==, 2); // 5.0s / 2.5s = 2

    sub_result = psy_duration_subtract(d_s, d_half);
    g_assert_nonnull(sub_result); // It's possible to subtract durations

    g_assert_true(psy_duration_equal(sub_result, d_half)); // 5.0 / 2 = 2.5s

    mul_res = psy_duration_multiply_scalar(d_half, 2);
    g_assert_nonnull(mul_res);
    g_assert_true(psy_duration_equal(mul_res, d_s));

    psy_duration_free(d_us);
    psy_duration_free(d_ms);
    psy_duration_free(d_s);
    psy_duration_free(d_res);
    psy_duration_free(d_temp);
    psy_duration_free(d_half);
    psy_duration_free(sub_result);
    psy_duration_free(mul_res);
}

static void
test_duration_rounded_division(void)
{
    PsyDuration *ten   = psy_duration_new(10);
    PsyDuration *eight = psy_duration_new(8);
    PsyDuration *six   = psy_duration_new(6);

    gint64 r1 = psy_duration_divide_rounded(ten, eight);
    gint64 r2 = psy_duration_divide_rounded(ten, six);

    g_assert_cmpint(r1, ==, 1); // 10 / 8 = 1.25
    g_assert_cmpint(r2, ==, 2); // 10 / 6 = 1.6667

    psy_duration_free(ten);
    psy_duration_free(eight);
    psy_duration_free(six);
}

static void
test_duration_comparisons(void)
{
    PsyDuration *one_s;
    PsyDuration *alittlemore;
    PsyDuration *alittleless;

    one_s       = psy_duration_new(1.0);
    alittleless = psy_duration_new_us(999999);
    alittlemore = psy_duration_new_us(999999 + 2);

    g_assert_true(psy_duration_not_equal(one_s, alittleless));
    g_assert_true(psy_duration_not_equal(one_s, alittlemore));

    g_assert_true(psy_duration_equal(one_s, one_s));
    g_assert_true(psy_duration_less_equal(one_s, one_s));
    g_assert_true(psy_duration_greater_equal(one_s, one_s));

    g_assert_true(psy_duration_less(alittleless, one_s));
    g_assert_true(psy_duration_less_equal(alittleless, one_s));
    g_assert_false(psy_duration_greater_equal(alittleless, one_s));
    g_assert_false(psy_duration_greater(alittleless, one_s));

    g_assert_true(psy_duration_greater(alittlemore, one_s));
    g_assert_true(psy_duration_greater_equal(alittlemore, one_s));
    g_assert_false(psy_duration_less(alittlemore, one_s));
    g_assert_false(psy_duration_less_equal(alittlemore, one_s));

    psy_duration_free(one_s);
    psy_duration_free(alittlemore);
    psy_duration_free(alittleless);
}

static void
test_clock_zero_time(void)
{
    PsyClock *clock = psy_clock_new();

    gint64 zero_time = psy_clock_get_zero_time();
    g_assert_cmpint(zero_time, >, 0);

    psy_clock_free(clock);
}

int
main(int argc, char **argv)
{
    g_test_init(&argc, &argv, NULL);

    g_test_add_func("/clock/create", test_clock_create);
    g_test_add_func("/clock/zero_time", test_clock_zero_time);
    g_test_add_func("/time_point/arithmetics", test_time_point_arithmetic);
    g_test_add_func("/time_point/comparisons", test_time_point_comparisons);
    g_test_add_func("/time_point/copy", test_time_point_copy);
    g_test_add_func("/duration/arithmetics", test_duration_check_arithmetics);
    g_test_add_func("/duration/rounded_division",
                    test_duration_rounded_division);
    g_test_add_func("/duration/comparisons", test_duration_comparisons);

    return g_test_run();
}
