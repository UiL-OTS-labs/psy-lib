
#pragma once

#include "psy-parallel-port.h"

G_BEGIN_DECLS

#define PSY_TYPE_PARPORT psy_parport_get_type()

G_DECLARE_FINAL_TYPE(PsyParport, psy_parport, PSY, PARPORT, PsyParallelPort)

G_MODULE_EXPORT PsyParport *
psy_parport_new(void);

G_END_DECLS
