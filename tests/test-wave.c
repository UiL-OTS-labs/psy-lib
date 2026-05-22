#include "unit-test-utilities.h"
#include <psylib.h>

PsyAudioDevice *g_device = NULL;

typedef struct {
    GMainLoop *loop;
    gboolean   started;
    gboolean   stopped;
} WaveStatus;

static PsyInitializer *g_init = NULL;

static void
wave_setup(void)
{
    g_init = psy_initializer_new();

    g_device         = psy_audio_device_new();
    gchar  *dev_name = NULL;
    GError *error    = NULL;
    // clang-format off
    g_object_set(g_device,
            "num-output-channels", 2,
//            "num-input-channels", 1,
            "sample-rate", PSY_AUDIO_SAMPLE_RATE_44100,
            NULL);
    // clang-format on

    psy_audio_device_open(g_device, &error);
    if (error) {
        g_message("Unable to open audio device: %s", error->message);
        g_clear_object(&g_device);
        return;
    }

    g_object_get(g_device, "name", &dev_name, NULL);
    g_message("test-wave uses audio device: %s", dev_name);
    g_free(dev_name);

    // The tests that are playing audio should start the device itself
    psy_audio_device_stop(g_device);
}

static void
wave_teardown(void)
{
    if (g_device) {
        psy_audio_device_close(g_device);
        g_object_unref(g_device);
    }
    g_device = NULL;

    psy_initializer_free(g_init);
}

static gboolean
quit_loop(gpointer data)
{
    g_warning("Closing loop in timeout %p", data);
    GMainLoop *loop = data;

    g_main_loop_quit(loop);

    return G_SOURCE_REMOVE;
}

static void
wave_started(PsyStimulus *self, PsyTimePoint *tp, gpointer data)
{
    (void) self;
    (void) tp;
    g_info("Callback wave started, stimulus = %p, tp %p data = %p",
           (void *) self,
           (void *) tp,
           data);
    WaveStatus *status = data;
    status->started    = TRUE;
}

static void
wave_stopped(PsyStimulus *self, PsyTimePoint *tp, gpointer data)
{
    (void) self;
    (void) tp;
    g_info("Callback wave started, stimulus = %p, tp %p data = %p",
           (void *) self,
           (void *) tp,
           data);
    WaveStatus *status = data;
    status->stopped    = TRUE;

    PsyTimePoint *start = psy_stimulus_get_start_time(self);
    PsyDuration  *dur   = psy_time_point_subtract(tp, start);

    g_info("Wave duration was: %lf seconds", psy_duration_get_seconds(dur));

    psy_duration_free(dur);

    g_main_loop_quit(status->loop);
}

static void
test_wave_create(void)
{
    if (!g_device) {
        g_test_skip_printf("No audio device: skipping: %s", __func__);
        return;
    }
    PsyWave *tone = psy_wave_new(g_device);
    g_object_set(tone, "num-channels", 2, NULL);

    g_assert_nonnull(tone);

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

    g_assert_cmpint(wave, ==, PSY_WAVE_FORM_SINE);
    g_assert_cmpfloat(default_freq, ==, 440.0);
    g_assert_cmpfloat(default_volume, ==, 0.5);
}

static void
test_wave_set_running(void)
{
    if (!g_device) {
        g_test_skip_printf("No audio device: skipping: %s", __func__);
        return;
    }
    PsyWave *tone = psy_wave_new(g_device);
    psy_auditory_stimulus_set_num_channels(PSY_AUDITORY_STIMULUS(tone), 1);
    PsyDuration *dur = psy_duration_new(.5);
    psy_stimulus_set_duration(PSY_STIMULUS(tone), dur);

    gboolean running;

    g_assert_nonnull(tone);

    g_object_get(tone, "running", &running, NULL);
    g_assert_false(running);

    g_object_set(tone, "running", TRUE, NULL);
    g_object_get(tone, "running", &running, NULL);
    g_assert_true(running); // should now be running

    g_object_set(tone, "running", FALSE, NULL);
    g_object_get(tone, "running", &running, NULL);

    g_assert_false(running); // until we turn it of

    g_object_unref(tone);
    psy_duration_free(dur);
}

static void
test_wave_play(void)
{
    if (!g_device) {
        g_test_skip_printf("No audio device: skipping: %s", __func__);
        return;
    }
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

    g_assert_nonnull(tone);

    WaveStatus status = {.loop = loop};

    guint timeout_id = g_timeout_add(500, quit_loop, loop);

    g_signal_connect(tone, "started", G_CALLBACK(wave_started), &status);
    g_signal_connect(tone, "stopped", G_CALLBACK(wave_stopped), &status);

    psy_audio_device_start(g_device, &error);
    g_assert_no_error(error);
    if (error) {
        g_error("Unable to start the audio device: %s\n", error->message);
        g_clear_error(&error);
    }

    psy_stimulus_play_for(PSY_STIMULUS(tone), tp_start, dur);

    g_main_loop_run(loop);

    g_source_remove(timeout_id);

    psy_audio_device_stop(g_device);

    psy_time_point_free(tp_start);
    psy_time_point_free(now);
    psy_duration_free(dur);
    g_object_unref(clk);
    g_main_loop_unref(loop);

    g_message("tone refcount = %u", ((GObject *) tone)->ref_count);
    g_object_unref(tone);

    g_assert_true(status.started); // We expect the tone has started
    g_assert_true(status.stopped); // We expect the tone has stopped
}

static void
test_wave_play_noise(void)
{
    if (!g_device) {
        g_test_skip_printf("No audio device: skipping: %s", __func__);
        return;
    }
    PsyWave      *tone     = psy_wave_new(g_device);
    GMainLoop    *loop     = g_main_loop_new(NULL, FALSE);
    PsyClock     *clk      = psy_clock_new();
    PsyTimePoint *now      = psy_clock_now(clk);
    PsyDuration  *dur      = psy_duration_new(.250);
    PsyTimePoint *tp_start = psy_time_point_add(now, dur);

    g_assert_nonnull(tone);

    PsyWaveForm wave = PSY_WAVE_FORM_WHITE_UNIFORM_NOISE;

    GError *error = NULL;

    // clang-format off
    g_object_set(tone,
            "num-channels", 1, 
            "duration", dur,
            "wave-form", wave,
            NULL);
    // clang-format on

    psy_gst_stimulus_set_running(PSY_GST_STIMULUS(tone), TRUE);

    WaveStatus status = {.loop = loop};

    guint timeout_id = g_timeout_add(500, quit_loop, loop);

    g_signal_connect(tone, "started", G_CALLBACK(wave_started), &status);
    g_signal_connect(tone, "stopped", G_CALLBACK(wave_stopped), &status);

    psy_audio_device_start(g_device, &error);
    g_assert_no_error(error);
    if (error) {
        g_error("Unable to start the audio device: %s\n", error->message);
        g_clear_error(&error);
    }

    psy_stimulus_play_for(PSY_STIMULUS(tone), tp_start, dur);

    g_main_loop_run(loop);

    g_source_remove(timeout_id);

    psy_audio_device_stop(g_device);

    psy_time_point_free(tp_start);
    psy_time_point_free(now);
    psy_duration_free(dur);

    g_object_unref(clk);
    g_main_loop_unref(loop);

    g_object_unref(tone);

    g_assert_true(status.started); //"The tone should have started.
    g_assert_true(status.stopped); //"The tone should have stopped.
}

int
main(int argc, char **argv)
{
    g_test_init(&argc, &argv, NULL);

    UnitTestUtilsInit init_utils = {.log_file      = "test-wave.txt",
                                    .domains       = NULL,
                                    .log_level     = G_LOG_LEVEL_INFO,
                                    .save_pictures = TRUE};

    unit_test_utils_init(&init_utils);

    wave_setup();

    g_test_add_func("/wave/create", test_wave_create);
    g_test_add_func("/wave/set_running", test_wave_set_running);
    g_test_add_func("/wave/play", test_wave_play);
    g_test_add_func("/wave/play_noise", test_wave_play_noise);

    int ret = g_test_run();

    wave_teardown();

    return ret;
}
