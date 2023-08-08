
#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>
#include <CUnit/TestRun.h>
#include <glib.h>
#include <stdlib.h>

#include "suites.h"
#include "unit-test-utilities.h"

static gboolean g_audio;
static gboolean verbose;
static gboolean g_save_images = FALSE;
static gint     g_port_num    = -1;
static gint64   g_seed        = -1;

static const char *g_audio_backend = "jack";

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

    if (g_audio) {
        error = add_audio_suite(g_audio_backend);
        if (error)
            return error;
    }

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

    error = add_vector_suite();
    if (error)
        return error;

    error = add_vector3_suite();
    if (error)
        return error;

    error = add_vector4_suite();
    if (error)
        return error;

    return error;
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
    deinitialize_random();
    g_option_context_free(context);

    return n_test_failed != 0;
}
