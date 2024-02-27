
#include <CUnit/CUnit.h>

#include <psy-clock.h>
#include <psy-duration.h>

static void
test_clock(void)
{
    PsyClock     *clock = NULL;
    PsyTimePoint *t1, *t2;
    PsyDuration  *dur = NULL;
    gint64        us;

    clock = psy_clock_new();
    CU_ASSERT_PTR_NOT_NULL_FATAL(clock);

    for (int i = 0; i < 9; i++) {
        t1 = psy_clock_now(clock);
        t2 = psy_clock_now(clock);
        CU_ASSERT_TRUE(psy_time_point_less_equal(t1, t2));
        psy_time_point_free(t1);
        psy_time_point_free(t2);
    }
    t1 = NULL;
    t2 = NULL;

    t1 = psy_clock_now(clock);
    g_object_get(clock, "now", &t2, NULL);

    CU_ASSERT_PTR_NOT_NULL_FATAL(t1);
    CU_ASSERT_PTR_NOT_NULL_FATAL(t2);

    if (!t1 || !t2)
        goto failure;

    dur = psy_time_point_subtract(t2, t1);

    g_assert(dur);
    us = psy_duration_get_us(dur);
    CU_ASSERT_TRUE(us >= 0); // Negative durations would be weird in this case.
failure:
    psy_clock_free(clock);
    psy_time_point_free(t1);
    psy_time_point_free(t2);
    psy_duration_free(dur);
}

static void
check_time_point_arithmetic(void)
{
    gint64        us;
    PsyTimePoint *t1     = psy_time_point_new();
    PsyTimePoint *t2     = psy_time_point_new();
    PsyTimePoint *tz     = NULL;
    PsyDuration  *onesec = psy_duration_new_s(1);

    PsyDuration *dur = psy_time_point_subtract(t1, t2);
    us               = psy_duration_get_us(dur);
    CU_ASSERT_EQUAL(us, 0);
    g_clear_pointer(&dur, psy_duration_free);

    dur = psy_time_point_duration_since_start(t1);
    us  = psy_duration_get_us(dur);
    CU_ASSERT_EQUAL(us, 0);

    g_clear_pointer(&dur, psy_duration_free);
    PsyTimePoint *time = psy_time_point_add(t1, onesec);
    dur                = psy_time_point_subtract(time, t1);
    CU_ASSERT_TRUE(psy_duration_equal(dur, onesec));
    tz = psy_time_point_subtract_dur(time, onesec);
    CU_ASSERT_TRUE(psy_time_point_equal(tz, t1));
    g_clear_pointer(&time, psy_time_point_free);
    g_clear_pointer(&dur, psy_duration_free);

    psy_duration_free(onesec);
    psy_time_point_free(t1);
    psy_time_point_free(t2);
    psy_time_point_free(tz);
}

static void
test_psy_time_point_copy(void)
{
    PsyClock     *clk = psy_clock_new();
    PsyTimePoint *now = psy_clock_now(clk);
    PsyTimePoint *dup = psy_time_point_copy(now);

    CU_ASSERT_TRUE(psy_time_point_equal(dup, now));

    g_object_unref(clk);
    psy_time_point_free(now);
    psy_time_point_free(dup);
}

static void
check_time_point_comparisons(void)
{
    PsyClock     *clock = psy_clock_new();
    PsyTimePoint *t2, *t1 = psy_clock_now(clock);
    g_usleep(1000);
    t2 = psy_clock_now(clock);
    CU_ASSERT_TRUE(psy_time_point_less(t1, t2));
    CU_ASSERT_TRUE(psy_time_point_less_equal(t1, t2));
    CU_ASSERT_TRUE(psy_time_point_greater(t2, t1));
    CU_ASSERT_TRUE(psy_time_point_greater_equal(t2, t1));
    CU_ASSERT_TRUE(psy_time_point_equal(t1, t1));
    CU_ASSERT_TRUE(psy_time_point_not_equal(t1, t2));

    g_object_unref(clock);
    psy_time_point_free(t1);
    psy_time_point_free(t2);
}

// static void
// check_time_point_overflow(void)
// {
//     PsyTimePoint *t1, *t2, *toverflow;
//     PsyDuration  *dur_max  = psy_duration_new_us(G_MAXINT64);
//     PsyDuration  *one_us   = psy_duration_new_us(1);
//     PsyDuration  *two_us   = psy_duration_new_us(2);
//     PsyDuration  *n_two_us = psy_duration_new_us(-2);
//     t1                     = g_object_new(PSY_TYPE_TIME_POINT, NULL);
//
//     t2 = psy_time_point_add(t1, dur_max); // highest possible valid time
//     point CU_ASSERT_PTR_NOT_NULL(t2); toverflow = psy_time_point_add(t2,
//     one_us); // Adding should overflow CU_ASSERT_PTR_NULL(toverflow);
//     toverflow = psy_time_point_subtract_dur(
//         t2, n_two_us); // subtracting a neg dur too.
//     CU_ASSERT_PTR_NULL(toverflow);
//
//     // subtracting a postive dur from smallest time point should overflow
//     // adding a negative too.
//     g_clear_object(&t2);
//     t2 = psy_time_point_subtract_dur(t1, dur_max); // lowest possible time
//     point CU_ASSERT_PTR_NOT_NULL(t2); toverflow =
//     psy_time_point_subtract_dur(t2, two_us); CU_ASSERT_PTR_NULL(toverflow);
//     toverflow = psy_time_point_add(t2, n_two_us);
//     CU_ASSERT_PTR_NULL(toverflow);
//
//     g_object_unref(t1);
//     g_object_unref(t2);
//     g_object_unref(dur_max);
//     g_object_unref(one_us);
//     g_object_unref(two_us);
//     g_object_unref(n_two_us);
// }

static void
check_duration_arithmetics(void)
{
    PsyDuration *d_us = NULL, *d_ms = NULL, *d_s = NULL, *d_res = NULL,
                *d_temp = NULL, *d_half = NULL, *sub_result = NULL,
                *mul_res = NULL;

    gdouble epsilon = 1e-9;

    d_us = psy_duration_new_us(5);
    d_ms = psy_duration_new_ms(5);
    d_s  = psy_duration_new_s(5);

    g_assert(d_us && d_ms && d_s);
    gint64  us, ms, s;
    gdouble seconds;

    d_temp = psy_duration_add(d_us, d_ms);
    g_assert(d_temp);
    d_res = psy_duration_add(d_temp, d_s);
    g_assert(d_res);

    us      = psy_duration_get_us(d_res);
    ms      = psy_duration_get_ms(d_res);
    s       = psy_duration_get_s(d_res);
    seconds = psy_duration_get_seconds(d_res);

    CU_ASSERT_EQUAL(us, 5 + 5 * 1000 + 5 * 1000000);
    CU_ASSERT_EQUAL(ms, 5 + 5 * 1000);
    CU_ASSERT_EQUAL(s, 5);
    CU_ASSERT_DOUBLE_EQUAL(seconds, 5.005005, epsilon);

    d_half = psy_duration_divide_scalar(d_s, 2);
    CU_ASSERT_EQUAL(psy_duration_get_ms(d_half), 2500)
    CU_ASSERT_EQUAL(psy_duration_get_us(d_half), 2500 * 1000);
    CU_ASSERT_EQUAL(psy_duration_divide(d_s, d_half), 2);

    sub_result = psy_duration_subtract(d_s, d_half);
    CU_ASSERT_PTR_NOT_NULL_FATAL(sub_result);

    CU_ASSERT_TRUE(psy_duration_equal(sub_result, d_half));

    mul_res = psy_duration_multiply_scalar(d_half, 2);
    CU_ASSERT_PTR_NOT_NULL_FATAL(mul_res);
    CU_ASSERT(psy_duration_equal(mul_res, d_s));

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
check_duration_rounded_division(void)
{
    PsyDuration *ten   = psy_duration_new(10);
    PsyDuration *eight = psy_duration_new(8);
    PsyDuration *six   = psy_duration_new(6);

    gint64 r1 = psy_duration_divide_rounded(ten, eight);
    gint64 r2 = psy_duration_divide_rounded(ten, six);

    CU_ASSERT_EQUAL(r1, 1); // 10 / 8 = 1.25
    CU_ASSERT_EQUAL(r2, 2); // 10 / 6 = 1.6667

    psy_duration_free(ten);
    psy_duration_free(eight);
    psy_duration_free(six);
}

static void
check_duration_comparisons(void)
{
    PsyDuration *one_s;
    PsyDuration *alittlemore;
    PsyDuration *alittleless;

    one_s       = psy_duration_new(1.0);
    alittleless = psy_duration_new_us(999999);
    alittlemore = psy_duration_new_us(999999 + 2);

    CU_ASSERT_TRUE(psy_duration_not_equal(one_s, alittleless));
    CU_ASSERT_TRUE(psy_duration_not_equal(one_s, alittlemore));

    CU_ASSERT_TRUE(psy_duration_equal(one_s, one_s));
    CU_ASSERT_TRUE(psy_duration_less_equal(one_s, one_s));
    CU_ASSERT_TRUE(psy_duration_greater_equal(one_s, one_s));

    CU_ASSERT_TRUE(psy_duration_less(alittleless, one_s));
    CU_ASSERT_TRUE(psy_duration_less_equal(alittleless, one_s));
    CU_ASSERT_FALSE(psy_duration_greater_equal(alittleless, one_s));
    CU_ASSERT_FALSE(psy_duration_greater(alittleless, one_s));

    CU_ASSERT_TRUE(psy_duration_greater(alittlemore, one_s));
    CU_ASSERT_TRUE(psy_duration_greater_equal(alittlemore, one_s));
    CU_ASSERT_FALSE(psy_duration_less(alittlemore, one_s));
    CU_ASSERT_FALSE(psy_duration_less_equal(alittlemore, one_s));

    psy_duration_free(one_s);
    psy_duration_free(alittlemore);
    psy_duration_free(alittleless);
}

int
add_time_utilities_suite(void)
{
    CU_Suite *suite = CU_add_suite("test reference count", NULL, NULL);
    CU_Test  *test  = NULL;

    if (!suite)
        return 1;

    test = CU_add_test(suite, "Test clock", test_clock);
    if (!test)
        return 1;

    test = CU_add_test(
        suite, "Test timepoint aritmetics", check_time_point_arithmetic);
    if (!test)
        return 1;

    test = CU_ADD_TEST(suite, test_psy_time_point_copy);

    test = CU_add_test(
        suite, "Test time point comparisons", check_time_point_comparisons);
    if (!test)
        return 1;

    //    test = CU_add_test(
    //        suite, "Test time point overflow", check_time_point_overflow);
    //    if (!test)
    //        return 1;

    test = CU_add_test(
        suite, "Test duration arithmetics", check_duration_arithmetics);
    if (!test)
        return 1;
    test = CU_add_test(suite,
                       "Test duration rounded division",
                       check_duration_rounded_division);
    if (!test)
        return 1;
    test = CU_add_test(
        suite, "Test duration comparisons", check_duration_comparisons);
    if (!test)
        return 1;

    return 0;
}
