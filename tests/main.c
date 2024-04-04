
#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>
#include <CUnit/TestRun.h>
#include <glib.h>
#include <gst/gst.h>
#include <portaudio.h>
#include <psylib.h>
#include <signal.h>
#include <stdlib.h>

#include "suites.h"
#include "unit-test-utilities.h"

static gboolean g_audio;
static gboolean verbose;
static gboolean g_save_images = FALSE;
static gint     g_port_num    = -1;
static gint64   g_seed        = -1;
static gchar   *g_log_domain  = NULL;
static gchar   *g_log_level   = "info";

static const char *g_audio_backend = "portaudio";

static void
signal_handler(int sig)
{
    switch (sig) {
    case SIGINT:
    case SIGABRT:
    case SIGSEGV:
        remove_log_handler();
        g_print("Recieved signal %d\nquitting\n", sig);
        exit(sig);
    }
}

/* clang-format off */
GOptionEntry options[] = {
    {"port-num", 'p',    G_OPTION_FLAG_NONE, G_OPTION_ARG_INT,   &g_port_num,
        "Specify a port number to open for the parallel tests.","-1"},
    {"save-images", 'S', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE,  &g_save_images,
        "Save images int /tmp_folder/psy-unit-tests/", NULL},
    {"seed",     's',    G_OPTION_FLAG_NONE, G_OPTION_ARG_INT64, &g_seed,
        "Seed for random functions [0 - 2^32)","-1"},
    {"verbose",  'v',    G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE,  &verbose,
        "Run the suite verbosely", NULL},
    {"audio",  'a',    G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE,  &g_audio,
        "Also run the audio tests", NULL},
    {"audio-backend",  'b', G_OPTION_FLAG_NONE, G_OPTION_ARG_STRING,
        &g_audio_backend, "the audio backend to use", NULL},
    {"log-domain",  'd', G_OPTION_FLAG_NONE, G_OPTION_ARG_STRING,
        &g_log_domain, "the log domain to monitor", NULL},
    {"log-level",  'l', G_OPTION_FLAG_NONE, G_OPTION_ARG_STRING,
        &g_log_level, "the log level debug, info, message, warning or critical", "info"},
    {0,},
};

/* clang-format on */

static int
add_suites_to_registry(void)
{
    int error = 0;

    error = add_ref_count_suite();
    if (error)
        return error;

    error = add_audio_channel_mapping_suite();
    if (error)
        return error;

    if (g_audio) {
        error = add_audio_suite(g_audio_backend);
        if (error)
            return error;
    }

    error = add_audio_utils_suite();
    if (error)
        return error;

    error = add_canvas_suite();
    if (error)
        return error;

    error = add_color_suite();
    if (error)
        return error;

    error = add_image_suite();
    if (error)
        return error;

    error = add_gl_canvas_suite();
    if (error)
        return error;

    error = add_gl_utils_suite();
    if (error)
        return error;

    error = add_matrix4_suite();
    if (error)
        return error;

    error = add_parallel_suite(g_port_num);
    if (error)
        return error;

    error = add_picture_suite();
    if (error)
        return error;

    error = add_queue_suite();
    if (error)
        return error;

    error = add_stepping_suite();
    if (error)
        return error;

    error = add_text_suite();
    if (error)
        return error;

    error = add_time_utilities_suite();
    if (error)
        return error;

    error = add_utility_suite();
    if (error)
        return error;

    error = add_visual_stimulus_suite();
    if (error)
        return error;

    error = add_visual_stimuli_suite();
    if (error)
        return error;

    error = add_vector_suite();
    if (error)
        return error;

    error = add_vector3_suite();
    if (error)
        return error;

    error = add_vector4_suite();
    if (error)
        return error;

    if (g_audio) {
        error = add_wave_suite();
        if (error)
            return error;
    }

    return error;
}

static void
init_libs(void)
{
#if defined PSY_HAVE_PORTAUDIO
    pa_Initialize();
#endif

    gst_init(NULL, NULL);
}

static void
deinitialize_libs(void)
{
    gst_deinit();

    Pa_Terminate();
}

static void
setup_log_handler(void)
{
    install_log_handler();
    GLogLevelFlags level = G_LOG_LEVEL_INFO;

    if (g_log_domain)
        set_log_handler_domain(g_log_domain);

    if (g_log_level) {
        if (g_strcmp0(g_log_level, "debug") == 0) {
            level = G_LOG_LEVEL_DEBUG;
        }
        else if (g_strcmp0(g_log_level, "info") == 0) {
            level = G_LOG_LEVEL_INFO;
        }
        else if (g_strcmp0(g_log_level, "message") == 0) {
            level = G_LOG_LEVEL_MESSAGE;
        }
        else if (g_strcmp0(g_log_level, "warning") == 0) {
            level = G_LOG_LEVEL_WARNING;
        }
        else if (g_strcmp0(g_log_level, "critical") == 0) {
            level = G_LOG_LEVEL_CRITICAL;
        }
        else {
            level = G_LOG_LEVEL_INFO;
        }
    }

    set_log_handler_level(level);
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
    GOptionContext *context = g_option_context_new("");
    g_option_context_add_main_entries(context, options, NULL);

    int     ret, n_test_failed = -1;
    GError *error = NULL;

    if (!g_option_context_parse(context, &argc, &argv, &error)) {
        g_printerr("Unable to parse options: %s\n", error->message);
        g_option_context_free(context);
        return EXIT_FAILURE;
    }

    setup_log_handler();
    setup_signal_handlers();

    if (g_seed < 0) {
        if (!init_random()) {
            g_critical("Oops unable to init random");
            return EXIT_FAILURE;
        }
    }
    else {
        if (!init_random_with_seed(g_seed)) {
            g_critical("Oops unable to init random");
            return EXIT_FAILURE;
        }
    }

    init_libs();

    set_save_images(g_save_images ? TRUE : FALSE);

    CU_initialize_registry();

    if (verbose)
        CU_basic_set_mode(CU_BRM_VERBOSE);

    ret = add_suites_to_registry();
    if (ret == 0) {
        CU_basic_run_tests();
        n_test_failed = (int) CU_get_number_of_tests_failed();
    }
    else {
        g_printerr("A CUnit error occurred: %s\n", CU_get_error_msg());
    }

    CU_cleanup_registry();
    g_print("\nRan with a seed of %u\n", random_seed());

    deinitialize_libs();
    deinitialize_random();

    remove_log_handler(); // clear logging stuff.

    g_option_context_free(context);

    return n_test_failed != 0;
}
