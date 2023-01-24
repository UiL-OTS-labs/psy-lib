
#include "psy-parport.h"

#include <error.h>
#include <fcntl.h>
#include <linux/parport.h>
#include <linux/ppdev.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>

/**
 * PsyParport:
 *
 * PsyParport is a final class for parallel ports on Linux. It derives from
 * PsyParallePort and implements it.
 */

typedef struct _PsyParport {
    PsyParallelPort parent;
    int             fd;
} PsyParport;

G_DEFINE_TYPE(PsyParport, psy_parport, PSY_TYPE_PARALLEL_PORT)

static void
psy_parport_init(PsyParport *self)
{
    (void) self;
}

static void
parport_open(PsyParallelPort *self, gint port_num, GError **error)
{
    int          mode;
    gchar        buffer[64];
    PsyParport  *pp       = PSY_PARPORT(self);
    const gchar *dev_name = NULL;

    PsyParallelPortClass *parallel_cls = PSY_PARALLEL_PORT_GET_CLASS(self);

    psy_parallel_port_close(self);

    PSY_PARALLEL_PORT_CLASS(psy_parport_parent_class)
        ->open(self, port_num, error);

    g_snprintf(buffer, sizeof(buffer), "/dev/parport%u", port_num);
    parallel_cls->set_port_name(self, buffer);

    dev_name = psy_parallel_port_get_port_name(self);

    errno  = 0;
    pp->fd = open(dev_name, O_RDWR);
    if (pp->fd < 0) {
        g_set_error(error,
                    PSY_PARALLEL_PORT_ERROR,
                    PSY_PARALLEL_PORT_ERROR_OPEN,
                    "Unable to open %s: %s",
                    dev_name,
                    g_strerror(errno));
        return;
    }

    if (ioctl(pp->fd, PPGETMODE, &mode)) {
        goto error;
    }
    if (mode != IEEE1284_MODE_COMPAT) {
        mode = IEEE1284_MODE_COMPAT;
        if (ioctl(pp->fd, PPSETMODE, &mode))
            goto error;
    }

    int set_flags = PP_FASTWRITE | PP_FASTREAD;
    if (ioctl(pp->fd, PPSETFLAGS, &set_flags))
        goto error;

    int is_output =
        psy_parallel_port_get_direction(self) == PSY_IO_DIRECTION_OUT;

    if (ioctl(pp->fd, PPDATADIR, &is_output))
        goto error;

    parallel_cls->set_port_name(self, buffer);
    parallel_cls->open(self, port_num, error);
    return;

error:

    g_set_error(error,
                PSY_PARALLEL_PORT_ERROR,
                PSY_PARALLEL_PORT_ERROR_OPEN,
                "Unable to configure device %s: %s",
                dev_name,
                g_strerror(errno));
}

static void
parport_close(PsyParallelPort *self)
{
    PsyParport *pp = PSY_PARPORT(self);

    if (pp->fd >= 0) {
        ioctl(pp->fd, PPRELEASE);
        close(pp->fd);
        pp->fd = -1;
    }

    PSY_PARALLEL_PORT_CLASS(psy_parport_parent_class)->close(self);
}

static void
parport_write(PsyParallelPort *self, guint8 pins, GError **error)
{
    PsyParport *pp      = PSY_PARPORT(self);
    gboolean    is_open = psy_parallel_port_is_open(self);
    gboolean    is_output =
        psy_parallel_port_get_direction(self) == PSY_IO_DIRECTION_OUT;

    if (!is_open) {
        g_set_error(error,
                    PSY_PARALLEL_PORT_ERROR,
                    PSY_PARALLEL_PORT_ERROR_DEV_CLOSED,
                    "Can't write to closed device");
        return;
    }

    if (!is_output) {
        g_set_error(
            error,
            PSY_PARALLEL_PORT_ERROR,
            PSY_PARALLEL_PORT_ERROR_DIRECTION,
            "Unable to write to a port that is not configured for output.");
    }

    if (ioctl(pp->fd, PPWDATA, &pins) == -1) {
        g_set_error(error,
                    PSY_PARALLEL_PORT_ERROR,
                    PSY_PARALLEL_PORT_ERROR_FAILED,
                    "Unable to write lines: %s",
                    g_strerror(errno));
        return;
    }

    psy_parallel_port_set_pins(self, pins);
}

static void
parport_write_pin(PsyParallelPort *self,
                  gint             pin,
                  PsyIoLevel       level,
                  GError         **error)
{
    guint8 current = psy_parallel_port_get_pins(self);
    guint8 final;
    if (level == PSY_IO_LEVEL_HIGH) {
        final = current | 1ul << pin;
    }
    else {
        final = current & ~(1ul << pin);
    }

    psy_parallel_port_write(self, final, error);
}

static guint8
parport_read(PsyParallelPort *self, GError **error)
{
    PsyParport *pp      = PSY_PARPORT(self);
    gboolean    is_open = psy_parallel_port_is_open(self);
    gboolean    is_input =
        psy_parallel_port_get_direction(self) == PSY_IO_DIRECTION_IN;
    guint8 lines = 0;

    if (!is_open) {
        g_set_error(error,
                    PSY_PARALLEL_PORT_ERROR,
                    PSY_PARALLEL_PORT_ERROR_DEV_CLOSED,
                    "Can't read from closed device");
        return 0;
    }

    if (!is_input) {
        g_set_error(
            error,
            PSY_PARALLEL_PORT_ERROR,
            PSY_PARALLEL_PORT_ERROR_DIRECTION,
            "Unable to read from a port that is not configured as input.");
    }

    if (ioctl(pp->fd, PPWDATA, &lines) == -1) {
        g_set_error(error,
                    PSY_PARALLEL_PORT_ERROR,
                    PSY_PARALLEL_PORT_ERROR_FAILED,
                    "Unable to read lines: %s",
                    g_strerror(errno));
        return 0;
    }

    psy_parallel_port_set_pins(self, lines);
    return lines;
}

static PsyIoLevel
parport_read_pin(PsyParallelPort *self, gint pin, GError **error)
{
    guint8 pins = psy_parallel_port_read(self, error);
    if (error && (*error != NULL)) {
        return PSY_IO_LEVEL_LOW;
    }

    return pins & (1ul << pin) ? PSY_IO_LEVEL_HIGH : PSY_IO_LEVEL_LOW;
}

static void
psy_parport_class_init(PsyParportClass *cls)
{
    GObjectClass *obj_cls = G_OBJECT_CLASS(cls);
    (void) obj_cls;

    PsyParallelPortClass *parallel_cls = PSY_PARALLEL_PORT_CLASS(cls);

    parallel_cls->open      = parport_open;
    parallel_cls->close     = parport_close;
    parallel_cls->write     = parport_write;
    parallel_cls->write_pin = parport_write_pin;
    parallel_cls->read      = parport_read;
    parallel_cls->read_pin  = parport_read_pin;
}
