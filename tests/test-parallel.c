
#include <CUnit/CUnit.h>
#include <CUnit/TestDB.h>

#include "hw/psy-parallel-port.h"
#include "psy-config.h"

gint g_port_num = -1;

static void
parallel_port_create(void)
{
    guint  pins;
    gchar *name = NULL;
    gint   port_num;
    gint   dir = 0;

    gboolean is_output, is_input, is_open;

    PsyParallelPort *port = psy_parallel_port_new();

    CU_ASSERT_PTR_NOT_NULL(port);
    if (!port)
        return;

    // clang-format off
    g_object_get(port,
                 "direction", &dir,
                 "pins", &pins,
                 "port-name", &name,
                 "port-num", &port_num,
                 "is_open", &is_open,
                 "is-output", &is_output,
                 "is-input", &is_input,
                 NULL);
    // clang-format on

    CU_ASSERT_EQUAL(dir, PSY_IO_DIRECTION_OUT);
    CU_ASSERT_STRING_EQUAL(name, "");
    CU_ASSERT_EQUAL(port_num, -1);
    CU_ASSERT_EQUAL(pins, 0);

    CU_ASSERT_FALSE(psy_parallel_port_is_open(port));
    CU_ASSERT_FALSE(psy_parallel_port_is_output(port));
    CU_ASSERT_FALSE(psy_parallel_port_is_input(port));
    CU_ASSERT_FALSE(is_input);
    CU_ASSERT_FALSE(is_output);
    CU_ASSERT_FALSE(is_input);

    g_free(name);
    g_object_unref(port);
}

static void
parallel_port_as_input(void)
{

    PsyIoDirection   dir;
    PsyParallelPort *port = psy_parallel_port_new();

    CU_ASSERT_PTR_NOT_NULL_FATAL(port);

    psy_parallel_port_set_direction(port, PSY_IO_DIRECTION_IN);

    g_object_get(port, "direction", &dir, NULL);

    CU_ASSERT_EQUAL(dir, PSY_IO_DIRECTION_IN);

    g_object_unref(port);
}

static void
parallel_port_open(void)
{
    gchar  *name;
    gint    port_num;
    GError *error = NULL;

    PsyParallelPort *port = psy_parallel_port_new();

    gchar expected_name[BUFSIZ];
#if defined(HAVE_LINUX_PARPORT_H)
    g_snprintf(expected_name, BUFSIZ, "/dev/parport%d", g_port_num);
#else
    g_snprintf(expected_name, BUFSIZ, "LPT%d", g_port_num + 1);
#endif

    psy_parallel_port_open(port, g_port_num, &error);
    gboolean open = psy_parallel_port_is_open(port);
    CU_ASSERT_TRUE(open);
    if (!open) {
        fprintf(stderr, "Unable to open port: %s", error->message);
        g_error_free(error);
        g_object_unref(port);
        return;
    }
    CU_ASSERT_PTR_NULL(error);
    if (error) {
        g_print("Error = %s\n", error->message);
    }

    g_object_get(port, "port-num", &port_num, "port-name", &name, NULL);

    CU_ASSERT_STRING_EQUAL(name, expected_name);
    CU_ASSERT_EQUAL(port_num, g_port_num);

    CU_ASSERT_TRUE(psy_parallel_port_is_output(port));
    CU_ASSERT_FALSE(psy_parallel_port_is_input(port));

    psy_parallel_port_set_direction(port, PSY_IO_DIRECTION_IN);

    CU_ASSERT_FALSE(psy_parallel_port_is_output(port));
    CU_ASSERT_TRUE(psy_parallel_port_is_input(port));

    psy_parallel_port_close(port);
    g_free(name);
    g_object_unref(port);
}

int
add_parallel_suite(gint port_num)
{
#if defined(HAVE_LINUX_PARPORT_H) // Check for other port implementations here
    CU_Suite *suite = CU_add_suite("parallel port tests", NULL, NULL);
    CU_Test  *test  = NULL;

    if (!suite)
        return 1;

    test = CU_add_test(suite,
                       "ParallelPort gets sensible default values",
                       parallel_port_create);
    if (!test)
        return 1;

    test = CU_add_test(
        suite, "Close ports may change to input", parallel_port_as_input);
    if (!test)
        return 1;

    if (port_num >= 0) {

        // These test must be enabled via the command line the
        // will be used as device number to open.

        g_port_num = port_num;

        test = CU_add_test(suite, "ParallelPort open port", parallel_port_open);
        if (!test)
            return 1;
    }

#else
    #pragma message "Can't test with parallel device"
#endif

    return 0;
}
