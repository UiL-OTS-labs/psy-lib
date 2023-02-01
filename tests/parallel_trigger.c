
#include <hw/psy-parallel-trigger.h>
#include <psy-clock.h>

gint dur_us = 1000;

typedef struct TriggerInfo {
    GMainLoop *loop;
    guint      n;
} TriggerInfo;

void
finished(PsyParallelTrigger *trigger,
         guint               mask,
         PsyTimePoint       *tstart,
         PsyTimePoint       *tfinish,
         gpointer            data)
{
    PsyDuration  *dur   = psy_duration_new_us(dur_us);
    PsyTimePoint *newtp = psy_time_point_add(tfinish, dur);

    TriggerInfo *info = data;

    if (info->n > 0) {
        psy_parallel_trigger_write(trigger, mask, newtp, dur, NULL);
        info->n--;
    }
    else {
        g_main_loop_quit(info->loop);
    }

    g_object_unref(dur);
    g_object_unref(newtp);
}

int
main()
{
    GMainContext *context = g_main_context_new();
    g_main_context_push_thread_default(context);

    PsyTimePoint *now           = NULL;
    PsyDuration  *trigger_dur   = NULL;
    PsyTimePoint *trigger_start = NULL;
    PsyDuration  *onset_dur     = NULL;

    GMainLoop *loop = g_main_loop_new(context, FALSE);
    PsyClock  *clk  = psy_clock_new();

    TriggerInfo info = {.loop = loop, .n = 1000};

    PsyParallelTrigger *trigger = psy_parallel_trigger_new();
    GError             *error   = NULL;

    g_signal_connect(trigger, "finished", G_CALLBACK(finished), &info);

    psy_parallel_trigger_open(trigger, 0, &error);
    if (error) {
        g_printerr("Unable to open trigger: %s\n", error->message);
        g_error_free(error);
        goto exit;
    }

    now           = psy_clock_now(clk);
    trigger_dur   = psy_duration_new_us(dur_us);
    onset_dur     = psy_duration_new_ms(5);
    trigger_start = psy_time_point_add(now, onset_dur);

    psy_parallel_trigger_write(
        trigger, 255, trigger_start, trigger_dur, &error);

    if (error) {
        g_printerr("Unable to write trigger: %s\n", error->message);
        g_error_free(error);
        goto exit;
    }

    g_main_loop_run(loop);

exit:

    g_object_unref(clk);

    g_object_unref(onset_dur);
    g_object_unref(trigger_dur);
    g_object_unref(trigger_start);
    g_object_unref(now);

    g_object_unref(trigger);

    g_main_loop_unref(loop);

    g_main_context_pop_thread_default(context);
    g_main_context_unref(context);
}
