#include <stdio.h>

#include "hw/psy-parallel-port.h"
#include "psy-config.h"
#include <psylib.h>

gint g_port_num = -1;

static void
setup_parallel_port_suite(void)
{
    /* Determine whether we have a parallel port */
    gint                  n_ports = 0;
    PsyParallelPortInfo **infos   = NULL;

    PsyParallelPort *port = psy_parallel_port_new();
    psy_parallel_port_enumerate(port, &infos, &n_ports);

    if (n_ports > 0) {
        g_assert(infos != NULL);
        g_port_num = psy_parallel_port_info_port_number(infos[0]);
    }

    psy_parallel_port_free(port);
}

static void
test_parallel_port_create(void)
{
    guint  pins;
    gchar *name = NULL;
    gint   port_num;
    gint   dir = 0;

    gboolean is_output, is_input, is_open;

    PsyParallelPort *port = psy_parallel_port_new();

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

    g_assert_cmpint(dir, ==, PSY_IO_DIRECTION_OUT);
    g_assert_cmpstr(name, ==, "");
    g_assert_cmpint(port_num, ==, -1);
    g_assert_cmpuint(pins, ==, 0u);

    g_assert_false(psy_parallel_port_is_open(port));
    g_assert_false(psy_parallel_port_is_output(port));
    g_assert_false(psy_parallel_port_is_input(port));
    g_assert_false(is_input);
    g_assert_false(is_output);
    g_assert_false(is_input);

    g_free(name);
    g_object_unref(port);
}

static void
test_parallel_port_as_input(void)
{
    PsyIoDirection   dir;
    PsyParallelPort *port = NULL;

    port = psy_parallel_port_new();

    g_assert_nonnull(port);

    psy_parallel_port_set_direction(port, PSY_IO_DIRECTION_IN);

    g_object_get(port, "direction", &dir, NULL);

    g_assert_cmpint(dir, ==, PSY_IO_DIRECTION_IN);

    g_object_unref(port);
}

static void
test_parallel_port_open(void)
{
    gchar  *name;
    gint    port_num;
    GError *error = NULL;

    if (g_port_num < 0) {
        g_test_skip("No parallel port available skipping this test");
        return;
    }

    PsyParallelPort *port = psy_parallel_port_new();

    gchar expected_name[BUFSIZ];
#if defined(HAVE_LINUX_PARPORT_H)
    g_snprintf(expected_name, BUFSIZ, "/dev/parport%d", g_port_num);
#elif defined(WIN32)
    const gchar *win_address;
    if (g_port_num == 0)
        win_address = "0x378";
    else if (g_port_num == 1)
        win_address = "0x278";
    else if (g_port_num == 2)
        win_address = "0x3BC";
    else
        win_address = "";
    g_snprintf(expected_name, BUFSIZ, "%s", win_address);
#endif

    psy_parallel_port_open(port, g_port_num, &error);
    gboolean open = psy_parallel_port_is_open(port);
    g_assert_true(open);
    g_assert_no_error(error);
    if (!open) {
        fprintf(stderr, "Unable to open port: %s", error->message);
        g_clear_error(&error);
        g_object_unref(port);
        return;
    }

    g_object_get(port, "port-num", &port_num, "port-name", &name, NULL);

    g_assert_cmpstr(name, ==, expected_name);
    g_assert_cmpint(port_num, ==, g_port_num);

    g_assert_true(psy_parallel_port_is_output(port));
    g_assert_false(psy_parallel_port_is_input(port));

    psy_parallel_port_set_direction(port, PSY_IO_DIRECTION_IN);

    g_assert_false(psy_parallel_port_is_output(port));
    g_assert_true(psy_parallel_port_is_input(port));

    psy_parallel_port_close(port);
    g_free(name);
    g_object_unref(port);
}

int
main(int argc, char **argv)
{
    g_test_init(&argc, &argv, NULL);

    setup_parallel_port_suite();

    g_test_add_func("/parallel_port/create", test_parallel_port_create);
    g_test_add_func("/parallel_port/as_input", test_parallel_port_as_input);
    g_test_add_func("/parallel_port/open", test_parallel_port_open);

    return g_test_run();
}
