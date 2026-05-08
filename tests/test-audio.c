#include <psylib.h>

#include "psy-audio-device.h"
#include "unit-test-utilities.h"

typedef enum AudioBackend { BE_PORTAUDIO, BE_ALSA, BE_JACK } AudioBackend;

typedef struct {
    const char  *test_name;
    AudioBackend backend;
} AudioTestParams;

static const char *JACK      = "jack";
static const char *PORTAUDIO = "portaudio";
static const char *ALSA      = "ALSA";

static const char *
backend_to_str(AudioBackend be)
{
    switch (be) {
    case BE_PORTAUDIO:
        return PORTAUDIO;
    case BE_ALSA:
        return ALSA;
    case BE_JACK:
        return JACK;
    default:
        g_assert_not_reached();
        return NULL;
    }
}

typedef PsyAudioDevice *(*audio_backend_allocater_func)(void);

#if defined HAVE_PORTAUDIO
PsyAudioDevice *
alloc_pa_device(void)
{
    return PSY_AUDIO_DEVICE(psy_pa_device_new());
}
#endif

#if defined HAVE_JACK2
PsyAudioDevice *
alloc_jack_device(void)
{
    return psy_jack_audio_device_new();
}
#endif

#if defined HAVE_ALSA
PsyAudioDevice *
alloc_alsa_device(void)
{
    // TODO
    return NULL;
}
#endif

static audio_backend_allocater_func
pick_backend_allocater(AudioBackend be)
{
    audio_backend_allocater_func create_device = NULL;
#ifdef HAVE_PORTAUDIO
    if (be == BE_PORTAUDIO) {
        g_info("Using %s as backend", PORTAUDIO);
        create_device = alloc_pa_device;
    }
#endif

#ifdef HAVE_ALSA
    if (be == BE_ALSA) {
        g_info("alsa backend isn't yet implemented");
        create_device = alloc_alsa_device;
        return NULL;
    }
#endif

#ifdef HAVE_JACK
    if (be == BE_JACK) {
        g_info("jack backend isn't yet implemented, and probably won't");
        create_device = alloc_jack_device();
        return NULL;
    }
#endif

    return create_device;
}

/* ********* setup test parameters ************* */

static void
audio_device_create(const void *data)
{
    const AudioTestParams       *params  = data;
    const char                  *backend = backend_to_str(params->backend);
    audio_backend_allocater_func create_device = NULL;

    g_info("audio_device_create with backend: %s", backend);

    create_device = pick_backend_allocater(params->backend);

    // handles currently unimplemented devices without marking failed test
    if (create_device == NULL) {
        g_test_skip_printf("Skipping %s: as backend %s isn't ready yet",
                           params->test_name,
                           backend);
        return;
    }

    g_assert_nonnull(create_device);

    PsyAudioDevice    *device = create_device();
    gboolean           is_open;
    PsyAudioSampleRate sample_rate;
    gchar             *name;

    g_assert_nonnull(device);

    // clang-format off
    g_object_get(device,
                 "is-open", &is_open,
                 "sample-rate", &sample_rate,
                 "name", &name,
                 NULL);
    // clang-format on

    g_assert_false(is_open);
    g_assert_cmpint(sample_rate, ==, PSY_AUDIO_SAMPLE_RATE_48000);
    g_assert_cmpstr(name, ==, "");

    g_free(name);

    g_object_unref(device);
}

static void
audio_device_enumerate(const void *data)
{
    const AudioTestParams *params = data;

    int         be      = params->backend;
    const char *backend = backend_to_str(be);

    g_info("audio_device_enumerate with backend: %s", backend);

    audio_backend_allocater_func create_device = NULL;

    create_device = pick_backend_allocater(be);
    if (create_device == NULL) {
        g_test_skip_printf("No device for backend %s", backend);
        return;
    }

    PsyAudioDevice *device = create_device();

    PsyAudioDeviceInfo **infos     = NULL;
    guint                num_infos = 0;

    psy_audio_device_enumerate_devices(device, &infos, &num_infos);

    if (num_infos > 0) {
        g_assert_nonnull(infos);

        // free mem
        for (guint i = 0; i < num_infos; i++)
            psy_audio_device_info_free(infos[i]);
        g_free(infos);
    }

    g_object_unref(device);
}

typedef struct OnStarted {
    GMainLoop *loop;
    gboolean   started;
} OnStarted;

typedef struct OnStop {
    GMainLoop      *loop;
    PsyAudioDevice *device;
} OnStop;

static void
on_started(PsyAudioDevice *device, PsyTimePoint *tp, gpointer data)
{
    (void) device, (void) tp;

    OnStarted *on_started = data;
    on_started->started   = true;

    // g_main_loop_quit(on_started->loop);
}

static gboolean
quit_loop(gpointer data)
{
    OnStop *stop_data = data;

    // make sure the audio device is closed before terminating the loop.
    psy_audio_device_close(stop_data->device);
    g_main_loop_quit(stop_data->loop);

    return G_SOURCE_REMOVE;
}

static void
audio_device_open(const void *data)
{
    const AudioTestParams *params = data;

    int         be      = params->backend;
    const char *backend = backend_to_str(be);

    audio_backend_allocater_func create_device = pick_backend_allocater(be);
    if (create_device == NULL) {
        g_test_skip_printf("No device for backend %s", backend);
        return;
    }

    g_info("audio_device_create with backend: %s", backend);

    PsyAudioDevice *device  = create_device();
    gboolean        is_open = FALSE, started = FALSE;
    gchar          *name;
    GError         *error = NULL;
    GMainLoop      *loop  = g_main_loop_new(NULL, FALSE);

    g_info("Device = %p\n", (void *) device);

    OnStarted cb_data   = {.loop = loop, .started = FALSE};
    OnStop    stop_data = {.loop = loop, .device = device};

    g_assert_nonnull(device);

    psy_audio_device_open(device, &error);
    g_assert_null(error);

    // clang-format off
    g_object_get(device,
                 "is-open", &is_open,
                 "started", &started,
                 "name", &name,
                 NULL);
    // clang-format on
    g_signal_connect(device, "started", G_CALLBACK(on_started), &cb_data);
    g_timeout_add(100, G_SOURCE_FUNC(quit_loop), &stop_data);

    g_main_loop_run(loop);
    g_assert_true(is_open);
    g_assert_true(cb_data.started);

    g_assert_cmpstr(name, ==, psy_audio_device_get_default_name(device));

    g_free(name);
    g_clear_error(&error);
    g_object_unref(device);

    g_main_loop_unref(cb_data.loop);
}

int
main(int argc, char **argv)
{

    g_test_init(&argc, &argv, NULL);

    // clang-format off
    AudioTestParams create_tests[] = {
#if defined HAVE_PORTAUDIO
        {
            .test_name = "/audio/create-pa-device",
            .backend   = BE_PORTAUDIO
        },
#endif
#if defined HAVE_JACK
        {
            .test_name = "/audio/create-jack-device",
            .test.backend = BE_JACK },
#endif
#if defined HAVE_ALSA
        {
            .test_name = "/audio/create-alsa-device",
            .backend = BE_ALSA
        },
#endif
    };
    // clang-format on

    for (size_t i = 0; i < G_N_ELEMENTS(create_tests); i++) {
        g_test_add_data_func(
            create_tests[i].test_name, &create_tests[i], audio_device_create);
    }

    // clang-format off
    AudioTestParams enumerate_tests[] = {
#if defined HAVE_PORTAUDIO
        {
            .test_name = "/audio/pa-device-enumerate",
            .backend   = BE_PORTAUDIO
        },
#endif
#if defined HAVE_JACK
        {
            .test_name = "/audio/jack-device-enumerate",
            .backend = BE_JACK,
        },
#endif
#if defined HAVE_ALSA
        {
            .test_name = "/audio/alsa-device-enumerate",
            .backend = BE_ALSA,
        },
#endif
    };
    // clang-format on

    for (size_t i = 0; i < G_N_ELEMENTS(enumerate_tests); i++) {
        g_test_add_data_func(enumerate_tests[i].test_name,
                             &enumerate_tests[i],
                             audio_device_enumerate);
    }

    // clang-format off
    AudioTestParams open_tests[] = {
#if defined HAVE_PORTAUDIO
        {
            .test_name = "/audio/pa-device-open",
            .backend   = BE_PORTAUDIO
        },
#endif
#if defined HAVE_JACK
        {
            .test_name = "/audio/jack-device-open",
            .backend = BE_JACK,
        },
#endif
#if defined HAVE_ALSA
        {
            .test_name = "/audio/alsa-device-open",
            .backend = BE_ALSA,
        },
#endif
    };
    // clang-format on

    for (size_t i = 0; i < G_N_ELEMENTS(enumerate_tests); i++) {
        g_test_add_data_func(
            open_tests[i].test_name, &open_tests[i], audio_device_open);
    }

    return g_test_run();
}
