
#pragma once

#include <gio/gio.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define PSY_TYPE_PARALLEL_PORT psy_parallel_port_get_type()
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

#define PSY_PARALLEL_PORT_ERROR psy_parallel_port_error_quark()

/**
 * PsyParallelPortError:
 * @PSY_PARALLEL_PORT_ERROR_OPEN: Unable to open the device
 * @PSY_PARALLEL_PORT_ERROR_DEV_CLOSED: Unable to perform action on a device
 *     that isn't open yet.
 * @PSY_PARALLEL_PORT_ERROR_DIRECTION: Unable to perform action because the
 *     devices is not configured in the desired direction for the action
 *     e.g. writing to a deviced configured as #PSY_IO_DIRECTION_INPUT
 * @PSY_PARALLEL_PORT_ERROR_FAILED: Operation failed (check error message?).
 */
typedef enum {
    PSY_PARALLEL_PORT_ERROR_OPEN,
    PSY_PARALLEL_PORT_ERROR_DEV_CLOSED,
    PSY_PARALLEL_PORT_ERROR_DIRECTION,
    PSY_PARALLEL_PORT_ERROR_FAILED,
} PsyParallelPortError;

G_MODULE_EXPORT GQuark
psy_parallel_port_error_quark(void);

/**
 * PsyParallelPortClass:
 * @open: this will set the pin number on the instance, a deriving class
 *        should actually open a device and chain up in order to tell which
 *        device is opened.
 * @close: this will undo the open action.
 * @set_port_name: Sets the "OS" name of the parallelport. This should
 *                 typically be called from a derived class' open function.
 * @write: This function should be implemented in the deriving class as it's
 *         not implemented in PsyParallelPortClass, the deriving class makes
 *         sure that the mask is put to the datalines of the parallel port.
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

    void (*set_port_name)(PsyParallelPort *self, const gchar *name);

    void (*write)(PsyParallelPort *self, guint8 mask, GError **error);
    void (*write_pin)(PsyParallelPort *self,
                      gint             pin,
                      PsyIoLevel       level,
                      GError         **error);

    guint8 (*read)(PsyParallelPort *self, GError **error);
    PsyIoLevel (*read_pin)(PsyParallelPort *self, gint pin, GError **error);

    gpointer padding[8];

} PsyParallelPortClass;

G_MODULE_EXPORT PsyParallelPort *
psy_parallel_port_new(void);

G_MODULE_EXPORT void
psy_parallel_port_open(PsyParallelPort *self, gint dev_num, GError **error);

G_MODULE_EXPORT void
psy_parallel_port_close(PsyParallelPort *self);

G_MODULE_EXPORT gboolean
psy_parallel_port_is_open(PsyParallelPort *self);

G_MODULE_EXPORT const gchar *
psy_parallel_port_get_port_name(PsyParallelPort *self);

G_MODULE_EXPORT void
psy_parallel_port_set_direction(PsyParallelPort *self, PsyIoDirection dir);

G_MODULE_EXPORT PsyIoDirection
psy_parallel_port_get_direction(PsyParallelPort *self);

G_MODULE_EXPORT gboolean
psy_parallel_port_is_output(PsyParallelPort *self);

G_MODULE_EXPORT gboolean
psy_parallel_port_is_input(PsyParallelPort *self);

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

G_MODULE_EXPORT guint8
psy_parallel_port_get_pins(PsyParallelPort *self);

// private

void
psy_parallel_port_set_pins(PsyParallelPort *self, guint8 lines);

G_END_DECLS
