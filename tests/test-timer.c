#include <criterion/criterion.h>
#include <criterion/new/assert.h>
#include <psylib.h>
#include <signal.h>

#include "unit-test-utilities.h"

// Setup the tests

const int NUM_TIMERS       = 100;
const int NUM_SIMULTANEOUS = 25;

// make this configurable for CI
#define UPPER_BOUND 10000

static void
timer_setup(void)
{
    install_log_handler();
    set_log_handler_level(G_LOG_LEVEL_DEBUG);
    set_log_handler_file("test-timer.txt");
}

static void
timer_teardown(void)
{
    set_log_handler_file(NULL);
    remove_log_handler();
}

TestSuite(timer, .init = timer_setup, .fini = timer_teardown);

// Have some utilities present

typedef struct {
    PsyInitializer *init;
    GMainLoop      *loop;
    GMainContext   *context;
    int             num_fired;
} TimerTestUtilities;

static TimerTestUtilities *
timer_test_utilities_new(void)
{
    TimerTestUtilities *ret = g_new(TimerTestUtilities, 1);

    ret->init = g_object_new(
        PSY_TYPE_INITIALIZER, "gstreamer", FALSE, "portaudio", FALSE, NULL);

    ret->context = g_main_context_new();
    g_main_context_push_thread_default(ret->context);

    ret->loop = g_main_loop_new(ret->context, FALSE);

    ret->num_fired = 0;

    return ret;
}

static void
timer_test_utilities_free(TimerTestUtilities *utils)
{
    g_clear_pointer(&utils->loop, g_main_loop_unref);
    g_main_context_pop_thread_default(utils->context);
    g_clear_pointer(&utils->context, g_main_context_unref);

    g_clear_object(&utils->init);

    g_free(utils);
}

Test(timer, timer_create)
{
    PsyTimer     *t1;
    PsyTimePoint *tf = NULL;

    t1 = psy_timer_new();
    cr_assert(t1 != NULL, "Timers can be created");

    cr_assert(zero(tf = psy_timer_get_fire_time(t1)),
              "The don't have an fire time when not yet set");
    psy_timer_free(t1);
}

Test(timer, set_fire_time)
{
    PsyTimer     *t1;
    PsyClock     *clk = psy_clock_new();
    PsyTimePoint *now = psy_clock_now(clk);
    t1                = psy_timer_new();

    PsyTimePoint *ft = NULL;

    g_object_set(t1, "fire-time", now, NULL);

    ft = psy_timer_get_fire_time(t1);
    cr_assert(ne(ft, NULL), "The timer should have a fire-time when it is set");

    psy_time_point_free(ft);
    psy_time_point_free(now);
    psy_clock_free(clk);
    psy_timer_free(t1);
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

Test(timer, fire)
{
    TimerTestUtilities *utils = timer_test_utilities_new();
    cr_assert(ne(utils, NULL));

    PsyTimer     *t1  = NULL;
    PsyClock     *clk = psy_clock_new();
    PsyTimePoint *now = psy_clock_now(clk);
    PsyTimePoint *ft  = NULL;

    t1 = psy_timer_new();
    cr_assert(ne(t1, NULL));

    TimerFireTest test_data = {utils, now, t1, FALSE, FALSE, FALSE};

    g_signal_connect(t1, "fired", G_CALLBACK(on_timer_fire1), &test_data);
    g_object_set(t1, "fire-time", now, NULL);

    GSource *timeout = g_timeout_source_new(10);
    g_source_set_callback(timeout, quit_loop, utils, NULL);
    g_source_attach(timeout, utils->context);
    g_source_unref(timeout);

    g_timeout_add(10, G_SOURCE_FUNC(quit_loop), utils->loop);

    ft = psy_timer_get_fire_time(t1);
    cr_assert(ne(ft, NULL));

    g_main_loop_run(utils->loop);

    cr_expect(eq(test_data.fired, TRUE), "The timer should have fired");
    cr_expect(test_data.time_is_equal_to_set);
    cr_expect(test_data.timer_is_the_same);

    psy_time_point_free(ft);
    psy_time_point_free(now);
    psy_timer_free(t1);
    psy_clock_free(clk);

    timer_test_utilities_free(utils);
}

typedef struct {
    TimerTestUtilities *utils;
    PsyClock           *clk;
    PsyTimePoint       *fire_time; // owned
    PsyTimePoint       *scheduled; // owned
    PsyTimer           *timer;     // owned.
} TimerFireAccuratelyTest;

static TimerFireAccuratelyTest *
timer_fire_accuratately_test_new(TimerTestUtilities *utils,
                                 PsyClock           *clk,
                                 PsyTimer           *timer,
                                 PsyTimePoint       *scheduled)
{
    TimerFireAccuratelyTest *ret = g_new0(TimerFireAccuratelyTest, 1);

    g_assert(utils != NULL && clk != NULL && timer != NULL
             && scheduled != NULL);

    ret->utils     = utils;
    ret->clk       = clk;
    ret->timer     = timer;
    ret->scheduled = scheduled;

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

    fire_data->utils->num_fired++;

    if (fire_data->utils->num_fired == NUM_TIMERS) {
        g_main_loop_quit(fire_data->utils->loop);
    }
}

Test(timer, fire_accurately, .timeout = 2.0)
{
    gint num_correct, n_failed = 0;

    TimerTestUtilities *utils = timer_test_utilities_new();
    PsyClock           *clk   = psy_clock_new();
    PsyTimePoint       *now   = psy_clock_now(clk);
    GPtrArray          *timer_data
        = g_ptr_array_new_full(NUM_TIMERS, timer_fire_accuratately_test_free);

    g_info("Timer accuracy test");

    gint upper_time_bound = UPPER_BOUND;

    for (int i = 0; i < NUM_TIMERS; i++) {

        PsyTimer *t1 = psy_timer_new();

        PsyDuration  *dur = psy_duration_new_ms(g_random_int_range(100, 1000));
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
        cr_expect(ge(u64, psy_duration_get_us(time_diff), 0),
                  "Timers should not fire to early");

#if !defined(_WIN32) // Seems unlikely in CI does seem to work in vm/pc
        cr_expect(lt(i64, psy_duration_get_us(time_diff), upper_time_bound));
#endif
        g_info("The timer was fired at %" PRId64 " us",
               psy_duration_get_us(time_diff));
        if (psy_duration_get_us(time_diff) >= upper_time_bound) {
            cr_log_warn("Timer was late %lf\n",
                        psy_duration_get_seconds(time_diff));
            n_failed++;
        }

        psy_duration_free(time_diff);
    }

    num_correct        = NUM_TIMERS - n_failed;
    gdouble percentage = (double) num_correct / NUM_TIMERS * 100;
    cr_expect(gt(dbl, percentage, 90.0),
              "90%% of timers are expected to finish on time");

    g_ptr_array_unref(timer_data);

    psy_time_point_free(now);
    psy_clock_free(clk);

    timer_test_utilities_free(utils);
}

static gboolean
async_quit_loop(gpointer data)
{
    TimerFireAccuratelyTest *test_data = data;
    g_main_loop_quit(test_data->utils->loop);

    return G_SOURCE_REMOVE;
}

static void
fire_async_cb(PsyTimePoint *tp, gpointer data)
{
    (void) tp;
    TimerFireAccuratelyTest *fire_data = data;

    // Must be freed
    fire_data->fire_time = psy_clock_now(fire_data->clk);

    fire_data->utils->num_fired++;

    if (fire_data->utils->num_fired == NUM_TIMERS) {
        g_info("Stopping fire_async test");
        GSource *source = g_idle_source_new();
        g_source_set_callback(source, async_quit_loop, data, NULL);
        g_source_attach(source, fire_data->utils->context);
        g_source_unref(source);
    }
}

Test(timer, fire_async)
{
    gint                num_correct, n_failed = 0;
    TimerTestUtilities *utils = timer_test_utilities_new();
    PsyClock           *clk   = psy_clock_new();
    PsyTimePoint       *now   = psy_clock_now(clk);

    GPtrArray *timer_data
        = g_ptr_array_new_full(NUM_TIMERS, timer_fire_accuratately_test_free);

    gint upper_time_bound = UPPER_BOUND;

    g_info("Timer async callback test");

    for (int i = 0; i < NUM_TIMERS; i++) {

        PsyTimer *t1 = psy_timer_new();

        PsyDuration  *dur = psy_duration_new_ms(g_random_int_range(100, 1000));
        PsyTimePoint *time_future = psy_time_point_add(now, dur);
        psy_duration_free(dur);

        TimerFireAccuratelyTest *test_data
            = timer_fire_accuratately_test_new(utils, clk, t1, time_future);
        psy_timer_set_async_fire_cb(t1, fire_async_cb, test_data);

        g_object_set(t1, "fire-time", time_future, NULL);

        g_ptr_array_add(timer_data, test_data);
    }

    g_main_loop_run(utils->loop);

    for (int i = 0; i < NUM_TIMERS; i++) {
        TimerFireAccuratelyTest *test_data = g_ptr_array_index(timer_data, i);
        PsyDuration             *time_diff = NULL;

        time_diff = psy_time_point_subtract(test_data->fire_time,
                                            test_data->scheduled);

        // We expect that a timer is not fired ahead of time.
        cr_expect(ge(i64, psy_duration_get_us(time_diff), 0),
                  "Timers should not be called to early");

#if !defined(_WIN32) // Seems unlikely in CI does seem to work in vm/pc
        cr_expect(lt(i64, psy_duration_get_us(time_diff), 1000),
                  "Timers should not be fired to late");
#endif
        g_info("The timer was fired at %" PRId64 " us",
               psy_duration_get_us(time_diff));
        if (psy_duration_get_us(time_diff) >= upper_time_bound) {
            cr_log_warn("Timer was late %lf\n",
                        psy_duration_get_seconds(time_diff));
            n_failed++;
        }

        psy_duration_free(time_diff);
    }

    num_correct        = NUM_TIMERS - n_failed;
    gdouble percentage = (double) num_correct / NUM_TIMERS * 100;
    cr_expect(gt(percentage, 90.0),
              "expect 90%% of the timers to fire accurately");

    g_ptr_array_unref(timer_data);

    psy_time_point_free(now);
    psy_clock_free(clk);

    timer_test_utilities_free(utils);
}

static void
on_timer_fire_simutaneously(PsyTimer *t, PsyTimePoint *tp, gpointer data)
{
    (void) t;
    (void) tp;
    TimerFireAccuratelyTest *fire_data = data;

    // Must be freed
    fire_data->fire_time = psy_clock_now(fire_data->clk);

    fire_data->utils->num_fired++;

    if (fire_data->utils->num_fired == NUM_SIMULTANEOUS) {
        cr_log_info("quiting %s", __func__);
        g_main_loop_quit(fire_data->utils->loop);
    }
}

Test(timer, simultaneous)
{
    gint                num_correct, n_failed = 0;
    TimerTestUtilities *utils      = timer_test_utilities_new();
    PsyClock           *clk        = psy_clock_new();
    PsyTimePoint       *now        = psy_clock_now(clk);
    GPtrArray          *timer_data = g_ptr_array_new_full(
        NUM_SIMULTANEOUS, timer_fire_accuratately_test_free);
    PsyDuration *dur = psy_duration_new_ms(100);

    g_info("Timer simultaneous test");

    const int us_upper_bound = UPPER_BOUND;

    for (int i = 0; i < NUM_SIMULTANEOUS; i++) {

        PsyTimer *t1 = psy_timer_new();

        // freed by the function timer_fire_accuratately_test_free
        PsyTimePoint *time_future = psy_time_point_add(now, dur);

        TimerFireAccuratelyTest *test_data
            = timer_fire_accuratately_test_new(utils, clk, t1, time_future);

        g_object_set(t1, "fire-time", time_future, NULL);
        g_signal_connect(
            t1, "fired", G_CALLBACK(on_timer_fire_simutaneously), test_data);

        g_ptr_array_add(timer_data, test_data);
    }

    g_main_loop_run(utils->loop);

    for (int i = 0; i < NUM_SIMULTANEOUS; i++) {
        TimerFireAccuratelyTest *test_data = g_ptr_array_index(timer_data, i);
        PsyDuration             *time_diff = NULL;

        time_diff = psy_time_point_subtract(test_data->fire_time,
                                            test_data->scheduled);

        // We expect that a timer is not fired ahead of time.
        cr_expect(ge(psy_duration_get_us(time_diff), 0),
                  "Expect that timers are not ahead of time");

        g_info("The timer was fired at %" PRId64 " us",
               psy_duration_get_us(time_diff));
        if (psy_duration_get_us(time_diff) >= us_upper_bound) {
            cr_log_warn("Timer was late %lf\n",
                        psy_duration_get_seconds(time_diff));
            n_failed++;
        }

        psy_duration_free(time_diff);
    }

    num_correct        = NUM_SIMULTANEOUS - n_failed;
    gdouble percentage = (double) num_correct / NUM_SIMULTANEOUS * 100;
    cr_expect(ge(dbl, percentage, 90.0),
              "Expect that at least 90 of timers is fired in time");

    psy_duration_free(dur);
    g_ptr_array_unref(timer_data);

    psy_time_point_free(now);
    psy_clock_free(clk);

    timer_test_utilities_free(utils);
}
