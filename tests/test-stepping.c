
#include <assert.h>

#include <CUnit/CUnit.h>

#include <psy-clock.h>
#include <psy-loop.h>
#include <psy-stepping-stones.h>
#include <psy-trial.h>

typedef struct ActivateData {
    GApplication *app;
    gboolean      activated;
    gboolean      entered;
    gboolean      left;
} ActivateData;

void
on_trial_enter(PsyStep *self, PsyTimePoint *tstamp, gpointer data)
{
    ActivateData *ad = data;
    ad->entered      = TRUE;
    psy_step_leave(self, tstamp);
}

void
on_trial_leave(PsyStep *self, PsyTimePoint *tstamp, gpointer data)
{
    (void) self;
    (void) tstamp;
    ActivateData *ad = data;
    ad->left         = TRUE;
    g_application_release(ad->app);
}

void
on_final_step_leave(PsyStep *step, PsyTimePoint *timestamp, gpointer data)
{
    (void) step;
    (void) timestamp;
    GApplication *app = G_APPLICATION(data);

    g_application_release(app);
}

static void
on_basic_step_shut_down(GApplication *app, gpointer data)
{
    (void) app;
    PsyStep *step = PSY_STEP(data);
    g_object_unref(step);
}

static void
on_basic_step_activate(GApplication *app, gpointer data)
{
    PsyClock     *clk = psy_clock_new();
    PsyTimePoint *now, *in_cb;
    ActivateData *ad = data;
    ad->activated    = TRUE;
    (void) in_cb;

    g_assert(g_application_get_is_registered(G_APPLICATION(app)));

    g_application_hold(app);

    PsyTrial *trial = psy_trial_new();

    g_signal_connect(trial, "enter", G_CALLBACK(on_trial_enter), ad);
    g_signal_connect(trial, "leave", G_CALLBACK(on_trial_leave), ad);
    g_signal_connect(
        app, "shutdown", G_CALLBACK(on_basic_step_shut_down), trial);

    now = psy_clock_now(clk);
    psy_step_enter(PSY_STEP(trial), now);

    g_object_unref(clk);
    g_object_unref(now);
}

static void
test_basic_step(void)
{
    int           status;
    GApplication *app = g_application_new(NULL, G_APPLICATION_FLAGS_NONE);

    ActivateData data = {.app = app};

    g_signal_connect(
        app, "activate", G_CALLBACK(on_basic_step_activate), &data);

    status = g_application_run(G_APPLICATION(app), 0, NULL);
    assert(status == 0);
    g_object_unref(app);

    CU_ASSERT_TRUE(data.activated);
    CU_ASSERT_TRUE(data.entered);
    CU_ASSERT_TRUE(data.left);
}

typedef struct LoopStats {
    gint64 count;
    gint64 sum;
    gint64 index_at_end;
} LoopStats;

typedef struct LoopParams {
    gint64           index;
    gint64           increment;
    gint64           stop;
    PsyLoopCondition condition;
    LoopStats        stats;
} LoopParams;

static void
on_loop_iter(PsyLoop      *loop,
             gint64        index,
             PsyTimePoint *timestamp,
             gpointer      data)
{
    LoopStats *stats = data;

    stats->index_at_end = index;
    stats->count++;
    stats->sum += index;

    g_assert_true(index == psy_loop_get_index(loop));

    psy_step_activate(PSY_STEP(loop), timestamp);
}

static void
on_basic_loop_activate(GApplication *app, gpointer data)
{
    PsyClock *clk = psy_clock_new();
    g_application_hold(app);
    LoopParams *params = data;
    // clang-format off
    PsyLoop* loop = g_object_new(PSY_TYPE_LOOP,
                                 "index", params->index,
                                 "stop", params->stop,
                                 "increment", params->increment,
                                 "condition", params->condition,
                                 NULL
                                 );
    // clang-format on

    g_signal_connect(
        loop, "iteration", G_CALLBACK(on_loop_iter), &params->stats);
    g_signal_connect(loop, "leave", G_CALLBACK(on_final_step_leave), app);

    PsyTimePoint *timestamp = psy_clock_now(clk);
    psy_step_enter(PSY_STEP(loop), timestamp);

    g_object_unref(clk);
    g_object_unref(timestamp);
}

static void
test_basic_loop(void)
{
    int status;

    gint64           index, stop, increment;
    PsyLoopCondition condition;

    // Test default parameters
    PsyLoop *loop = psy_loop_new();

    // clang-format off
    g_object_get(loop,
                 "index", &index,
                 "stop", &stop,
                 "increment", &increment,
                 "condition", &condition,
                 NULL);
    // clang-format on

    CU_ASSERT_EQUAL(index, 0);
    CU_ASSERT_EQUAL(stop, 0);
    CU_ASSERT_EQUAL(increment, 1);
    CU_ASSERT_EQUAL(condition, PSY_LOOP_CONDITION_LESS);
    CU_ASSERT_FALSE(psy_loop_test(loop));

    psy_loop_destroy(loop);

    // test PSY_LOOP_LESS
    index = 0, increment = 1, stop = 5;
    LoopParams lparams
        = {.index = index, .stop = stop, .increment = increment, .stats = {0}};
    LoopStats regular = {0};

    GApplication *app = g_application_new(NULL, G_APPLICATION_FLAGS_NONE);
    g_signal_connect(
        app, "activate", G_CALLBACK(on_basic_loop_activate), &lparams);

    status = g_application_run(app, 0, NULL);
    g_assert(status == 0);
    g_object_unref(app);

    for (gint64 i = index; i < stop; i += increment) {
        regular.count++;
        regular.sum += i;
        regular.index_at_end = i;
    }

    CU_ASSERT_EQUAL(lparams.stats.count, regular.count);
    CU_ASSERT_EQUAL(lparams.stats.sum, regular.sum);
    CU_ASSERT_EQUAL(lparams.stats.index_at_end, regular.index_at_end);

    // test PSY_LOOP_GREATER_EQUAL
    index = 10, increment = -2, stop = -10;
    lparams.index     = index;
    lparams.stop      = stop;
    lparams.increment = increment;
    lparams.condition = PSY_LOOP_CONDITION_GREATER_EQUAL;
    memset(&lparams.stats, 0, sizeof(lparams.stats));
    memset(&regular, 0, sizeof(regular));

    app = g_application_new(NULL, G_APPLICATION_FLAGS_NONE);
    g_signal_connect(
        app, "activate", G_CALLBACK(on_basic_loop_activate), &lparams);

    status = g_application_run(app, 0, NULL);
    g_assert(status == 0);
    g_object_unref(app);

    for (gint64 i = index; i >= stop; i += increment) {
        regular.count++;
        regular.sum += i;
        regular.index_at_end = i;
    }

    CU_ASSERT_EQUAL(lparams.stats.count, regular.count);
    CU_ASSERT_EQUAL(lparams.stats.sum, regular.sum);
    CU_ASSERT_EQUAL(lparams.stats.index_at_end, regular.index_at_end);

    // PSY_LOOP_LESS_EQUAL
    index = 0, increment = 2, stop = 19;
    lparams.index     = index;
    lparams.stop      = stop;
    lparams.increment = increment;
    lparams.condition = PSY_LOOP_CONDITION_LESS_EQUAL;
    memset(&lparams.stats, 0, sizeof(lparams.stats));
    memset(&regular, 0, sizeof(regular));

    app = g_application_new(NULL, G_APPLICATION_FLAGS_NONE);
    g_signal_connect(
        app, "activate", G_CALLBACK(on_basic_loop_activate), &lparams);

    status = g_application_run(app, 0, NULL);
    g_assert(status == 0);
    g_object_unref(app);

    for (gint64 i = index; i <= stop; i += increment) {
        regular.count++;
        regular.sum += i;
        regular.index_at_end = i;
    }

    CU_ASSERT_EQUAL(lparams.stats.count, regular.count);
    CU_ASSERT_EQUAL(lparams.stats.sum, regular.sum);
    CU_ASSERT_EQUAL(lparams.stats.index_at_end, regular.index_at_end);

    // PSY_LOOP_GREATER
    index = 0, increment = 2, stop = 19;
    lparams.index     = index;
    lparams.stop      = stop;
    lparams.increment = increment;
    lparams.condition = PSY_LOOP_CONDITION_GREATER;
    memset(&lparams.stats, 0, sizeof(lparams.stats));
    memset(&regular, 0, sizeof(regular));

    app = g_application_new(NULL, G_APPLICATION_FLAGS_NONE);
    g_signal_connect(
        app, "activate", G_CALLBACK(on_basic_loop_activate), &lparams);

    status = g_application_run(app, 0, NULL);
    g_assert(status == 0);
    g_object_unref(app);

    for (gint64 i = index; i > stop; i += increment) {
        regular.count++;
        regular.sum += i;
        regular.index_at_end = i;
    }

    CU_ASSERT_EQUAL(lparams.stats.count, regular.count);
    CU_ASSERT_EQUAL(lparams.stats.sum, regular.sum);
    CU_ASSERT_EQUAL(lparams.stats.index_at_end, regular.index_at_end);

    // PSY_LOOP_EQUAL
    index = 50, increment = 2, stop = 50;
    lparams.index     = index;
    lparams.stop      = stop;
    lparams.increment = increment;
    lparams.condition = PSY_LOOP_CONDITION_EQUAL;
    memset(&lparams.stats, 0, sizeof(lparams.stats));
    memset(&regular, 0, sizeof(regular));

    app = g_application_new(NULL, G_APPLICATION_FLAGS_NONE);
    g_signal_connect(
        app, "activate", G_CALLBACK(on_basic_loop_activate), &lparams);

    status = g_application_run(app, 0, NULL);
    g_assert(status == 0);
    g_object_unref(app);

    for (gint64 i = index; i == stop; i += increment) {
        regular.count++;
        regular.sum += i;
        regular.index_at_end = i;
    }

    CU_ASSERT_EQUAL(lparams.stats.count, regular.count);
    CU_ASSERT_EQUAL(lparams.stats.sum, regular.sum);
    CU_ASSERT_EQUAL(lparams.stats.index_at_end, regular.index_at_end);
}

typedef struct StoneParams {
    gint stones_activated;
    gint trial_activated;
    gint loop_iterations;
} StoneParams;

static void
on_stones_enter(PsyStep *step, gint64 timestamp, gpointer data)
{
    // g_print("%s\n", __func__);
    g_assert(PSY_IS_STEPPING_STONES(step));
    (void) step;
    (void) timestamp;
    StoneParams *pars = data;
    pars->stones_activated++;
}

static void
on_stones_leave(PsyStep *step, gint64 timestamp, gpointer data)
{
    // g_print("%s\n", __func__);
    (void) step;
    (void) timestamp;

    GApplication *app = data;
    g_application_release(app);
}

static void
on_stones_trial_enter(PsyStep *step, gint64 timestamp, gpointer data)
{
    // g_print("%s\n", __func__);
    (void) timestamp;
    StoneParams *pars = data;
    PsyClock    *clk  = psy_clock_new();

    PsyTimePoint *stamp = psy_clock_now(clk);
    psy_step_leave(step, stamp);
    pars->trial_activated++;

    g_object_unref(clk);
    g_object_unref(stamp);
}

static void
on_stones_loop_iterate(PsyLoop *loop,
                       gint64   index,
                       gint64   timestamp,
                       gpointer data)
{
    // g_print("%s\n", __func__);
    StoneParams *pars = data;
    (void) index;
    (void) timestamp;

    pars->loop_iterations++;
    PsyClock     *clk = psy_clock_new();
    PsyTimePoint *now = psy_clock_now(clk);
    psy_loop_iterate(loop, now);
    g_object_unref(clk);
    g_object_unref(now);
}

static void
on_basic_stepping_stones_activate(GApplication *app, gpointer data)
{
    (void) data;
    g_application_hold(app);
    GError       *error = NULL;
    StoneParams  *pars  = data;
    PsyClock     *clk   = psy_clock_new();
    PsyTimePoint *now   = NULL;

    PsySteppingStones *stones = psy_stepping_stones_new();
    PsyTrial          *trial  = psy_trial_new();
    PsyLoop *loop = psy_loop_new_full(0, 10000, 1, PSY_LOOP_CONDITION_LESS);
    PsySteppingStones *rolling_stones = psy_stepping_stones_new();
    PsySteppingStones *empty          = psy_stepping_stones_new();
    PsyTrial          *inner_trial    = psy_trial_new();

    g_signal_connect(stones, "enter", G_CALLBACK(on_stones_enter), pars);
    g_signal_connect(stones, "leave", G_CALLBACK(on_stones_leave), app);
    g_signal_connect(trial, "enter", G_CALLBACK(on_stones_trial_enter), pars);
    g_signal_connect(
        loop, "iteration", G_CALLBACK(on_stones_loop_iterate), pars);
    g_signal_connect(
        rolling_stones, "enter", G_CALLBACK(on_stones_enter), pars);
    g_signal_connect(empty, "enter", G_CALLBACK(on_stones_enter), pars);
    g_signal_connect(
        inner_trial, "enter", G_CALLBACK(on_stones_trial_enter), pars);

    psy_stepping_stones_add_step_by_name(
        stones, "trial", PSY_STEP(trial), &error);
    g_assert_no_error(error);
    g_assert_true(PSY_STEP(stones) == psy_step_get_parent(PSY_STEP(trial)));
    psy_stepping_stones_add_step_by_name(
        stones, "trial", PSY_STEP(loop), &error);
    g_assert_error(
        error, PSY_STEPPING_STONES_ERROR, PSY_STEPPING_STONES_ERROR_KEY_EXISTS);
    g_error_free(error);
    error = NULL;
    psy_stepping_stones_add_step(stones, PSY_STEP(loop));
    psy_stepping_stones_add_step(stones, PSY_STEP(rolling_stones));

    // Add the inner trial to the inner stepping stones.
    psy_stepping_stones_add_step(rolling_stones, PSY_STEP(inner_trial));
    psy_stepping_stones_add_step(rolling_stones, PSY_STEP(empty));

    now = psy_clock_now(clk);
    psy_step_enter(PSY_STEP(stones), now);
    g_object_unref(now);
    g_object_unref(clk);
}

static void
test_stepping_stones(void)
{
    int           status;
    GApplication *app = g_application_new(NULL, G_APPLICATION_FLAGS_NONE);

    StoneParams params = {0};

    g_signal_connect(app,
                     "activate",
                     G_CALLBACK(on_basic_stepping_stones_activate),
                     &params);
    status = g_application_run(G_APPLICATION(app), 0, NULL);
    g_assert(status == 0);
    g_object_unref(app);

    CU_ASSERT_EQUAL(params.stones_activated, 3);
    CU_ASSERT_EQUAL(params.trial_activated, 2);
    CU_ASSERT_EQUAL(params.loop_iterations, 10000);
}

int
add_stepping_suite(void)
{
    CU_Suite *suite = CU_add_suite("Stepping tests.", NULL, NULL);
    CU_Test  *test  = NULL;

    if (!suite)
        return 1;

    test = CU_add_test(suite, "Test base class PsyStep", test_basic_step);
    if (!test)
        return 1;

    test = CU_add_test(suite, "Test PsyLoop", test_basic_loop);
    if (!test)
        return 1;

    test = CU_add_test(suite, "Test PsySteppingStones", test_stepping_stones);
    if (!test)
        return 1;

    return 0;
}
