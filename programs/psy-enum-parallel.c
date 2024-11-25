
#include "hw/psy-parallel-port.h"
#include <psylib.h>

int
main(void)
{
    gint                  n     = 0;
    PsyParallelPortInfo **ports = NULL;

    PsyParallelPort *port = psy_parallel_port_new();

    psy_parallel_port_enumerate(port, &ports, &n);

    for (int i = 0; i < n; i++) {
        g_print("Port %d: %s\n",
                psy_parallel_port_info_port_number(ports[i]),
                psy_parallel_port_info_name(ports[i]));
    }

    psy_parallel_port_free(port);
}
