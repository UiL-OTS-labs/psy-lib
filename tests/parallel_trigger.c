
#include <hw/psy-parallel-trigger.h>
#include <psy-clock.h>

const gchar *g_option_str = "This is a small program to test triggers with a "
                            "PsyParallelTriggerDevice";

gint dur_ms       = 1;
gint num_triggers = 1000;

// clang-format off
static GOptionEntry entries[] = {
    {"duration", 'd', G_OPTION_FLAG_NONE, G_OPTION_ARG_INT, &dur_ms,        "The duration of the trigger", NULL},
    {"num",      'n', G_OPTION_FLAG_NONE, G_OPTION_ARG_INT, &num_triggers,  "The number of triggers",      NULL},
    {0}
};

// clang-format on

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
    (void) tstart;
    PsyDuration  *dur   = psy_duration_new_ms(dur_ms);
    PsyTimePoint *newtp = psy_time_point_add(tfinish, dur);

    TriggerInfo *info = data;

    if (info->n > 1) {
        psy_parallel_trigger_write(trigger, mask, newtp, dur, NULL);
        info->n--;
    }
    else {
        g_main_loop_quit(info->loop);
    }

    psy_duration_free(dur);
    g_object_unref(newtp);
}

int
main(int argc, char **argv)
{
    GError *error = NULL;

    GOptionContext *option_context = g_option_context_new(g_option_str);
    g_option_context_add_main_entries(option_context, entries, NULL);

    if (!g_option_context_parse(option_context, &argc, &argv, &error)) {
        g_printerr("Unable to parse command line options: %s\n",
                   error->message);
        g_error_free(error);
        return EXIT_FAILURE;
    }

    GMainContext *context = g_main_context_new();
    g_main_context_push_thread_default(context);

    PsyTimePoint *now           = NULL;
    PsyDuration  *trigger_dur   = NULL;
    PsyTimePoint *trigger_start = NULL;
    PsyDuration  *onset_dur     = NULL;

    GMainLoop *loop = g_main_loop_new(context, FALSE);
    PsyClock  *clk  = psy_clock_new();

    TriggerInfo info = {.loop = loop, .n = num_triggers};

    PsyParallelTrigger *trigger = psy_parallel_trigger_new();

    g_signal_connect(trigger, "finished", G_CALLBACK(finished), &info);

    psy_parallel_trigger_open(trigger, 0, &error);
    if (error) {
        g_printerr("Unable to open trigger: %s\n", error->message);
        g_error_free(error);
        goto exit;
    }

    now           = psy_clock_now(clk);
    trigger_dur   = psy_duration_new_ms(dur_ms);
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

    if (onset_dur)
        psy_duration_free(onset_dur);
    if (trigger_dur)
        psy_duration_free(trigger_dur);
    if (trigger_start)
        psy_time_point_free(trigger_start);
    if (now)
        psy_time_point_free(now);

    g_object_unref(trigger);

    g_main_loop_unref(loop);

    g_main_context_pop_thread_default(context);
    g_main_context_unref(context);
}
