
#include "unit-test-utilities.h"

#include <munit.h>
#include <psylib.h>
#include <signal.h>

static int
initializer_setup(void)
{
    install_log_handler();
    set_log_handler_level(G_LOG_LEVEL_DEBUG);
    set_log_handler_file("test-init.txt");
    return 0;
}

static int
initializer_teardown(void)
{
    set_log_handler_file(NULL);
    remove_log_handler();
    return 0;
}

static MunitResult
test_initializer_default(const MunitParameter params[], void *user_data)
{
    (void) params;
    (void) user_data;
    gboolean        gst, portaudio;
    PsyInitializer *initialzer = g_object_new(PSY_TYPE_INITIALIZER, NULL);

    g_object_get(initialzer, "gstreamer", &gst, "portaudio", &portaudio, NULL);

    munit_assert_int(gst, ==, TRUE);
    munit_assert_int(portaudio, ==, TRUE);

    g_object_unref(initialzer);

    return MUNIT_OK;
}

static MunitResult
test_initializer_specific(const MunitParameter params[], void *user_data)
{
    (void) params;
    (void) user_data;
    gboolean gst, portaudio;

    // No need to say we do "need" gstreamer. Initialization is everything
    // unless explictly turned off as portaudio here below.
    PsyInitializer *initialzer
        = g_object_new(PSY_TYPE_INITIALIZER, "portaudio", FALSE, NULL);

    g_object_get(initialzer, "gstreamer", &gst, "portaudio", &portaudio, NULL);

    munit_assert_int(gst, ==, TRUE);
    munit_assert_int(portaudio, ==, FALSE);

    g_object_unref(initialzer);

    return MUNIT_OK;
}

// clang-format off
MunitTest tests[] = {
    {"default",test_initializer_default, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"specific", test_initializer_specific, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {0}
};
// clang-format on

MunitSuite suite = {"initializer/", tests, NULL, 1, MUNIT_SUITE_OPTION_NONE};

static void
signal_handler(int sig)
{
    switch (sig) {
    case SIGINT:
    case SIGABRT:
    case SIGSEGV:
        remove_log_handler();
        g_print("Received signal %d\nquitting\n", sig);
        exit(sig);
    }
}

static void
setup_signal_handlers(void)
{
    if (signal(SIGINT, signal_handler) == SIG_ERR) {
        g_printerr("Unable to catch SIGINT");
    }
    if (signal(SIGABRT, signal_handler) == SIG_ERR) {
        g_printerr("Unable to catch SIGABRT");
    }
    if (signal(SIGSEGV, signal_handler) == SIG_ERR) {
        g_printerr("Unable to catch SIGABRT");
    }
}

int
main(int argc, char **argv)
{
    initializer_setup();
    setup_signal_handlers();
    int ret = munit_suite_main(&suite, NULL, argc, argv);
    initializer_teardown();
    return ret;
}
