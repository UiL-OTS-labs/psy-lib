
#include <CUnit/CUnit.h>
#include <CUnit/TestDB.h>

#include "hw/parallel-port.h"
#include "psy-config.h"

gint g_port_num = -1;

static void
parallel_port_create(void)
{
    fprintf(stderr, "%s\n", __FUNCTION__);
    guint8 pins;
    gchar *name = NULL;
    gint   port_num;
    gint   dir = 0;

    PsyParallelPort *port = psy_parallel_port_new();

    CU_ASSERT_PTR_NOT_NULL(port);
    if (!port)
        return;

    g_object_get(port,
                 "direction",
                 &dir,
                 "pins",
                 &pins,
                 "port-name",
                 &name,
                 "port-num",
                 &port_num,
                 NULL);

    CU_ASSERT_EQUAL(dir, PSY_IO_DIRECTION_OUT);
    CU_ASSERT_STRING_EQUAL(name, "");
    CU_ASSERT_EQUAL(port_num, -1);
    CU_ASSERT_EQUAL(pins, 0);

    CU_ASSERT_FALSE(psy_parallel_port_is_open(port));
    CU_ASSERT_FALSE(psy_parallel_port_is_output(port));
    CU_ASSERT_FALSE(psy_parallel_port_is_input(port));

    g_free(name);
    g_object_unref(port);
    fprintf(stderr, "%s\n", __FUNCTION__);
}

static void
parallel_port_open(void)
{
    fprintf(stderr, "%s\n", __FUNCTION__);
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
    fprintf(stderr, "%s\n", __FUNCTION__);
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

    if (port_num >= 0) {

        // These test must be enabled via the command line the
        // will be used as device number to open.

        g_port_num = port_num;

        test = CU_add_test(suite, "ParallelPort open port", parallel_port_open);
        if (!test)
            return 1;
    }

#else
    #message "Can't test with parallel device"
#endif

    return 0;
}
