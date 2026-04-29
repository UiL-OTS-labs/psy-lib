

#include "psy-init.h"
#include "unit-test-utilities.h"
#include <criterion/criterion.h>
#include <criterion/new/assert.h>
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
    set_log_handler_file("test-wave.txt");

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
        g_critical("Unable to open audio device: %s", error->message);
        cr_log_warn("Unable to open a audio device: %s", error->message);
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
    if (g_device)
        psy_audio_device_close(g_device);

    g_message("%s: g_device refcount = %u",
              __func__,
              ((GObject *) g_device)->ref_count);
    g_object_unref(g_device);
    g_message("%s: g_device refcount = %u",
              __func__,
              ((GObject *) g_device)->ref_count);
    g_device = NULL;

    set_log_handler_file(NULL);
    psy_initializer_free(g_init);
}

TestSuite(wave, .init = wave_setup, .fini = wave_teardown);

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

Test(wave, create)
{
    if (!g_device) {
        cr_log_warn("No audio device: skipping: %s", __func__);
        return;
    }
    PsyWave *tone = psy_wave_new(g_device);
    g_object_set(tone, "num-channels", 2, NULL);

    cr_assert(ne(tone, NULL), "It must be possible to create wave's");

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

    cr_expect(eq(int, wave, PSY_WAVE_FORM_SINE),
              "The default wave form is sine");
    cr_expect(eq(default_freq, 440.0), "The default frequency is 440");
    cr_expect(eq(default_volume, 0.5), "The default volume is .5");
}

Test(wave, set_running)
{
    if (!g_device) {
        cr_log_warn("No audio device: skipping: %s", __func__);
        return;
    }
    PsyWave *tone = psy_wave_new(g_device);
    psy_auditory_stimulus_set_num_channels(PSY_AUDITORY_STIMULUS(tone), 1);
    PsyDuration *dur = psy_duration_new(.5);
    psy_stimulus_set_duration(PSY_STIMULUS(tone), dur);

    gboolean running;

    cr_assert(tone != NULL);

    g_object_get(tone, "running", &running, NULL);
    cr_assert(
        eq(running, FALSE),
        "The tone is not running by default"); // should be initially !running

    g_object_set(tone, "running", TRUE, NULL);
    g_object_get(tone, "running", &running, NULL);
    cr_assert(eq(running, TRUE),
              "The tone should now be running"); // should now be running

    g_object_set(tone, "running", FALSE, NULL);
    g_object_get(tone, "running", &running, NULL);

    cr_assert(eq(running, FALSE),
              "Until we turn it off"); // until we turn it of

    g_object_unref(tone);
    psy_duration_free(dur);
}

Test(wave, play)
{
    if (!g_device) {
        cr_log_warn("No audio device: skipping: %s", __func__);
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

    cr_assert(ne(tone, NULL));

    WaveStatus status = {.loop = loop};

    guint timeout_id = g_timeout_add(500, quit_loop, loop);

    g_signal_connect(tone, "started", G_CALLBACK(wave_started), &status);
    g_signal_connect(tone, "stopped", G_CALLBACK(wave_stopped), &status);

    psy_audio_device_start(g_device, &error);
    cr_expect(eq(error, NULL),
              "No should be no errors opening the audio device");
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

    cr_expect(eq(status.started, TRUE), "We expect the tone has started");
    cr_expect(eq(status.stopped, TRUE), "We expect the tone has stopped");
}

Test(wave, play_noise)
{
    if (!g_device) {
        cr_log_warn("No audio device: skipping: %s", __func__);
        return;
    }
    PsyWave      *tone     = psy_wave_new(g_device);
    GMainLoop    *loop     = g_main_loop_new(NULL, FALSE);
    PsyClock     *clk      = psy_clock_new();
    PsyTimePoint *now      = psy_clock_now(clk);
    PsyDuration  *dur      = psy_duration_new(.250);
    PsyTimePoint *tp_start = psy_time_point_add(now, dur);

    cr_assert(ne(tone, NULL));

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
    cr_expect(zero(error));
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

    cr_expect(status.started, "The tone should have started.");
    cr_expect(status.stopped, "The tone should have stopped.");
}
