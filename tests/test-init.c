
#include "unit-test-utilities.h"

#include <criterion/criterion.h>
#include <criterion/new/assert.h>
#include <criterion/parameterized.h>
#include <psylib.h>

static void
initializer_setup(void)
{
    install_log_handler();
    set_log_handler_level(G_LOG_LEVEL_DEBUG);
    set_log_handler_file("test-init.txt");
}

static void
initializer_teardown(void)
{
    set_log_handler_file(NULL);
    remove_log_handler();
}

Test(init,
     init_default,
     .init = initializer_setup,
     .fini = initializer_teardown)
{
    gboolean        gst, portaudio;
    PsyInitializer *initialzer = g_object_new(PSY_TYPE_INITIALIZER, NULL);

    g_object_get(initialzer, "gstreamer", &gst, "portaudio", &portaudio, NULL);

    cr_assert(eq(int, gst, TRUE), "gstreamer should be initialized by default");
    cr_assert(eq(int, portaudio, TRUE),
              "portaudio should be initialized by default");

    g_object_unref(initialzer);
}

static void
cr_free_strings(struct criterion_test_params *crp)
{
    char **strings = crp->params;
    for (size_t i = 0; i < crp->length; i++) {
        cr_free(strings[i]);
    }
    cr_free(strings);
}

static char *
cr_strdup(const char *string)
{
    char *ret = cr_calloc(strlen(string) + 1, sizeof(string[0]));
    ret       = strcpy(ret, string);
    return ret;
}

ParameterizedTestParameters(init, init_specific)
{
    const char  *params[] = {"portaudio", "gstreamer"};
    const size_t n_params = sizeof(params) / sizeof(params[0]);
    char       **strings  = cr_malloc(n_params * sizeof(char *));

    for (size_t i = 0; i < n_params; i++) {
        strings[i] = cr_strdup(params[i]);
    }

    return cr_make_param_array(
        char *, strings, sizeof(params) / sizeof(params[0]), cr_free_strings);
}

ParameterizedTest(const char **param,
                  init,
                  init_specific,
                  .init = initializer_setup,
                  .fini = initializer_teardown)
{
    gboolean gst, portaudio;

    const char *backend = *param;

    // No need to say we do "need" gstreamer. Initialization is everything
    // unless explictly turned off as portaudio here below.
    PsyInitializer *initialzer
        = g_object_new(PSY_TYPE_INITIALIZER, backend, FALSE, NULL);

    g_object_get(initialzer, "gstreamer", &gst, "portaudio", &portaudio, NULL);

    if (strcmp(backend, "gstreamer") == 0) {
        cr_expect(eq(int, portaudio, TRUE), "Port audio should be on");
        cr_assert(eq(int, gst, FALSE), "Gstreamer should be off");
    }
    else if (strcmp(backend, "portaudio") == 0) {
        cr_expect(eq(int, portaudio, FALSE), "Port audio should be off");
        cr_assert(eq(int, gst, TRUE), "Gstreamer should be on");
    }

    g_object_unref(initialzer);
}
