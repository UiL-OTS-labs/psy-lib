

#include <CUnit/CUnit.h>
#include <psylib.h>

#include "unit-test-utilities.h"

typedef PsyAudioDevice *(*audio_backend_allocater_func)(void);
audio_backend_allocater_func g_current_backend_allocater = NULL;

typedef struct AudioBackendAllocater {
    audio_backend_allocater_func alloc;
} AudioBackendAllocater;

#if defined HAVE_PORTAUDIO
PsyAudioDevice *
alloc_pa_device(void)
{
    return psy_pa_device_new();
}

AudioBackendAllocater pa_allocater = {.alloc = alloc_pa_device};
#endif

#if defined HAVE_JACK2
PsyAudioDevice *
alloc_jack_device(void)
{
    return psy_jack_audio_device_new();
}

AudioBackendAllocater jack_allocater = {.alloc = alloc_jack_device};
#endif

#if defined HAVE_ALSA
PsyAudioDevice *
alloc_alsa_device(void)
{
    #pragma message "Returning NULL isn't nice."
    // return psy_alsa_audio_device_new();
    return NULL;
}

AudioBackendAllocater alsa_allocater = {.alloc = alloc_alsa_device};
#endif

static void
audio_device_create(void)
{
    PsyAudioDevice    *device = g_current_backend_allocater();
    gboolean           is_open;
    PsyAudioSampleRate sample_rate;
    gchar             *name;

    CU_ASSERT_PTR_NOT_NULL_FATAL(device);

    // clang-format off
    g_object_get(device,
                 "is-open", &is_open,
                 "sample-rate", &sample_rate,
                 "name", &name,
                 NULL);
    // clang-format on
    CU_ASSERT_FALSE(is_open);
    CU_ASSERT_EQUAL(sample_rate, PSY_AUDIO_SAMPLE_RATE_48000);
    CU_ASSERT_STRING_EQUAL(name, "");

    g_free(name);

    g_object_unref(device);
}

static void
audio_device_enumerate(void)
{
    PsyAudioDevice *device = g_current_backend_allocater();

    PsyAudioDeviceInfo **infos     = NULL;
    guint                num_infos = 0;

    psy_audio_device_enumerate_devices(device, &infos, &num_infos);

    if (num_infos > 0) {
        CU_ASSERT_PTR_NOT_NULL(infos);

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

    g_main_loop_quit(on_started->loop);
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
audio_device_open(void)
{
    PsyAudioDevice *device  = g_current_backend_allocater();
    gboolean        is_open = FALSE, started = FALSE;
    gchar          *name;
    GError         *error = NULL;
    GMainLoop      *loop  = g_main_loop_new(NULL, FALSE);

    g_print("Device = %p\n", (void *) device);

    OnStarted cb_data   = {.loop = loop, .started = FALSE};
    OnStop    stop_data = {.loop = loop, .device = device};

    CU_ASSERT_PTR_NOT_NULL_FATAL(device);

    psy_audio_device_open(device, &error);
    CU_ASSERT_PTR_NULL(error);

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
    CU_ASSERT_TRUE(is_open);
    CU_ASSERT_TRUE(cb_data.started);
    CU_ASSERT_STRING_EQUAL(name, psy_audio_device_get_default_name(device));

    g_free(name);
    g_clear_error(&error);
    g_object_unref(device);

    g_main_loop_unref(cb_data.loop);
}

int
add_audio_suite(const gchar *backend)
{
    CU_Suite *suite = CU_add_suite("audio tests", NULL, NULL);
    CU_Test  *test  = NULL;

    GHashTable *backend_table = NULL;

    backend_table = g_hash_table_new(g_str_hash, g_str_equal);

#if defined HAVE_PORTAUDIO
    g_hash_table_insert(backend_table, "portaudio", &pa_allocater);
#endif
#if defined HAVE_JACK2
    g_hash_table_insert(backend_table, "jack", &jack_allocater);
#endif
#if defined HAVE_ALSA
    g_hash_table_insert(backend_table, "alsa", &alsa_allocater);
#endif

    if (g_hash_table_contains(backend_table, backend)) {
        AudioBackendAllocater *allocater
            = g_hash_table_lookup(backend_table, backend);
        g_current_backend_allocater = allocater->alloc;
    }
    else {
        g_printerr(
            "%s:%d: The current config of psylib doesn't know about audio "
            "backend :'%s', and the test are trying to use this backend.\n",
            __FILE__,
            __LINE__,
            backend);
        return 1;
    }

    if (!suite)
        return 1;

    test = CU_ADD_TEST(suite, audio_device_create);
    if (!test)
        return 1;

    test = CU_ADD_TEST(suite, audio_device_enumerate);
    if (!test)
        return 1;

    test = CU_ADD_TEST(suite, audio_device_open);
    if (!test)
        return 1;

    return 0;
}
