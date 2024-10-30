
#include "unit-test-utilities.h"

#include <munit.h>
#include <psylib.h>

// Setup the tests

static int
timer_setup(void)
{
    install_log_handler();
    set_log_handler_level(G_LOG_LEVEL_INFO);
    set_log_handler_file("test-timer.txt");
    return 0;
}

static int
timer_teardown(void)
{
    set_log_handler_file(NULL);
    remove_log_handler();
    return 0;
}

// Have some utilities present

typedef struct {
    PsyInitializer *init;
    GMainLoop      *loop;
    GMainContext   *context;
} TimerTestUtilities;

static void *
fixture_new(const MunitParameter params[], void *user_data)
{
    (void) params;
    (void) user_data;
    TimerTestUtilities *ret = g_new(TimerTestUtilities, 1);

    ret->init = g_object_new(
        PSY_TYPE_INITIALIZER, "gstreamer", FALSE, "portaudio", FALSE, NULL);

    ret->context = g_main_context_new();
    g_main_context_push_thread_default(ret->context);

    ret->loop = g_main_loop_new(ret->context, FALSE);

    return ret;
}

static void
fixture_free(void *data)
{
    TimerTestUtilities *utils = data;

    g_clear_pointer(&utils->loop, g_main_loop_unref);
    g_main_context_pop_thread_default(utils->context);
    g_clear_pointer(&utils->context, g_main_context_unref);

    g_clear_object(&utils->init);
    g_free(utils);
}

static MunitResult
test_timer_create(const MunitParameter params[], void *user_data)
{
    (void) user_data;
    (void) params;
    PsyTimer *t1;

    t1 = psy_timer_new();
    munit_assert_not_null(t1);

    munit_assert_null(psy_timer_get_fire_time(t1));
    psy_timer_free(t1);

    return MUNIT_OK;
}

static MunitResult
test_timer_set_fire_time(const MunitParameter params[], void *user_data)
{
    (void) params;
    (void) user_data;

    PsyTimer     *t1;
    PsyClock     *clk = psy_clock_new();
    PsyTimePoint *now = psy_clock_now(clk);

    t1 = psy_timer_new();
    munit_assert_not_null(t1);
    if (!t1)
        return MUNIT_FAIL;

    g_object_set(t1, "fire-time", now, NULL);

    munit_assert_not_null(psy_timer_get_fire_time(t1));

    psy_time_point_free(now);
    psy_clock_free(clk);
    psy_timer_free(t1);

    return MUNIT_OK;
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
    TimerFireTest *fire_data = data;

    fire_data->fired = TRUE;
    fire_data->time_is_equal_to_set
        = psy_time_point_equal(fire_data->time_in, tp);
    fire_data->timer_is_the_same = fire_data->timer_in == t;

    // g_main_loop_quit(fire_data->utils->loop);

    return G_SOURCE_REMOVE;
}

static gboolean
quit_loop(gpointer data)
{
    TimerTestUtilities *utils = data;

    g_main_loop_quit(utils->loop);

    return G_SOURCE_REMOVE;
}

static MunitResult
test_timer_fire(const MunitParameter params[], void *user_data)
{
    (void) params;
    TimerTestUtilities *utils = user_data;

    PsyTimer     *t1  = NULL;
    PsyClock     *clk = psy_clock_new();
    PsyTimePoint *now = psy_clock_now(clk);

    t1 = psy_timer_new();
    munit_assert_not_null(t1);
    if (!t1)
        return MUNIT_FAIL;

    TimerFireTest test_data = {utils, now, t1, FALSE, FALSE, FALSE};

    g_signal_connect(t1, "fired", G_CALLBACK(on_timer_fire1), &test_data);
    g_object_set(t1, "fire-time", now, NULL);

    GSource *timeout = g_timeout_source_new(10);
    g_source_set_callback(timeout, quit_loop, utils, NULL);
    g_source_attach(timeout, utils->context);
    g_source_unref(timeout);

    g_timeout_add(10, G_SOURCE_FUNC(quit_loop), utils->loop);

    munit_assert_not_null(psy_timer_get_fire_time(t1));

    g_main_loop_run(utils->loop);

    munit_assert_true(test_data.fired);
    munit_assert_true(test_data.time_is_equal_to_set);
    munit_assert_true(test_data.timer_is_the_same);

    psy_time_point_free(now);
    psy_timer_free(t1);
    psy_clock_free(clk);

    return MUNIT_OK;
}

const int NUM_TIMERS = 100;
int       g_num_fired;

typedef struct {
    TimerTestUtilities *utils;
    PsyClock           *clk;
    PsyTimePoint       *fire_time; // owned
    PsyTimePoint       *scheduled; // owned
    int                *num_fired;
    PsyTimer           *timer; // owned.
} TimerFireAccuratelyTest;

static TimerFireAccuratelyTest *
timer_fire_accuratately_test_new(TimerTestUtilities *utils,
                                 PsyClock           *clk,
                                 PsyTimer           *timer,
                                 PsyTimePoint       *scheduled)
{
    TimerFireAccuratelyTest *ret = g_new(TimerFireAccuratelyTest, 1);

    g_assert(utils != NULL && clk != NULL && timer != NULL
             && scheduled != NULL);

    ret->utils     = utils;
    ret->clk       = clk;
    ret->timer     = timer;
    ret->scheduled = scheduled;

    // if *(ret->num_fired) == NUM_TIMERS where done
    ret->num_fired = &g_num_fired;

    return ret;
}

static void
timer_fire_accuratately_test_free(gpointer data)
{
    TimerFireAccuratelyTest *test_data = data;

    // psy_clock_free(test_data->clk); clock is not owned
    psy_time_point_free(test_data->fire_time);
    psy_time_point_free(test_data->scheduled);
    psy_timer_free(test_data->timer);

    g_free(test_data);
}

static void
on_timer_fire_accurately(PsyTimer *t, PsyTimePoint *tp, gpointer data)
{
    (void) t;
    (void) tp;
    TimerFireAccuratelyTest *fire_data = data;

    // Must be freed
    fire_data->fire_time = psy_clock_now(fire_data->clk);

    (*fire_data->num_fired)++;

    if ((*fire_data->num_fired) == NUM_TIMERS) {
        g_main_loop_quit(fire_data->utils->loop);
    }
}

static MunitResult
test_timer_fire_accurately(const MunitParameter params[], void *user_data)
{
    (void) params;
    gint                num_correct, n_failed = 0;
    TimerTestUtilities *utils = user_data;
    PsyClock           *clk   = psy_clock_new();
    PsyTimePoint       *now   = psy_clock_now(clk);
    GPtrArray          *timer_data
        = g_ptr_array_new_full(NUM_TIMERS, timer_fire_accuratately_test_free);

    for (int i = 0; i < NUM_TIMERS; i++) {

        PsyTimer *t1 = psy_timer_new();

        PsyDuration *dur = psy_duration_new_ms(munit_rand_int_range(100, 1000));
        PsyTimePoint *time_future = psy_time_point_add(now, dur);
        psy_duration_free(dur);

        g_object_set(t1, "fire-time", time_future, NULL);
        TimerFireAccuratelyTest *test_data
            = timer_fire_accuratately_test_new(utils, clk, t1, time_future);

        g_signal_connect(
            t1, "fired", G_CALLBACK(on_timer_fire_accurately), test_data);

        g_ptr_array_add(timer_data, test_data);
    }

    g_main_loop_run(utils->loop);

    for (int i = 0; i < NUM_TIMERS; i++) {
        TimerFireAccuratelyTest *test_data = g_ptr_array_index(timer_data, i);
        PsyDuration             *time_diff = NULL;

        time_diff = psy_time_point_subtract(test_data->fire_time,
                                            test_data->scheduled);

        // We expect that a timer is not fired ahead of time.
        munit_assert_int64(psy_duration_get_us(time_diff), >=, 0);

#if !defined(_WIN32) // Seems unlikely in CI does seem to work in vm/pc
        munit_assert_int64(psy_duration_get_us(time_diff), <, 1000);
#endif
        g_info("The timer was fired at %" PRId64 " us",
               psy_duration_get_us(time_diff));
        if (psy_duration_get_us(time_diff) >= 1000) {
            munit_logf(MUNIT_LOG_WARNING,
                       "Timer was late %lf\n",
                       psy_duration_get_seconds(time_diff));
            n_failed++;
        }

        psy_duration_free(time_diff);
    }

    num_correct        = NUM_TIMERS - n_failed;
    gdouble percentage = (double) num_correct / NUM_TIMERS * 100;
    munit_assert_double(percentage, >, 90.0);

    g_ptr_array_unref(timer_data);

    psy_time_point_free(now);
    psy_clock_free(clk);

    return MUNIT_OK;
}

// clang-format off
MunitTest tests[] = {
    {"create",test_timer_create, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"set-fire-time",test_timer_set_fire_time, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"fire",test_timer_fire, fixture_new, fixture_free , MUNIT_TEST_OPTION_NONE, NULL},
    {"fire-accurately",test_timer_fire_accurately, fixture_new, fixture_free , MUNIT_TEST_OPTION_NONE, NULL},
    {0}
};
// clang-format on

MunitSuite suite = {"psy-timer/", tests, NULL, 1, MUNIT_SUITE_OPTION_NONE};

int
main(int argc, char **argv)
{
    timer_setup();
    int ret = munit_suite_main(&suite, NULL, argc, argv);
    timer_teardown();
    return ret;
}

//
// int
// add_timer_suite(void)
// {
//     CU_Suite *suite
//         = CU_add_suite("PsyTimer suite", timer_setup, timer_teardown);
//     CU_Test *test;
//     if (!suite)
//         return 1;
//
//     test = CU_ADD_TEST(suite, test_timer_create);
//     if (!test)
//         return 1;
//
//     test = CU_ADD_TEST(suite, test_timer_set_fire_time);
//     if (!test)
//         return 1;
//
//     test = CU_ADD_TEST(suite, test_timer_fire);
//     if (!test)
//         return 1;
//
//     test = CU_ADD_TEST(suite, test_timer_fire_accurately);
//     if (!test)
//         return 1;
//
//     return 0;
// }
