#include "psy-enums.h"
#include "psy-stepping-stones.h"
#include <psylib.h>

#if GLIB_CHECK_VERSION(2, 74, 0)
gint g_default_flags = G_APPLICATION_DEFAULT_FLAGS;
#else
gint g_default_flags = G_APPLICATION_FLAGS_NONE;
#endif

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
    psy_time_point_free(now);
}

static void
test_basic_step(void)
{
    int           status;
    GApplication *app = g_application_new(NULL, g_default_flags);

    ActivateData data = {.app = app};

    g_signal_connect(
        app, "activate", G_CALLBACK(on_basic_step_activate), &data);

    status = g_application_run(G_APPLICATION(app), 0, NULL);
    g_assert_cmpint(status, ==, 0);
    g_object_unref(app);

    g_assert_true(data.activated);
    g_assert_true(data.entered);
    g_assert_true(data.left);
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
    psy_time_point_free(timestamp);
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

    g_assert_cmpint(index, ==, 0);     // "the default index starts at 0");
    g_assert_cmpint(stop, ==, 0);      // "the default stop is 0");
    g_assert_cmpint(increment, ==, 1); // "The default increment is 1");
    // The default continue criterion is whether the index is less than stop
    g_assert_cmpint(condition, ==, PSY_LOOP_CONDITION_LESS);
    g_assert_false(psy_loop_test(loop));

    psy_loop_free(loop);

    // test PSY_LOOP_LESS
    index = 0, increment = 1, stop = 5;
    LoopParams lparams
        = {.index = index, .stop = stop, .increment = increment, .stats = {0}};
    LoopStats regular = {0};

    GApplication *app = g_application_new(NULL, g_default_flags);
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

    g_assert_cmpint(lparams.stats.count, ==, regular.count);
    g_assert_cmpint(lparams.stats.sum, ==, regular.sum);
    g_assert_cmpint(lparams.stats.index_at_end, ==, regular.index_at_end);

    // test PSY_LOOP_GREATER_EQUAL
    index = 10, increment = -2, stop = -10;
    lparams.index     = index;
    lparams.stop      = stop;
    lparams.increment = increment;
    lparams.condition = PSY_LOOP_CONDITION_GREATER_EQUAL;
    memset(&lparams.stats, 0, sizeof(lparams.stats));
    memset(&regular, 0, sizeof(regular));

    app = g_application_new(NULL, g_default_flags);
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

    g_assert_cmpint(lparams.stats.count, ==, regular.count);
    g_assert_cmpint(lparams.stats.sum, ==, regular.sum);
    g_assert_cmpint(lparams.stats.index_at_end, ==, regular.index_at_end);

    // PSY_LOOP_LESS_EQUAL
    index = 0, increment = 2, stop = 19;
    lparams.index     = index;
    lparams.stop      = stop;
    lparams.increment = increment;
    lparams.condition = PSY_LOOP_CONDITION_LESS_EQUAL;
    memset(&lparams.stats, 0, sizeof(lparams.stats));
    memset(&regular, 0, sizeof(regular));

    app = g_application_new(NULL, g_default_flags);
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

    g_assert_cmpint(lparams.stats.count, ==, regular.count);
    g_assert_cmpint(lparams.stats.sum, ==, regular.sum);
    g_assert_cmpint(lparams.stats.index_at_end, ==, regular.index_at_end);

    // PSY_LOOP_GREATER
    index = 0, increment = 2, stop = 19;
    lparams.index     = index;
    lparams.stop      = stop;
    lparams.increment = increment;
    lparams.condition = PSY_LOOP_CONDITION_GREATER;
    memset(&lparams.stats, 0, sizeof(lparams.stats));
    memset(&regular, 0, sizeof(regular));

    app = g_application_new(NULL, g_default_flags);
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

    g_assert_cmpint(lparams.stats.count, ==, regular.count);
    g_assert_cmpint(lparams.stats.sum, ==, regular.sum);
    g_assert_cmpint(lparams.stats.index_at_end, ==, regular.index_at_end);

    // PSY_LOOP_EQUAL
    index = 50, increment = 2, stop = 50;
    lparams.index     = index;
    lparams.stop      = stop;
    lparams.increment = increment;
    lparams.condition = PSY_LOOP_CONDITION_EQUAL;
    memset(&lparams.stats, 0, sizeof(lparams.stats));
    memset(&regular, 0, sizeof(regular));

    app = g_application_new(NULL, g_default_flags);
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

    g_assert_cmpint(lparams.stats.count, ==, regular.count);
    g_assert_cmpint(lparams.stats.sum, ==, regular.sum);
    g_assert_cmpint(lparams.stats.index_at_end, ==, regular.index_at_end);
}

static void
test_loop_add_children(void)
{
    PsyLoop  *loop  = psy_loop_new();
    PsyTrial *trial = psy_trial_new();
    PsyTrial *child = NULL;
    GError   *error = NULL;

    psy_loop_set_step(loop, PSY_STEP(trial), &error);
    g_assert_no_error(error);

    // Steps with children own them, transfer is full; the reference is now the
    // responsibility of the loop. we don't have to g_object_unref it here.
    GObject *gtrial = G_OBJECT(trial);
    g_assert_cmpuint(gtrial->ref_count, ==, 1u);

    g_object_get(loop, "child", &child, NULL);
    g_assert_cmphex((uintptr_t) trial, ==, (uintptr_t) child);
    g_object_unref(child);
    GObject *gchild = G_OBJECT(child);
    g_assert_cmpuint(gchild->ref_count, ==, 1u);

    psy_loop_free(loop);
}

typedef struct StoneParams {
    gint stones_activated;
    gint trial_activated;
    gint loop_iterations;
} StoneParams;

static void
on_stones_enter(PsyStep *step, gint64 timestamp, gpointer data)
{
    g_assert(PSY_IS_STEPPING_STONES(step));
    (void) step;
    (void) timestamp;
    StoneParams *pars = data;
    pars->stones_activated++;
}

static void
on_stones_leave(PsyStep *step, gint64 timestamp, gpointer data)
{
    (void) step;
    (void) timestamp;

    GApplication *app = data;
    g_application_release(app);
}

static void
on_stones_trial_enter(PsyStep *step, gint64 timestamp, gpointer data)
{
    (void) timestamp;
    StoneParams *pars = data;
    PsyClock    *clk  = psy_clock_new();

    PsyTimePoint *stamp = psy_clock_now(clk);
    psy_step_leave(step, stamp);
    pars->trial_activated++;

    g_object_unref(clk);
    psy_time_point_free(stamp);
}

static void
on_stones_loop_iterate(PsyLoop *loop,
                       gint64   index,
                       gint64   timestamp,
                       gpointer data)
{
    StoneParams *pars = data;
    (void) index;
    (void) timestamp;

    pars->loop_iterations++;
    PsyClock     *clk = psy_clock_new();
    PsyTimePoint *now = psy_clock_now(clk);
    psy_loop_iterate(loop, now);
    g_object_unref(clk);
    psy_time_point_free(now);
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
    PsyLoop *loop = psy_loop_new_full(0, 10, 1, PSY_LOOP_CONDITION_LESS);
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
    g_clear_error(&error);
    psy_stepping_stones_add_step(stones, PSY_STEP(loop), &error);
    g_assert_no_error(error);
    psy_stepping_stones_add_step(stones, PSY_STEP(rolling_stones), &error);
    g_assert_no_error(error);

    // Add the inner trial to the inner stepping stones.
    psy_stepping_stones_add_step(rolling_stones, PSY_STEP(inner_trial), &error);
    g_assert_no_error(error);
    psy_stepping_stones_add_step(rolling_stones, PSY_STEP(empty), &error);
    g_assert_no_error(error);

    now = psy_clock_now(clk);
    psy_step_enter(PSY_STEP(stones), now);
    psy_time_point_free(now);
    g_object_unref(clk);
}

static void
test_stepping_stones_default(void)
{
    int           status;
    GApplication *app = g_application_new(NULL, g_default_flags);

    StoneParams params = {0};

    g_signal_connect(app,
                     "activate",
                     G_CALLBACK(on_basic_stepping_stones_activate),
                     &params);
    status = g_application_run(G_APPLICATION(app), 0, NULL);
    g_assert(status == 0);
    g_object_unref(app);

    g_assert_cmpint(params.stones_activated, ==, 3);
    g_assert_cmpint(params.trial_activated, ==, 2);
    g_assert_cmpint(params.loop_iterations, ==, 10);
}

static void
test_stepping_stones_errors(void)
{
    GError            *error  = NULL;
    PsySteppingStones *stones = psy_stepping_stones_new();

    psy_stepping_stones_activate_next_by_name(stones, "blablah", &error);

    g_assert_error(error,
                   PSY_STEPPING_STONES_ERROR,
                   PSY_STEPPING_STONES_ERROR_NO_SUCH_KEY);
    g_clear_error(&error);

    psy_stepping_stones_activate_next_by_index(stones, 0, &error);
    g_assert_error(error,
                   PSY_STEPPING_STONES_ERROR,
                   PSY_STEPPING_STONES_ERROR_INVALID_INDEX);
    g_clear_error(&error);

    psy_stepping_stones_free(stones);
}

static void
test_steps_add_children(void)
{
    PsySteppingStones *stones = psy_stepping_stones_new();
    PsyLoop           *loop   = psy_loop_new();
    GError            *error  = NULL;

    guint num_steps = -1;

    g_object_get(stones, "num-steps", &num_steps, NULL);
    g_assert_cmpuint(num_steps, ==, 0u);

    PsyTrial *step_trial1 = psy_trial_new();
    PsyTrial *step_trial2 = psy_trial_new();
    PsyTrial *loop_trial  = psy_trial_new();

    gboolean ret;

    // Add fresh trials to a parent.
    ret = psy_stepping_stones_add_step(stones, PSY_STEP(step_trial1), &error);
    g_assert_true(ret);
    g_assert_no_error(error);

    g_object_get(stones, "num-steps", &num_steps, NULL);
    g_assert_cmpuint(num_steps, ==, 1u);

    ret = psy_stepping_stones_add_step_by_name(
        stones, "name", PSY_STEP(step_trial2), NULL);
    g_assert_true(ret);

    g_object_get(stones, "num-steps", &num_steps, NULL);
    g_assert_cmpuint(num_steps, ==, 2u);

    ret = psy_loop_set_step(loop, PSY_STEP(loop_trial), &error);
    g_assert_true(ret);
    g_assert_no_error(error);

    // Add trials that have a parent to as child step.
    // THESE SHOULD FAIL and should not add children.
    ret = psy_loop_set_step(loop, PSY_STEP(step_trial1), &error);
    g_assert_false(ret);
    g_assert_error(error, PSY_STEP_ERROR, PSY_STEP_ERROR_CHILD_HAS_PARENT);
    g_clear_error(&error);

    ret = psy_stepping_stones_add_step(stones, PSY_STEP(loop_trial), &error);
    g_assert_false(ret);
    g_assert_error(error, PSY_STEP_ERROR, PSY_STEP_ERROR_CHILD_HAS_PARENT);
    g_clear_error(&error);

    g_object_get(stones, "num-steps", &num_steps, NULL);
    g_assert_cmpuint(num_steps, ==, 2u);

    ret = psy_stepping_stones_add_step_by_name(
        stones, "name2", PSY_STEP(loop_trial), NULL);
    g_assert_false(ret);

    g_object_get(stones, "num-steps", &num_steps, NULL);
    g_assert_cmpuint(num_steps, ==, 2u);

    psy_stepping_stones_free(stones);
    psy_loop_free(loop);
}

static void
test_side_step_construct(void)
{
    PsySideStep *step = psy_side_step_new();
    g_assert_nonnull(step);
    psy_side_step_free(step);
}

typedef struct SideStepData {
    PsySteppingStones *stones;
    PsySideStep       *step1;
    PsySideStep       *step2;
    gint               step_1_activated;
    gint               step_2_activated;
    gboolean           step_2_first;
    gboolean           timed_out;
} SideStepData;

static void
side_stones_leave(PsyStep *stones, PsyTimePoint *tp, gpointer data)
{
    (void) stones;
    (void) tp;
    GMainLoop *loop = data;
    g_main_loop_quit(loop);
}

static void
side_step_activate(PsySideStep *step, PsyTimePoint *tp, gpointer data)
{
    (void) tp;
    SideStepData *ssd = data;
    if (ssd->step1 == step) {
        ssd->step_1_activated++;
    }
    if (ssd->step2 == step) {
        ssd->step_2_activated++;
        if (!ssd->step_1_activated) {
            ssd->step_2_first = TRUE;
            psy_stepping_stones_activate_next_by_index(ssd->stones, 0, NULL);
        }
    }
}

static void
test_steps_side_stepping(void)
{
    GMainLoop    *loop = g_main_loop_new(NULL, FALSE);
    PsyTimePoint *now  = NULL;
    PsyClock     *clk  = psy_clock_new();
    now                = psy_clock_now(clk);

    SideStepData data = {0};

    data.stones = psy_stepping_stones_new();
    data.step1  = psy_side_step_new();
    data.step2  = psy_side_step_new();

    psy_stepping_stones_add_step_by_name(
        data.stones, "step1", PSY_STEP(data.step1), NULL);
    psy_stepping_stones_add_step_by_name(
        data.stones, "step2", PSY_STEP(data.step2), NULL);

    g_signal_connect(data.stones, "leave", G_CALLBACK(side_stones_leave), loop);
    g_signal_connect(
        data.step1, "activate", G_CALLBACK(side_step_activate), &data);
    g_signal_connect(
        data.step2, "activate", G_CALLBACK(side_step_activate), &data);

    psy_step_enter(PSY_STEP(data.stones), now);

    psy_stepping_stones_activate_next_by_name(data.stones, "step2", NULL);

    g_main_loop_run(loop);

    g_assert_cmpint(data.step_1_activated, ==, 1);
    g_assert_cmpint(data.step_2_activated, ==, 2);
    g_assert_true(data.step_2_first);

    psy_time_point_free(now);
    psy_stepping_stones_free(data.stones);
    psy_clock_free(clk);
    g_main_loop_unref(loop);
}

static void
test_steps_cannot_embed_themselves(void)
{
    GError *error = NULL;

    PsyLoop *loop = psy_loop_new();
    psy_loop_set_step(loop, PSY_STEP(loop), &error);
    g_assert_error(error, PSY_STEP_ERROR, PSY_STEP_ERROR_INVALID_CHILD);
    g_clear_error(&error);

    PsySteppingStones *stones = psy_stepping_stones_new();
    psy_stepping_stones_add_step(stones, PSY_STEP(stones), &error);
    g_assert_error(error, PSY_STEP_ERROR, PSY_STEP_ERROR_INVALID_CHILD);
    g_clear_error(&error);

    psy_stepping_stones_add_step_by_name(
        stones, "myself", PSY_STEP(stones), &error);
    g_assert_error(error, PSY_STEP_ERROR, PSY_STEP_ERROR_INVALID_CHILD);
    g_clear_error(&error);

    g_object_unref(loop);
    g_object_unref(stones);
}

int
main(int argc, char **argv)
{
    g_test_init(&argc, &argv, NULL);

    g_test_add_func("/step/basic_step", test_basic_step);
    g_test_add_func("/step/basic_loop", test_basic_loop);
    g_test_add_func("/step/loop_add_children", test_loop_add_children);
    g_test_add_func("/step/stepping_stones_default",
                    test_stepping_stones_default);
    g_test_add_func("/step/stepping_stones_errors",
                    test_stepping_stones_errors);
    g_test_add_func("/step/steps_add_children", test_steps_add_children);
    g_test_add_func("/step/side_step_construct", test_side_step_construct);
    g_test_add_func("/step/side_stepping", test_steps_side_stepping);
    g_test_add_func("/step/dont_embed_themselves",
                    test_steps_cannot_embed_themselves);

    return g_test_run();
}
