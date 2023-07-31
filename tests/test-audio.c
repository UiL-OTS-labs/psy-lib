

#include <CUnit/CUnit.h>
#include <psylib.h>

#include "psy-config.h"

#include "unit-test-utilities.h"

typedef PsyAudioDevice *(*audio_backend_allocater_func)(void);
audio_backend_allocater_func g_current_backend_allocater = NULL;

typedef struct AudioBackendAllocater {
    audio_backend_allocater_func alloc;
} AudioBackendAllocater;

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
    const gchar       *def_name = NULL;

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

    def_name = psy_audio_device_get_default_name(device);

    if (PSY_IS_JACK_AUDIO_DEVICE(device)) {
        CU_ASSERT_STRING_EQUAL(def_name, "hw:0");
    }
    else {
        g_warning("missing test for %s\n",
                  G_OBJECT_CLASS_NAME(G_OBJECT_GET_CLASS(device)));
    }

    g_object_unref(device);
}

typedef struct OnStarted {
    GMainLoop *loop;
    gboolean   started;
} OnStarted;

static void
on_started(PsyAudioDevice *device, PsyTimePoint *tp, gpointer data)
{
    (void) device, (void) tp;

    OnStarted *on_started = data;
    on_started->started   = true;

    g_main_loop_quit(on_started->loop);
}

static gboolean
quit_loop(GMainLoop *loop)
{
    g_main_loop_quit(loop);
    return FALSE;
}

static void
audio_device_open(void)
{
    PsyAudioDevice *device = g_current_backend_allocater();
    gboolean        is_open;
    gchar          *name;
    GError         *error = NULL;
    GMainLoop      *loop  = g_main_loop_new(NULL, FALSE);

    OnStarted cb_data = {.loop = loop, .started = FALSE};

    CU_ASSERT_PTR_NOT_NULL_FATAL(device);

    psy_audio_device_open(device, &error);
    CU_ASSERT_PTR_NULL(error);

    // clang-format off
    g_object_get(device,
                 "is-open", &is_open,
                 "name", &name,
                 NULL);
    // clang-format on
    g_signal_connect(device, "started", G_CALLBACK(on_started), &cb_data);
    g_timeout_add(2000, G_SOURCE_FUNC(quit_loop), loop);

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
        g_printerr("The current config of psylib doesn't know about audio "
                   "backend :'%s'.\n",
                   backend);
        return 1;
    }

    if (!suite)
        return 1;

    test = CU_ADD_TEST(suite, audio_device_create);
    if (!test)
        return 1;

    test = CU_ADD_TEST(suite, audio_device_open);
    if (!test)
        return 1;

    return 0;
}
