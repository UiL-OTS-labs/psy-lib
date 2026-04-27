#include <criterion/criterion.h>
#include <criterion/new/assert.h>
#include <stdio.h>

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
        cr_assert(infos != NULL);
        g_port_num = psy_parallel_port_info_port_number(infos[0]);
    }
}

TestSuite(parallel_port, .init = setup_parallel_port_suite);

Test(parallel_port, create)
{
    guint  pins;
    gchar *name = NULL;
    gint   port_num;
    gint   dir = 0;

    if (g_port_num < 0) {
        cr_log_info("No parallel port available skipping this test");
        return;
    }

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

    cr_expect(eq(dir, PSY_IO_DIRECTION_OUT));
    cr_expect(eq(name, ""));
    cr_expect(eq(port_num, -1));
    cr_expect(eq(pins, 0u));

    cr_expect(not(psy_parallel_port_is_open(port)));
    cr_expect(not(psy_parallel_port_is_output(port)));
    cr_expect(not(psy_parallel_port_is_input(port)));
    cr_expect(not(is_input));
    cr_expect(not(is_output));
    cr_expect(not(is_input));

    g_free(name);
    g_object_unref(port);
}

Test(parallel_port, as_input)
{

    PsyIoDirection   dir;
    PsyParallelPort *port = NULL;

    port = psy_parallel_port_new();

    cr_assert(ne(port, NULL));

    psy_parallel_port_set_direction(port, PSY_IO_DIRECTION_IN);

    g_object_get(port, "direction", &dir, NULL);

    cr_assert(eq(int, dir, PSY_IO_DIRECTION_IN));

    g_object_unref(port);
}

Test(parallel_port, open)
{
    gchar  *name;
    gint    port_num;
    GError *error = NULL;

    if (g_port_num < 0) {
        cr_log_info("No parallel port available skipping this test");
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
    cr_assert(eq(open, TRUE));
    cr_assert(zero(error));
    if (!open) {
        fprintf(stderr, "Unable to open port: %s", error->message);
        g_clear_error(&error);
        g_object_unref(port);
        return;
    }

    g_object_get(port, "port-num", &port_num, "port-name", &name, NULL);

    cr_assert(eq(name, expected_name));
    cr_assert(eq(port_num, g_port_num));

    cr_assert(eq(psy_parallel_port_is_output(port), TRUE));
    cr_assert(not(psy_parallel_port_is_input(port)));

    psy_parallel_port_set_direction(port, PSY_IO_DIRECTION_IN);

    cr_assert(not(psy_parallel_port_is_output(port)));
    cr_assert(all(psy_parallel_port_is_input(port)));

    psy_parallel_port_close(port);
    g_free(name);
    g_object_unref(port);
}
