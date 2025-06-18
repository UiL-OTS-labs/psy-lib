
#pragma once

#include <gio/gio.h>
#include <glib-object.h>

#include "../psy-duration.h"
#include "../psy-enums.h"

G_BEGIN_DECLS

#define PSY_SERIAL_PORT_ERROR psy_serial_port_error_quark()

G_MODULE_EXPORT GQuark
psy_serial_port_error_quark(void);

#define PSY_TYPE_SERIAL_PORT psy_serial_port_get_type()

G_MODULE_EXPORT
G_DECLARE_DERIVABLE_TYPE(
    PsySerialPort, psy_serial_port, PSY, SERIAL_PORT, GObject)

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

    gssize (*write)(PsySerialPort *self,
                    const guint8  *bytes,
                    gsize          num_bytes,
                    GError       **error);

    void (*read)(PsySerialPort *self,
                 guint8       **bytes,
                 gsize          num_bytes,
                 gsize         *num_bytes_read,
                 GError       **error);

    gssize (*read_raw)(PsySerialPort *self, guint8 *bytes, gsize num_bytes);

    gpointer padding[8];

} PsySerialPortClass;

G_MODULE_EXPORT PsySerialPort *
psy_serial_port_new(void);

G_MODULE_EXPORT PsySerialPort *
psy_serial_port_new_with_name(const gchar *device_name);

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
psy_serial_port_get_is_open(PsySerialPort *self);

G_MODULE_EXPORT gssize
psy_serial_port_write(PsySerialPort *self,
                      const guint8  *bytes,
                      size_t         num_bytes,
                      GError       **error);

G_MODULE_EXPORT void
psy_serial_port_read(PsySerialPort *self,
                     guint8       **bytes,
                     gsize          num_bytes,
                     gsize         *num_bytes_read,
                     GError       **error);

G_MODULE_EXPORT gssize
psy_serial_port_read_raw(PsySerialPort *self, guint8 *bytes, gsize num_bytes);

G_MODULE_EXPORT void
psy_serial_port_set_baud_rate(PsySerialPort *self, PsyBaudRate rate);

G_MODULE_EXPORT PsyBaudRate
psy_serial_port_get_baud_rate(PsySerialPort *self);

G_MODULE_EXPORT void
psy_serial_port_set_timeout(PsySerialPort *self, PsyDuration *duration);

G_MODULE_EXPORT PsyDuration *
psy_serial_port_get_timeout(PsySerialPort *self);

G_MODULE_EXPORT void
psy_serial_port_set_num_min_chars(PsySerialPort *self, guint num_chars);

G_MODULE_EXPORT guint
psy_serial_port_get_num_min_chars(PsySerialPort *self);

G_END_DECLS
