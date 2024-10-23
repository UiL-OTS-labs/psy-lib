
#include <math.h>
#include <string.h>

#include "unit-test-utilities.h"
#include <CUnit/CUnit.h>

#include <psylib.h>

// Setup the tests

static int
timer_setup(void)
{
    set_log_handler_file("test-timer.txt");
    return 0;
}

static int
timer_teardown(void)
{
    set_log_handler_file(NULL);
    return 0;
}

// Have some utilities present

typedef struct {
    GMainLoop *loop;
    PsyClock  *clk;
} TimerTestUtilities;

static TimerTestUtilities *
timer_test_utilities_new(void)
{
    TimerTestUtilities *ret = g_new(TimerTestUtilities, 1);

    ret->clk  = psy_clock_new();
    ret->loop = g_main_loop_new(NULL, FALSE);

    return ret;
}

static void
timer_test_utilities_free(TimerTestUtilities *utils)
{
    g_clear_pointer(&utils->loop, g_main_loop_unref);
    g_clear_object(&utils->clk);
    g_free(utils);
}

static void
test_timer_create(void)
{
    PsyTimer *t1;

    t1 = psy_timer_new();
    CU_ASSERT_PTR_NOT_NULL_FATAL(t1);

    CU_ASSERT_PTR_NULL(psy_timer_get_fire_time(t1));
    psy_timer_free(t1);
}

static void
test_timer_set_fire_time(void)
{
    TimerTestUtilities *utils = timer_test_utilities_new();

    PsyTimer     *t1;
    PsyTimePoint *now = psy_clock_now(utils->clk);

    t1 = psy_timer_new();
    CU_ASSERT_PTR_NOT_NULL_FATAL(t1);

    g_object_set(t1, "fire-time", now, NULL);

    CU_ASSERT_PTR_NOT_NULL(psy_timer_get_fire_time(t1));

    psy_time_point_free(now);
    timer_test_utilities_free(utils);
}

typedef struct {
    TimerTestUtilities *utils;

    PsyTimePoint *time_in;
    PsyTimer     *timer_in;

    gboolean fired;
    gboolean time_is_equal_to_set;
    gboolean timer_is_the_same;
} TimerFireTest;

static gboolean
on_timer_fire1(PsyTimer *t, PsyTimePoint *tp, gpointer data)
{
    TimerFireTest *fire_test = data;

    fire_test->fired = TRUE;
    fire_test->time_is_equal_to_set
        = psy_time_point_equal(fire_test->time_in, tp);
    fire_test->timer_is_the_same = fire_test->timer_in == t;

    g_main_loop_quit(fire_test->utils->loop);

    return G_SOURCE_REMOVE;
}

static void
test_timer_fire(void)
{
    TimerTestUtilities *utils = timer_test_utilities_new();
    PsyTimer           *t1    = NULL;
    PsyTimePoint       *now   = psy_clock_now(utils->clk);

    t1 = psy_timer_new();

    TimerFireTest test_data = {utils, now, t1, FALSE, FALSE, FALSE};

    CU_ASSERT_PTR_NOT_NULL_FATAL(t1);

    g_object_set(t1, "fire-time", now, NULL);
    g_signal_connect(t1, "fired", G_CALLBACK(on_timer_fire1), &test_data);

    g_timeout_add(10, G_SOURCE_FUNC(g_main_loop_quit), utils->loop);

    CU_ASSERT_PTR_NOT_NULL(psy_timer_get_fire_time(t1));

    g_main_loop_run(utils->loop);

    CU_ASSERT_TRUE(test_data.fired);
    CU_ASSERT_TRUE(test_data.time_is_equal_to_set);
    CU_ASSERT_TRUE(test_data.timer_is_the_same);

    psy_time_point_free(now);
    timer_test_utilities_free(utils);
}

int
add_timer_suite(void)
{
    CU_Suite *suite
        = CU_add_suite("PsyTimer suite", timer_setup, timer_teardown);
    CU_Test *test;
    if (!suite)
        return 1;

    test = CU_ADD_TEST(suite, test_timer_create);
    if (!test)
        return 1;

    test = CU_ADD_TEST(suite, test_timer_set_fire_time);
    if (!test)
        return 1;

    test = CU_ADD_TEST(suite, test_timer_fire);
    if (!test)
        return 1;

    return 0;
}
