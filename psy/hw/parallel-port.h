
#pragma once

#include <gio/gio.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define PSY_TYPE_PARALLE_PORT psy_parallel_port_get_type()
G_DECLARE_DERIVABLE_TYPE(
    PsyParallelPort, psy_parallel_port, PSY, PARALLEL_PORT, GObject)

/**
 * PsyIoDirection:
 * @PSY_IO_DIRECTION_INPUT: The device is/should be configured as input
 * @PSY_IO_DIRECTION_OUTPUT: The device is/should be configured as output
 *
 * These values may be used to configure a device as in- or output.
 */
typedef enum PsyIoDirection {
    PSY_IO_DIRECTION_IN,
    PSY_IO_DIRECTION_OUT,
} PsyIoDirection;

/**
 * PsyIoLevel:
 * @PSY_IO_LEVEL_HIGH: The logical high level of an signal
 * @PSY_IO_LEVEL_LOW: The logical low level of an signal
 *
 * These values may be used to configure a line etc to have a
 * high or low level voltage, it depends on the device what is the precise
 * level.
 */
typedef enum PsyIoLevel {
    PSY_IO_LEVEL_LOW,
    PSY_IO_LEVEL_HIGH,
} PsyIoLevel;

/**
 * PsyParallelPortClass:
 * @open: this will set the pin number on the instance, a deriving class should
 *        actually open a device and chain up in order to tell which device is
 *        opened.
 * @close: this will undo the open action.
 * @write: This function should be implemented in the deriving class as it's
 *         not implemented in PsyParallelPortClass, the deriving class makes
 *         sure that the mask is put to the datalines of the parallel port.
 * @set_device_name: Sets the "OS" name of the parallelport. This should
 *                   typically be called from a derived class' open function.
 * @write_pin: This should be implemented in the deriving class.
               this function should write on of the datalines high or low.
 * @read: This function should be implemented in the deriving class as it's
 *        not implemented in PsyParallelPortClass, the deriving class makes
 *        sure that you can obtain the mask of the pins on the ParallelPort.
 * @read_pin: This should be implemented in the deriving class. This
 *            function reads whether the signal is high or low.
 */
typedef struct _PsyParallelPortClass {
    GObjectClass parent_class;

    void (*open)(PsyParallelPort *self, gint dev_num, GError **error);
    void (*close)(PsyParallelPort *self);

    void (*set_device_name)(PsyParallelPort *self, const gchar *name);

    void (*write)(PsyParallelPort *self, guint8 mask, GError **error);
    void (*write_pin)(PsyParallelPort *self,
                      gint             pin,
                      PsyIoLevel       level,
                      GError         **error);

    guint8 (*read)(PsyParallelPort *self, GError **error);
    PsyIoLevel (*read_pin)(PsyParallelPort *self, gint pin, GError **error);

} PsyParallelPortClass;

G_MODULE_EXPORT void
psy_parallel_port_open(PsyParallelPort *self, gint dev_num, GError **error);

G_MODULE_EXPORT void
psy_parallel_port_close(PsyParallelPort *self);

G_MODULE_EXPORT gboolean
psy_parallel_port_is_open(PsyParallelPort *self);

G_MODULE_EXPORT void
psy_parallel_port_set_direction(PsyParallelPort *self, PsyIoDirection dir);

G_MODULE_EXPORT PsyIoDirection
psy_parallel_port_get_direction(PsyParallelPort *self);

G_MODULE_EXPORT void
psy_parallel_port_write(PsyParallelPort *self, guint8 mask, GError **eror);

G_MODULE_EXPORT void
psy_parallel_port_write_pin(PsyParallelPort *self,
                            gint             pin,
                            PsyIoLevel       level,
                            GError         **error);

G_MODULE_EXPORT guint8
psy_parallel_port_read(PsyParallelPort *self, GError **error);

G_MODULE_EXPORT PsyIoLevel
psy_parallel_port_read_pin(PsyParallelPort *self, gint pin, GError **error);

G_END_DECLS
