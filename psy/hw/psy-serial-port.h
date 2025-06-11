
#pragma once

#include <gio/gio.h>
#include <glib-object.h>

#include "../psy-enums.h"

G_BEGIN_DECLS

#define PSY_TYPE_SERIAL_PORT psy_serial_port_get_type()

G_MODULE_EXPORT
G_DECLARE_DERIVABLE_TYPE(
    PsySerialPort, psy_serial_port, PSY, SERIAL_PORT, GObject)

#define PSY_SERIAL_PORT_ERROR psy_serial_port_error_quark()

G_MODULE_EXPORT GQuark
psy_serial_port_error_quark(void);

/**
 * PsySerialPortClass:
 * @open: this will open the serial port, the device should have a name
 * @close: this will undo the open action.
 * @set_port_name: Sets the "OS" name of the serialport. This should
 *                 typically be called from a derived class' open function.
 * @write: This function should be implemented in the deriving class as it's
 *         not implemented in PsySerialPortClass, the deriving class makes
 *         sure that the mask is put to the datalines of the serial port.
 * @read: This function should be implemented in the deriving class as it's
 *        not implemented in PsySerialPortClass, the deriving class makes
 *        sure that you can obtain the mask of the pins on the SerialPort.
 */
typedef struct _PsySerialPortClass {
    GObjectClass parent_class;

    void (*open)(PsySerialPort *self, GError **error);
    void (*close)(PsySerialPort *self);

    void (*set_port_name)(PsySerialPort *self, const gchar *name);

    gsize (*write)(PsySerialPort *self,
                   const void    *bytes,
                   size_t         num_bytes,
                   GError       **error);

    gsize (*read)(PsySerialPort *self,
                  void          *bytes,
                  gsize          num_bytes,
                  GError       **error);

    gpointer padding[8];

} PsySerialPortClass;

G_MODULE_EXPORT PsySerialPort *
psy_serial_port_new(void);

G_MODULE_EXPORT void
psy_serial_port_free(PsySerialPort *self);

G_MODULE_EXPORT void
psy_serial_port_set_name(PsySerialPort *self, const gchar *name);

G_MODULE_EXPORT const char *
psy_serial_port_get_name(PsySerialPort *self);

G_MODULE_EXPORT void
psy_serial_port_open(PsySerialPort *self, GError **error);

G_MODULE_EXPORT void
psy_serial_port_close(PsySerialPort *self);

G_MODULE_EXPORT gboolean
psy_serial_port_is_open(PsySerialPort *self);

G_MODULE_EXPORT gsize
psy_serial_port_write(PsySerialPort *self,
                      const void    *bytes,
                      size_t         num_bytes,
                      GError       **error);

G_MODULE_EXPORT gsize
psy_serial_port_read(PsySerialPort *self,
                     void          *bytes,
                     gsize          num_bytes,
                     GError       **error);

G_END_DECLS
