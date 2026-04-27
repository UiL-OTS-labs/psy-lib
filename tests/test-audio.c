

#include <criterion/criterion.h>
#include <criterion/logging.h>
#include <criterion/new/assert.h>
#include <criterion/parameterized.h>

#include <psylib.h>

#include "psy-audio-device.h"
#include "unit-test-utilities.h"

static const char *JACK      = "jack";
static const char *PORTAUDIO = "portaudio";
static const char *ALSA      = "ALSA";

typedef enum AudioBackend { BE_PORTAUDIO, BE_ALSA, BE_JACK } AudioBackend;

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
        cr_log_info("Using %s as backend", PORTAUDIO);
        create_device = alloc_pa_device;
    }
#endif

#ifdef HAVE_ALSA
    if (be == BE_ALSA) {
        cr_log_info("alsa backend isn't yet implemented");
        create_device = alloc_alsa_device;
        return NULL;
    }
#endif

#ifdef HAVE_JACK
    if (be == BE_JACK) {
        cr_log_info("jack backend isn't yet implemented, and probably won't");
        create_device = alloc_jack_device();
        return NULL;
    }
#endif

    return create_device;
}

/* ********* setup test parameters ************* */

static struct criterion_test_params
create_default_params(void)
{
    static const int backends[] = {
#ifdef HAVE_PORTAUDIO
        BE_PORTAUDIO,
#endif
#ifdef HAVE_JACK2
        BE_JACK,
#endif
#ifdef HAVE_ALSA
        BE_ALSA,
#endif
    };

    size_t num_params = sizeof(backends) / sizeof(backends[0]);
    return cr_make_param_array(int, backends, num_params);
}

ParameterizedTestParameters(audio, device_create)
{
    return create_default_params();
}

ParameterizedTestParameters(audio, device_enumerate)
{
    return create_default_params();
}

ParameterizedTestParameters(audio, device_open)
{
    return create_default_params();
}

ParameterizedTest(int *param, audio, device_create)
{
    int                          be            = *param;
    const char                  *backend       = backend_to_str(be);
    audio_backend_allocater_func create_device = NULL;

    g_info("audio_device_create with backend: %s", backend);

    create_device = pick_backend_allocater(be);

    // handles currently unimplemented devices without marking failed test
    if (create_device == NULL)
        return;

    cr_assert(create_device != NULL);

    PsyAudioDevice    *device = create_device();
    gboolean           is_open;
    PsyAudioSampleRate sample_rate;
    gchar             *name;

    cr_assert(ne(device, NULL),
              "It should be possible to instantiate an audio device");

    // clang-format off
    g_object_get(device,
                 "is-open", &is_open,
                 "sample-rate", &sample_rate,
                 "name", &name,
                 NULL);
    // clang-format on
    cr_assert(not(is_open), "Created device should not be opened");
    cr_assert(eq(int, sample_rate, PSY_AUDIO_SAMPLE_RATE_48000),
              "48000 is the default sample rate");
    cr_assert(eq(str, name, ""), "Unopened device don't have a name");

    g_free(name);

    g_object_unref(device);
}

ParameterizedTest(int *param, audio, device_enumerate)
{
    int         be      = *param;
    const char *backend = backend_to_str(be);

    g_info("audio_device_enumerate with backend: %s", backend);

    audio_backend_allocater_func create_device = NULL;

    create_device = pick_backend_allocater(be);
    if (create_device == NULL) {
        cr_log_info("No device for backend %s", backend);
        return;
    }

    PsyAudioDevice *device = create_device();

    PsyAudioDeviceInfo **infos     = NULL;
    guint                num_infos = 0;

    psy_audio_device_enumerate_devices(device, &infos, &num_infos);

    if (num_infos > 0) {
        cr_expect(ne(infos, NULL),
                  "The backend should be able to enumerate some devices");

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

ParameterizedTest(int *param, audio, device_open)
{
    int         be      = *param;
    const char *backend = backend_to_str(be);

    audio_backend_allocater_func create_device = pick_backend_allocater(be);
    if (create_device == NULL) {
        cr_log_info("No device for backend %s", backend);
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

    cr_assert(ne(device, NULL), "The device must be created.");

    psy_audio_device_open(device, &error);
    cr_expect(zero(error));

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
    cr_expect(is_open, "The device should now be open");
    cr_expect(cb_data.started, "The device should now be started");

    cr_expect(eq(str,
                 (char *) name,
                 (char *) psy_audio_device_get_default_name(device)),
              "The device should have opened the default device");

    g_free(name);
    g_clear_error(&error);
    g_object_unref(device);

    g_main_loop_unref(cb_data.loop);
}

// int
// add_audio_suite(const gchar *backend)
// {
//     CU_Suite *suite = CU_add_suite("audio tests", NULL, NULL);
//     CU_Test  *test  = NULL;
//
//     GHashTable *backend_table = NULL;
//
//     backend_table = g_hash_table_new(g_str_hash, g_str_equal);
//
// #if defined HAVE_PORTAUDIO
//     g_hash_table_insert(backend_table, "portaudio", &pa_allocater);
// #endif
// #if defined HAVE_JACK2
//     g_hash_table_insert(backend_table, "jack", &jack_allocater);
// #endif
// #if defined HAVE_ALSA
//     g_hash_table_insert(backend_table, "alsa", &alsa_allocater);
// #endif
//
//     if (g_hash_table_contains(backend_table, backend)) {
//         AudioBackendAllocater *allocater
//             = g_hash_table_lookup(backend_table, backend);
//         g_current_backend_allocater = allocater->alloc;
//     }
//     else {
//         g_printerr(
//             "%s:%d: The current config of psylib doesn't know about audio "
//             "backend :'%s', and the test are trying to use this backend.\n",
//             __FILE__,
//             __LINE__,
//             backend);
//         return 1;
//     }
//
//     if (!suite)
//         return 1;
//
//     //    test = CU_ADD_TEST(suite, audio_device_create);
//     //    if (!test)
//     //        return 1;
//     //
//     //    test = CU_ADD_TEST(suite, audio_device_enumerate);
//     //    if (!test)
//     //        return 1;
//     //
//     //    test = CU_ADD_TEST(suite, audio_device_open);
//     //    if (!test)
//     //        return 1;
//
//     return 0;
// }
