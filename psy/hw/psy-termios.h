
#pragma once

#include <gio/gio.h>

#include "../psy-enums.h"
#include "psy-serial-port.h"

G_BEGIN_DECLS

#define PSY_TYPE_TERMIOS psy_termios_get_type()

G_MODULE_EXPORT
G_DECLARE_FINAL_TYPE(PsyTermios, psy_termios, PSY, TERMIOS, PsySerialPort)

G_MODULE_EXPORT PsyTermios *
psy_termios_new(void);

G_MODULE_EXPORT PsyTermios *
psy_termios_new_with_name(const gchar *device_name);

G_MODULE_EXPORT void
psy_termios_free(PsyTermios *self);

G_END_DECLS
