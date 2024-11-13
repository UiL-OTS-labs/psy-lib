
#include <stdio.h>

#include "psy-config.h"
#include "psy-inpout-port.h"

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
#endif

/* ******* loading of inpout dll for controlling SPP registers ******** */

static gint      open_count = 0;
static HINSTANCE inpout_dll = NULL;
static GMutex    g_open_mutex;

#if PSY_CPU_FAMILY == "x86"
const gchar *g_dll_name = "inpout32.dll";
#elif PSY_CPU_FAMILY == "x86_64"
const gchar *g_dll_name = "inpout64.dll";
#else
    #error "Unsupported platform"
#endif

static gboolean
load_inpout(GError **error)
{
    gboolean ret;
    g_mutex_lock(&g_open_mutex);

    open_count++;

    if (open_count == 1) {
        inpout_dll = LoadLibraryA(g_dll_name);

        if (inpout_dll == NULL) {
            const char buff[BUFSIZ];
            gint       error = GetLastError();
            psy_strerr(error, buff, BUFSIZ);

            g_set_error(error,
                        PSY_PARALLEL_PORT_ERROR,
                        "Unable to load library \"%s\":%s",
                        g_dll_name,
                        buff);
        }
    }

    g_mutex_unlock(&g_open_mutex);
    return TRUE;

error:

    open_count--;

    g_mutex_unlock(&g_open_mutex);
}

/**
 * PsyInpoutPort:
 *
 * PsyInpoutPort is a final class for parallel ports on Windows. It derives from
 * PsyParallelPort and implements it. Typically you can instantiate instances
 * of this class with [ctor@ParallelPort.new] and that constructor
 * determines the right backend for your system.
 */

typedef struct _PsyInpoutPort {
    PsyParallelPort parent;
    int             num;
    gint16          data_register;
    gint16          status_register;
    gint16          control_register;
} PsyInpoutPort;

G_DEFINE_TYPE(PsyInpoutPort, psy_inpout_port, PSY_TYPE_PARALLEL_PORT)

static void
psy_inpout_port_init(PsyInpoutPort *self)
{
    (void) self;
}

static void
inpout_port_open(PsyParallelPort *self, gint port_num, GError **error)
{
    int            mode;
    gchar          buffer[64];
    PsyInpoutPort *pp       = PSY_INPOUT_PORT(self);
    const gchar   *dev_name = NULL;

    PsyParallelPortClass *parallel_cls = PSY_PARALLEL_PORT_GET_CLASS(self);

    psy_parallel_port_close(self);

    PSY_PARALLEL_PORT_CLASS(psy_inpout_port_parent_class)
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

    if (ioctl(pp->fd, PPCLAIM)) { // Claim the device, before using it
        goto error;
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

    int is_output
        = psy_parallel_port_get_direction(self) == PSY_IO_DIRECTION_OUT ? 0 : 1;

    if (ioctl(pp->fd, PPDATADIR, &is_output) != 0)
        goto error;

    return;

error:

    psy_parallel_port_close(self);

    g_set_error(error,
                PSY_PARALLEL_PORT_ERROR,
                PSY_PARALLEL_PORT_ERROR_OPEN,
                "Unable to configure device %s: %s",
                dev_name,
                g_strerror(errno));
}

static void
inpout_port_close(PsyParallelPort *self)
{
    PsyInpoutPort *pp = PSY_INPOUT_PORT(self);

    if (pp->fd >= 0) {
        ioctl(pp->fd, PPRELEASE);
        close(pp->fd);
        pp->fd = -1;
    }

    PSY_PARALLEL_PORT_CLASS(psy_inpout_port_parent_class)->close(self);
}

static void
inpout_port_write(PsyParallelPort *self, guint8 pins, GError **error)
{
    PsyInpoutPort *pp      = PSY_INPOUT_PORT(self);
    gboolean       is_open = psy_parallel_port_is_open(self);
    gboolean       is_output
        = psy_parallel_port_get_direction(self) == PSY_IO_DIRECTION_OUT;

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
inpout_port_write_pin(PsyParallelPort *self,
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
inpout_port_read(PsyParallelPort *self, GError **error)
{
    PsyInpoutPort *pp      = PSY_INPOUT_PORT(self);
    gboolean       is_open = psy_parallel_port_is_open(self);
    gboolean       is_input
        = psy_parallel_port_get_direction(self) == PSY_IO_DIRECTION_IN;
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
inpout_port_read_pin(PsyParallelPort *self, gint pin, GError **error)
{
    guint8 pins = psy_parallel_port_read(self, error);
    if (error && (*error != NULL)) {
        return PSY_IO_LEVEL_LOW;
    }

    return pins & (1ul << pin) ? PSY_IO_LEVEL_HIGH : PSY_IO_LEVEL_LOW;
}

static void
psy_inpout_port_class_init(PsyInpoutPortClass *cls)
{
    GObjectClass *obj_cls = G_OBJECT_CLASS(cls);
    (void) obj_cls;

    PsyParallelPortClass *parallel_cls = PSY_PARALLEL_PORT_CLASS(cls);

    parallel_cls->open      = inpout_port_open;
    parallel_cls->close     = inpout_port_close;
    parallel_cls->write     = inpout_port_write;
    parallel_cls->write_pin = inpout_port_write_pin;
    parallel_cls->read      = inpout_port_read;
    parallel_cls->read_pin  = inpout_port_read_pin;
}
