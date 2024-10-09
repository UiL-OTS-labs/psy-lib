
#include <psylib.h>
#if __WIN32
    #include <windows.h>
#endif

gint time_resolution = 0;

// clang-format off
GOptionEntry options[] = {
#if _WIN32
    {"begin-period", 'b', G_OPTION_FLAG_NONE, G_OPTION_ARG_INT, &time_resolution, "(windows only)Specify a value for timeBeginPeriod [0-20] 0 == off", "N"},
#endif
    {0}
};
// clang-format on

typedef struct TestData {
    GMainLoop *loop;
    gboolean   one_has_fired;
    PsyClock  *clk;
} TestData;

void
fire_one(PsyTimer *timer, PsyTimePoint *tp_fire, gpointer data)
{
    (void) timer;
    TestData *tdata      = data;
    tdata->one_has_fired = TRUE;

    PsyTimePoint *tp_now = psy_clock_now(tdata->clk);
    PsyDuration  *dur    = psy_time_point_subtract(tp_now, tp_fire);

    g_info("Diff between firetime and measured = %lf",
           psy_duration_get_seconds(dur));

    psy_time_point_free(tp_now);
    psy_duration_free(dur);
}

void
fire_two(PsyTimer *timer, PsyTimePoint *tp_fire, gpointer data)
{
    (void) timer;
    TestData *tdata = data;
    g_assert(tdata->one_has_fired);

    PsyTimePoint *tp_now = psy_clock_now(tdata->clk);
    PsyDuration  *dur    = psy_time_point_subtract(tp_now, tp_fire);

    g_info("Diff between firetime and measured = %lf",
           psy_duration_get_seconds(dur));

    psy_time_point_free(tp_now);
    psy_duration_free(dur);

    g_main_loop_quit(tdata->loop);
}

void
on_fire(PsyTimer *timer, PsyTimePoint *fire_time, gpointer data)
{
    (void) timer;
    PsyClock     *clk    = psy_clock_new();
    PsyTimePoint *tp_now = psy_clock_now(clk);

    PsyDuration *dur = psy_time_point_subtract(tp_now, fire_time);
    g_debug("Diff between firetime and measured = %lf",
            psy_duration_get_seconds(dur));

    psy_duration_free(dur);
    psy_time_point_free(tp_now);
    psy_clock_free(clk);
    psy_time_point_free(data);
}

int
main(int argc, char **argv)
{
    GError         *error = NULL;
    GOptionContext *opts  = g_option_context_new("timer options");
    g_option_context_add_main_entries(opts, options, NULL);

    if (!g_option_context_parse(opts, &argc, &argv, &error)) {
        g_printerr("Oops unable to parse options: %s\n", error->message);
        g_option_context_free(opts);
        return EXIT_FAILURE;
    }
    g_option_context_free(opts);

#if _WIN32
    if (time_resolution > 0 && time_resolution <= 20) {
        g_info("Running with a resolution of: %dms", time_resolution);
        timeBeginPeriod(time_resolution);
    }
    else {
        g_info("Running with a default resolution");
    }
#endif

    TestData     tdata = {0x0, FALSE, 0x0};
    PsyTimer    *t1, *t2;
    PsyDuration *one_sec = psy_duration_new_ms(1000);
    PsyDuration *one     = psy_duration_new_ms(5);

    tdata.clk = psy_clock_new();

    t1 = psy_timer_new();
    t2 = psy_timer_new();

    PsyTimePoint *tp_now = psy_clock_now(tdata.clk);
    PsyTimePoint *tp1    = psy_time_point_add(tp_now, one_sec);
    PsyTimePoint *tp2    = psy_time_point_add(tp1, one_sec);

    psy_timer_set_fire_time(t1, tp1);
    psy_timer_set_fire_time(t2, tp2);

    GPtrArray    *timers  = g_ptr_array_new_full(200, g_object_unref);
    PsyTimePoint *running = NULL;

    for (int i = 0; i < 200; i++) {
        PsyTimer *t = psy_timer_new();
        g_ptr_array_add(timers, t);
        if (running) {
            running = psy_time_point_add(running, one);
        }
        else {
            running = psy_time_point_add(tp_now, one);
        }
        psy_timer_set_fire_time(t, running);
        g_signal_connect(t, "fired", G_CALLBACK(on_fire), running);
    }

    tdata.loop = g_main_loop_new(NULL, false);

    g_signal_connect(t1, "fired", G_CALLBACK(fire_one), &tdata);
    g_signal_connect(t2, "fired", G_CALLBACK(fire_two), &tdata);

    g_main_loop_run(tdata.loop);

    g_main_loop_unref(tdata.loop);

    g_ptr_array_unref(timers);

    psy_duration_free(one_sec);
    psy_duration_free(one);

    psy_clock_free(tdata.clk);

    psy_time_point_free(tp_now);
    psy_time_point_free(tp1);
    psy_time_point_free(tp2);

    g_clear_object(&t1);
    g_clear_object(&t2);

#if _WIN32
    if (time_resolution > 0 && time_resolution <= 20)
        timeEndPeriod(time_resolution);
#endif
    return 0;
}
