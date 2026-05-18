
#include "unit-test-utilities.h"

#include <psylib.h>

static void
test_init(void)
{
    gboolean        gst, portaudio;
    PsyInitializer *initialzer = g_object_new(PSY_TYPE_INITIALIZER, NULL);

    g_object_get(initialzer, "gstreamer", &gst, "portaudio", &portaudio, NULL);

    g_assert_true(gst);
    g_assert_true(portaudio);

    g_object_unref(initialzer);
}

static void
test_init_without_backend(const void *param)
{
    gboolean gst, portaudio;

    const char *backend = param;

    // No need to say we do "need" gstreamer. Initialization is everything
    // unless explictly turned off as portaudio here below.
    PsyInitializer *initialzer
        = g_object_new(PSY_TYPE_INITIALIZER, backend, FALSE, NULL);

    g_object_get(initialzer, "gstreamer", &gst, "portaudio", &portaudio, NULL);

    if (strcmp(backend, "gstreamer") == 0) {
        g_assert_cmpint(portaudio, ==, TRUE);
        g_assert_cmpint(gst, ==, FALSE);
    }
    else if (strcmp(backend, "portaudio") == 0) {
        g_assert_cmpint(portaudio, ==, FALSE);
        g_assert_cmpint(gst, ==, TRUE);
    }

    g_object_unref(initialzer);
}

int
main(int argc, char **argv)
{
    g_test_init(&argc, &argv, NULL);

    g_test_add_func("/init/default", test_init);
    g_test_add_data_func(
        "/init/without-pa", "portaudio", test_init_without_backend);
    g_test_add_data_func(
        "/init/without-gst", "gstreamer", test_init_without_backend);

    return g_test_run();
}
