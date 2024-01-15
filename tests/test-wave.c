

#include <CUnit/CUnit.h>
#include <psylib.h>

PsyAudioDevice *g_device = NULL;

// Useful to put a breakpoint on to get a backtrace to the origin of the error
static void
wave_log_handler(const gchar   *log_domain,
                 GLogLevelFlags log_level,
                 const gchar   *message,
                 gpointer       data)
{
    (void) data;
    if (g_strcmp0(log_domain, "Psy") == 0) {
        volatile int a = 1;
        (void) a;
    }
    else if (g_strcmp0(log_domain, "GObject") == 0) {
        volatile int b = 2;
        (void) b;
    }
    else {
        volatile int c = 3;
        (void) c;
    }
    // Pass info to the default log handler
    g_log_default_handler(log_domain, log_level, message, data);
}

typedef struct {
    GMainLoop *loop;
    gboolean   started;
    gboolean   stopped;
} WaveStatus;

static int
create_audio_device(void)
{
    g_log_set_default_handler(wave_log_handler, NULL);
    g_device = psy_audio_device_new();
    g_print("%s: g_device refcount = %u\n",
            __func__,
            ((GObject *) g_device)->ref_count);
    GError *error = NULL;
    // clang-format off
    g_object_set(g_device,
            "num-output-channels", 2,
//            "num-input-channels", 1,
            "sample-rate", PSY_AUDIO_SAMPLE_RATE_44100,
            NULL);
    // clang-format on
    psy_audio_device_open(g_device, &error);

    if (error) {
        g_critical("Unable to open audio device: %s", error->message);
        return 1;
    }

    // The tests that are playing audio should start the device itself
    psy_audio_device_stop(g_device);
    return 0;
}

static int
destroy_audio_device(void)
{
    psy_audio_device_close(g_device);
    g_print("%s: g_device refcount = %u\n",
            __func__,
            ((GObject *) g_device)->ref_count);
    g_object_unref(g_device);
    g_print("%s: g_device refcount = %u\n",
            __func__,
            ((GObject *) g_device)->ref_count);
    g_device = NULL;
    g_log_set_default_handler(g_log_default_handler, NULL);
    return 0;
}

static gboolean
quit_loop(gpointer data)
{
    GMainLoop *loop = data;

    g_main_loop_quit(loop);

    return G_SOURCE_REMOVE;
}

static void
wave_started(PsyStimulus *self, PsyTimePoint *tp, gpointer data)
{
    (void) self;
    (void) tp;
    WaveStatus *status = data;
    status->started    = TRUE;
}

static void
wave_stopped(PsyStimulus *self, PsyTimePoint *tp, gpointer data)
{
    (void) self;
    (void) tp;
    WaveStatus *status = data;
    status->stopped    = TRUE;

    PsyTimePoint *start = psy_stimulus_get_start_time(self);
    PsyDuration  *dur   = psy_time_point_subtract(tp, start);

    g_print("Wave duration was: %lf seconds", psy_duration_get_seconds(dur));

    g_object_unref(dur);

    g_main_loop_quit(status->loop);
}

static void
test_wave_create(void)
{
    PsyWave *tone = psy_wave_new(g_device);
    g_object_set(tone, "num-channels", 2, NULL);

    CU_ASSERT_PTR_NOT_NULL_FATAL(tone);

    gdouble     default_volume, default_freq;
    PsyWaveForm wave;

    // clang-format off
    g_object_get(
            tone,
            "volume", &default_volume,
            "wave-form", &wave,
            "freq", &default_freq,
            NULL);
    // clang-format on

    g_object_unref(tone);

    CU_ASSERT_EQUAL(wave, PSY_WAVE_FORM_SINE);
    CU_ASSERT_EQUAL(default_freq, 440.0);
    CU_ASSERT_EQUAL(default_volume, 0.5);
}

static void
test_wave_set_running(void)
{
    PsyWave *tone = psy_wave_new(g_device);
    psy_auditory_stimulus_set_num_channels(PSY_AUDITORY_STIMULUS(tone), 1);
    psy_stimulus_set_duration(PSY_STIMULUS(tone), psy_duration_new(.5));

    gboolean running;

    CU_ASSERT_PTR_NOT_NULL_FATAL(tone);

    g_object_get(tone, "running", &running, NULL);
    CU_ASSERT_EQUAL(running, FALSE); // should be initially !running

    g_object_set(tone, "running", TRUE, NULL);
    g_object_get(tone, "running", &running, NULL);
    CU_ASSERT_EQUAL(running, TRUE); // should now be running

    g_object_set(tone, "running", FALSE, NULL);
    g_object_get(tone, "running", &running, NULL);
    CU_ASSERT_EQUAL(running, FALSE); // until we turn it of

    g_object_unref(tone);
}

static void
test_wave_play(void)
{
    PsyWave      *tone     = psy_wave_new(g_device);
    GMainLoop    *loop     = g_main_loop_new(NULL, FALSE);
    PsyClock     *clk      = psy_clock_new();
    PsyTimePoint *now      = psy_clock_now(clk);
    PsyDuration  *dur      = psy_duration_new(.250);
    PsyTimePoint *tp_start = psy_time_point_add(now, dur);

    GError *error = NULL;

    g_object_set(tone, "num-channels", 2, NULL);
    g_object_set(tone, "duration", dur, NULL);
    g_object_set(tone, "running", TRUE, NULL);

    CU_ASSERT_PTR_NOT_NULL_FATAL(tone);

    WaveStatus status = {.loop = loop};

    g_timeout_add(2000, quit_loop, loop);

    g_signal_connect(tone, "started", G_CALLBACK(wave_started), &status);
    g_signal_connect(tone, "stopped", G_CALLBACK(wave_stopped), &status);

    psy_audio_device_start(g_device, &error);
    CU_ASSERT_PTR_NULL(error);
    if (error) {
        g_printerr("Unable to start the audio device: %s\n", error->message);
        g_clear_error(&error);
    }

    psy_stimulus_play_for(PSY_STIMULUS(tone), tp_start, dur);

    g_main_loop_run(loop);

    psy_audio_device_stop(g_device);

    g_object_unref(tp_start);
    g_object_unref(dur);
    g_object_unref(now);
    g_object_unref(clk);
    g_main_loop_unref(loop);

    g_print("tone refcount = %u", ((GObject *) tone)->ref_count);
    g_object_unref(tone);

    CU_ASSERT_TRUE(status.started);
    CU_ASSERT_TRUE(status.stopped);
}

int
add_wave_suite(void)
{
    CU_Suite *suite = CU_add_suite(
        "audio tests", create_audio_device, destroy_audio_device);
    CU_Test *test = NULL;

    if (!suite)
        return 1;

    test = CU_ADD_TEST(suite, test_wave_create);
    if (!test)
        return 1;

    test = CU_ADD_TEST(suite, test_wave_set_running);
    if (!test)
        return 1;

    test = CU_ADD_TEST(suite, test_wave_play);
    if (!test)
        return 1;

    return 0;
}
