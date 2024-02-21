
#include <psylib.h>

static void
on_trial_activate(PsyStep *self, PsyTimePoint *tp)
{
    (void) self;
    (void) tp;
    psy_step_leave(self, tp);
}

void
on_loop_leave(PsyStep *step, PsyTimePoint *tp, gpointer data)
{
    (void) step;
    (void) tp;
    GMainLoop *loop = data;
    g_main_loop_quit(loop);
}

int
main(void)
{
    GMainLoop *mainloop = g_main_loop_new(NULL, FALSE);

    PsyClock     *clk = psy_clock_new();
    PsyTimePoint *tp  = psy_clock_now(clk);

    PsyTrial *trial = psy_trial_new();
    g_signal_connect(trial, "activate", G_CALLBACK(on_trial_activate), NULL);

    PsyLoop *loop = psy_loop_new_full(0, 10, 1, PSY_LOOP_CONDITION_LESS);
    g_signal_connect(loop, "leave", G_CALLBACK(on_loop_leave), mainloop);
    // psy_loop_set_child(loop, PSY_STEP(trial));
    g_object_set(loop, "child", trial, NULL);

    psy_step_enter(PSY_STEP(loop), tp);

    g_main_loop_run(mainloop);

    //g_object_unref(loop);
    g_object_unref(clk);
    psy_time_point_free(tp);
}
