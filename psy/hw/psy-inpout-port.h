
#pragma once

#include "psy-parallel-port.h"

G_BEGIN_DECLS

#define PSY_TYPE_INPOUT_PORT psy_inpout_port_get_type()

G_MODULE_EXPORT
G_DECLARE_FINAL_TYPE(
    PsyInpoutPort, psy_inpout_port, PSY, INPOUT_PORT, PsyParallelPort)

G_END_DECLS
