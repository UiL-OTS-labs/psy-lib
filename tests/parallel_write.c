
#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include <psylib.h>

void test_sleep(int ms) {
#ifndef _WIN32
    usleep(ms * 1000);
#else
    Sleep(ms);
#endif
}

int
main(int argc, char **argv)
{
    (void) argc;
    (void) argv;

    PsyParallelPort *pp = psy_parallel_port_new();

    GError *error = NULL;

    psy_parallel_port_open(pp, 0, &error);
    if (error) {
        fprintf(stderr, "%s\n", error->message);
        goto the_end;
    }

    for (int i = 0; i < 100; i++) {
        psy_parallel_port_write(pp, 0, &error);
        if (error)
            break;
        psy_parallel_port_write(pp, 255, &error);
        if (error)
            break;
        test_sleep(1);
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
