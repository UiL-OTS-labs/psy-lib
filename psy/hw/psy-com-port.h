
#pragma once

#include <gio/gio.h>

#include "../psy-enums.h"
#include "psy-serial-port.h"

G_BEGIN_DECLS

#define PSY_TYPE_COM_PORT psy_com_port_get_type()

G_MODULE_EXPORT
G_DECLARE_FINAL_TYPE(PsyComPort, psy_com_port, PSY, COM_PORT, PsySerialPort)

G_MODULE_EXPORT PsyComPort *
psy_com_port_new(void);

G_MODULE_EXPORT PsyComPort *
psy_com_port_new_with_name(const gchar *device_name);

G_MODULE_EXPORT void
psy_com_port_free(PsyComPort *self);

G_END_DECLS
