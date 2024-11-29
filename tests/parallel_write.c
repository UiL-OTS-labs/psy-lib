
#include <stdio.h>

#ifdef _WIN32
    #include <windows.h>
#endif

#include <psylib.h>

void
test_sleep(int ms)
{
#ifndef _WIN32
    usleep(ms * 1000);
#else
    Sleep(ms);
#endif
}

int g_port_num = 0;

// clang-format off
GOptionEntry entries[] = {
    {"port-num", 'p', G_OPTION_FLAG_NONE, G_OPTION_ARG_INT, &g_port_num, "Specify a port num [0,1,2]", NULL},
    {NULL},
};
// clang-format on

int
main(int argc, char **argv)
{
    GError *error = NULL;

    GOptionContext *opts = g_option_context_new("Open a parallelport");
    g_option_context_add_main_entries(opts, entries, NULL);

    g_option_context_parse(opts, &argc, &argv, &error);
    g_option_context_free(opts);
    if (error) {
        g_printerr("Unable to parse cmd arguments: %s\n", error->message);
        return EXIT_FAILURE;
    }

    PsyParallelPort *pp = psy_parallel_port_new();

    psy_parallel_port_open(pp, g_port_num, &error);
    if (error) {
        fprintf(stderr, "%s\n", error->message);
        goto the_end;
    }

    for (int i = 0; i < 100; i++) {
        psy_parallel_port_write(pp, 0, &error);
        if (error)
            break;
        // test_sleep(1);
        psy_parallel_port_write(pp, 255, &error);
        if (error)
            break;
        // test_sleep(1);
    }

    psy_parallel_port_write(pp, 0, &error);

    if (error) {
        fprintf(stderr, "Error while writing: %s", error->message);
        g_error_free(error);
    }

the_end:

    g_object_unref(pp);
    if (error)
        g_error_free(error);
    return 0;
}
