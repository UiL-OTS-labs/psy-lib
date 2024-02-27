
#include <psylib.h>
#include <assert.h>

#define STOP 4
#define INDEX 0
#define INC 1

static void
on_loop_leave(PsyLoop *loop, PsyTimePoint *tp, gpointer outer_loop_incorrect)
{
    (void) loop;
    g_print("Leaving first loop starting incorrect one.\n");
    PsyStep *outer = outer_loop_incorrect;
    psy_step_enter(outer, tp);
}

void
on_second_loop_leave(PsyLoop *loop, PsyTimePoint *tp, gpointer main)
{
    (void) loop;
    (void) tp;
    GMainLoop *mainloop = main;
    g_main_loop_quit(mainloop);
}

static void
on_loop_enter(PsyLoop *loop, PsyTimePoint *tp)
{
    (void) tp;
    g_print("Setting up loop %p\n", (void *) loop);
    psy_loop_set_index(loop, INDEX);
    psy_loop_set_stop(loop, STOP);
    // The increment and condition of the loop remain valid in this case.
}

static void
on_trial_activate(PsyTrial *trial, PsyTimePoint *tp)
{
    gint64 outer = psy_step_get_loop_index(PSY_STEP(trial), 1, NULL);
    gint64 inner = psy_step_get_loop_index(PSY_STEP(trial), 0, NULL);
    g_print("{outer : %ld, inner : %ld}\n", outer, inner);
    psy_step_leave(PSY_STEP(trial), tp);
}

int
main(void)
{
    GMainLoop    *mainloop = g_main_loop_new(NULL, FALSE);
    PsyClock     *clk      = psy_clock_new();
    PsyTimePoint *tp2 = NULL, *tp1 = psy_clock_now(clk);
    PsyDuration  *dur = NULL;
    gboolean ret;

    PsyTrial *trial1 = psy_trial_new();
    PsyTrial *trial2 = psy_trial_new();

    PsyLoop *outer = psy_loop_new();
    PsyLoop *inner = psy_loop_new();

    // setup in on_loop_enter.
    psy_loop_set_step(outer, PSY_STEP(inner));
    psy_loop_set_step(inner, PSY_STEP(trial1));

    // We setup the incorrect loops fully at the start.
    // This fails at the second iteration of the outer_incorrect loop
    // as the inner_incorrect loop will still have it's index on 2, so it
    // stops directly.
    PsyLoop *outer_incorrect
        = psy_loop_new_full(INDEX, STOP, INC, PSY_LOOP_CONDITION_LESS);
    PsyLoop *inner_incorrect
        = psy_loop_new_full(INDEX, STOP, INC, PSY_LOOP_CONDITION_LESS);

    psy_loop_set_step(outer_incorrect, PSY_STEP(inner_incorrect));
    // pass a copy of trial as otherwise the first loop will destroy it on
    // destruction
    psy_loop_set_step(inner_incorrect, PSY_STEP(g_object_ref(trial2)));
    ret = psy_loop_set_step(inner_incorrect, PSY_STEP(trial1));
    assert(ret == FALSE);

    // Do something when a trial is activated.
    g_signal_connect(trial1, "activate", G_CALLBACK(on_trial_activate), NULL);
    g_signal_connect(trial2, "activate", G_CALLBACK(on_trial_activate), NULL);

    // Setup the loops.
    g_signal_connect(outer, "enter", G_CALLBACK(on_loop_enter), NULL);
    g_signal_connect(inner, "enter", G_CALLBACK(on_loop_enter), NULL);
    g_signal_connect(
        outer, "leave", G_CALLBACK(on_loop_leave), outer_incorrect);

    g_signal_connect(
        outer_incorrect, "leave", G_CALLBACK(on_second_loop_leave), mainloop);

    psy_step_enter(PSY_STEP(outer), tp1);

    psy_time_point_free(tp1);
    tp1 = psy_clock_now(clk);

    g_main_loop_run(mainloop);

    tp2 = psy_clock_now(clk);
    dur = psy_time_point_subtract(tp2, tp1);
    g_print("Running the loops took %lfs\n", psy_duration_get_seconds(dur));

    psy_duration_free(dur);
    psy_clock_free(clk);
    psy_time_point_free(tp1);
    psy_time_point_free(tp2);
    g_main_loop_unref(mainloop);
}
