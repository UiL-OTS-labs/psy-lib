
#include <CUnit/TestRun.h>
#include <glib.h>
#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <stdlib.h>

#include "suites.h"

gboolean verbose;

GOptionEntry options[] = {
    {"verbose", 'v', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &verbose, "Run the suite verbosely",""},
    {0}
};

static int
add_suites_to_registry(void)
{
    int error = 0;

    error = add_ref_count_suite();
    if (error)
        return error;

    error = add_color_suite();
    if (error)
        return error;

    return error;
}

int main(int argc, char** argv) {
    GOptionContext* context = g_option_context_new("");
    g_option_context_add_main_entries(context, options, NULL);
    int ret, n_test_failed = -1;
    GError* error = NULL;

    if (!g_option_context_parse(context, &argc, &argv, &error)) {
        g_printerr("Unable to parse options: %s\n", error->message);
        g_option_context_free(context);
        return EXIT_FAILURE;
    }
    
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
    g_option_context_free(context);

    return n_test_failed != 0;
}
