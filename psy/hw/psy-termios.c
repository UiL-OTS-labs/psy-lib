
#include <errno.h>
#include <math.h>

#include "enum-types.h"
#include "psy-config.h"
#include "psy-termios.h"

#if defined(HAVE_TERMIOS_H)
    #include <termios.h>
    // let assume we have to headers below too
    #include <fcntl.h>
    #include <sys/stat.h>
    #include <sys/types.h>
    #include <unistd.h>
#else
    #error                                                                     \
        "Oops, trying to compile psy-termios.c on a system without termios.h";
#endif

/**
 * PsyTermios:
 *
 * This is an implementation of a Serial port on a posix system that has
 * "termios.h". This class is not designed to create your own terminal using
 * a canonical mode terminal, it designed to do raw communication with the
 * other end.
 *
 * Suitable devices for this class are typically: /dev/ttyACMx or /dev/ttyUSBx/
 * where 0 >= x < num_devices
 */

// forward declaration
static int
termios_choose_baud_rate(PsyBaudRate rate);

static guint8
find_suitable_timeout(const PsyDuration *dur);

typedef struct _PsyTermios {
    PsySerialPort parent;
    int           fd;

    struct termios existing_config;
    struct termios config;
} PsyTermios;

G_DEFINE_FINAL_TYPE(PsyTermios, psy_termios, PSY_TYPE_SERIAL_PORT)

typedef enum PsyTermiosProperty {
    PROP_NULL,
    NUM_PROPS,
} PsyTermiosProperty;

// // Currently PsyTermios hasn't got any properties
// static GParamSpec *termios_props[NUM_PROPS];
//
// static void
// psy_termios_set_property(GObject      *object,
//                          guint         property_id,
//                          const GValue *value,
//                          GParamSpec   *spec)
// {
//     PsyTermios *self = PSY_TERMIOS(object);
//     (void) self;
//     (void) value;
//
//     switch ((PsyTermiosProperty) property_id) {
//     default:
//         G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, spec);
//     }
// }
//
// static void
// psy_termios_get_property(GObject    *object,
//                          guint       property_id,
//                          GValue     *value,
//                          GParamSpec *spec)
// {
//     PsyTermios *self = PSY_TERMIOS(object);
//     (void) self;
//     (void) value;
//
//     switch ((PsyTermiosProperty) property_id) {
//     default:
//         G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, spec);
//     }
// }

static void
psy_termios_init(PsyTermios *self)
{
    self->fd = -1; // 0 would be a valid fd
}

/*
 * Try to open the serial device's file. Stores the current configuration of the
 * serial port, in order to restore it on close, so it appears that psylib
 * hasn't touched the device. Then it sets the serial it tries to open the
 * serial port.
 */
static void
termios_open(PsySerialPort *self, GError **error)
{
    PsyTermios *termios_self = PSY_TERMIOS(self);

    termios_self->fd
        = open(psy_serial_port_get_name(self), O_RDWR | O_NOCTTY | O_NOFOLLOW);

    if (termios_self->fd < 0) {
        switch (errno) {
        case ENOENT: // file doesn't exist
            g_set_error(error,
                        PSY_SERIAL_PORT_ERROR,
                        PSY_SERIAL_PORT_ERROR_NO_SUCH_DEVICE,
                        "Unable to open %s, no such device",
                        psy_serial_port_get_name(self));
            break;
        case EACCES: // no permission
            g_set_error(error,
                        PSY_SERIAL_PORT_ERROR,
                        PSY_SERIAL_PORT_ERROR_NO_PERMISSION,
                        "Unable to open %s: %s, are you in the dialout group?",
                        psy_serial_port_get_name(self),
                        g_strerror(errno));
            break;
        default:
            g_set_error(error,
                        PSY_SERIAL_PORT_ERROR,
                        PSY_SERIAL_PORT_ERROR_FAILED,
                        "Unable to open device: %s",
                        g_strerror(errno));
        }
        return;
    }

    if (tcgetattr(termios_self->fd, &termios_self->existing_config) < 0
        || tcgetattr(termios_self->fd, &termios_self->config) < 0) {
        g_set_error(error,
                    PSY_SERIAL_PORT_ERROR,
                    PSY_SERIAL_PORT_ERROR_FAILED,
                    "Unable to get current tty config: %s",
                    g_strerror(errno));
        goto failure;
    }

    cfmakeraw(&termios_self->config);

    speed_t rate
        = termios_choose_baud_rate(psy_serial_port_get_baud_rate(self));
    if (cfsetspeed(&termios_self->config, rate) < 0) {
        g_set_error(error,
                    PSY_SERIAL_PORT_ERROR,
                    PSY_SERIAL_PORT_ERROR_FAILED,
                    "Unable to set baudrate: %s",
                    g_strerror(errno));
        return;
    }

    // Set desired timeout
    const PsyDuration *const timeout = psy_serial_port_get_timeout(self);
    termios_self->config.c_cc[VTIME] = find_suitable_timeout(timeout);
    // one decisecond = 100 ms
    PsyDuration *obtained_dur
        = psy_duration_new_ms((gint64) termios_self->config.c_cc[VTIME] * 100);
    psy_serial_port_set_timeout(self, obtained_dur);

    // Set desired minimal num characters
    guint min_chars                 = psy_serial_port_get_num_min_chars(self);
    termios_self->config.c_cc[VMIN] = min_chars;

    if (tcsetattr(termios_self->fd, TCSANOW, &termios_self->config) < 0) {
        g_set_error(error,
                    PSY_SERIAL_PORT_ERROR,
                    PSY_SERIAL_PORT_ERROR_FAILED,
                    "Unable to set the new serial port settings: %s",
                    g_strerror(errno));
    }

    PSY_SERIAL_PORT_CLASS(psy_termios_parent_class)->open(self, error);

    return;

failure:

    close(termios_self->fd);
    termios_self->fd = -1;
}

static void
termios_close(PsySerialPort *self)
{
    PsyTermios *termios_self = PSY_TERMIOS(self);

    if (termios_self->fd >= 0) {

        if (tcsetattr(
                termios_self->fd, TCSAFLUSH, &termios_self->existing_config)
            < 0) {
            g_warning("Unable to reset serial device config: %s",
                      g_strerror(errno));
        }

        if (close(termios_self->fd) < 0) {
            g_critical("Unable to close pid: %s", g_strerror(errno));
        }
        termios_self->fd = -1;
    }

    PSY_SERIAL_PORT_CLASS(psy_termios_parent_class)->close(self);
}

static gsize
termios_write(PsySerialPort *serial,
              const guint8  *bytes,
              gsize          num_bytes,
              GError       **error)
{
    PsyTermios *self = PSY_TERMIOS(serial);

    if (!psy_serial_port_get_is_open(serial)) {
        g_set_error(error,
                    PSY_TYPE_SERIAL_PORT_ERROR,
                    PSY_SERIAL_PORT_ERROR_CLOSED,
                    "Unable to write to a closed serial port");
        return 0;
    }

    gint64 ret = write(self->fd, bytes, num_bytes);
    if (ret < 0) {
        g_set_error(error,
                    PSY_TYPE_SERIAL_PORT_ERROR,
                    PSY_SERIAL_PORT_ERROR_FAILED,
                    "Unable to write to serial port: %s",
                    g_strerror(errno));
    }

    return ret;
}

static void
termios_read(PsySerialPort *serial,
             guint8       **bytes,
             gsize          num_bytes,
             gsize         *num_bytes_read,
             GError       **error)
{
    PsyTermios *self = PSY_TERMIOS(serial);

    // from man termios on Linux:
    // When in canonical mode:
    // The read buffer will only accept 4095 chars; this
    // provides the necessary space for  a  newline  char  if  the  input mode
    // is switched to canonical.
    guint8 buffer[4096];

    gint64 ret = read(self->fd, buffer, MIN(num_bytes, sizeof(buffer)));
    if (ret < 0) {
        g_set_error(error,
                    PSY_TYPE_SERIAL_PORT_ERROR,
                    PSY_SERIAL_PORT_ERROR_FAILED,
                    "Unable to read from serial port: %s",
                    g_strerror(errno));
        *num_bytes_read = 0;
        return;
    }
    else {
        *bytes = g_memdup2(buffer, (gsize) ret);
    }

    *num_bytes_read = ret >= 0 ? ret : 0;
}

static gssize
termios_read_raw(PsySerialPort *serial, guint8 *bytes, gsize num_bytes)
{
    PsyTermios *self = PSY_TERMIOS(serial);

    gssize ret = read(self->fd, bytes, num_bytes);

    return ret;
}

static void
psy_termios_class_init(PsyTermiosClass *cls)
{
    // GObjectClass *obj_cls = G_OBJECT_CLASS(cls);
    //
    // obj_cls->set_property = psy_termios_set_property;
    // obj_cls->get_property = psy_termios_get_property;

    PsySerialPortClass *serial_cls = PSY_SERIAL_PORT_CLASS(cls);

    serial_cls->open     = termios_open;
    serial_cls->close    = termios_close;
    serial_cls->write    = termios_write;
    serial_cls->read     = termios_read;
    serial_cls->read_raw = termios_read_raw;

    // g_object_class_install_properties(obj_cls, NUM_PROPS, termios_props);
}

/**
 * psy_termios_new:(constructor)
 *
 * Returns: a new PsyTermios instance of a PsySerialPort
 */
PsyTermios *
psy_termios_new(void)
{
    return g_object_new(PSY_TYPE_TERMIOS, NULL);
}

/**
 * psy_termios_new_with_name:(constructor)
 * @device_name: the name for the device e.g. /dev/ttyACM0
 *
 * Returns: a new PsyTermios instance of a PsySerialPort
 */
PsyTermios *
psy_termios_new_with_name(const gchar *device_name)
{
    return g_object_new(PSY_TYPE_TERMIOS, "name", device_name, NULL);
}

/**
 * psy_termios_free:(skip)
 *
 * Frees instance created with [ctor@Termios.new]
 */
void
psy_termios_free(PsyTermios *self)
{
    g_return_if_fail(PSY_IS_TERMIOS(self));
    g_object_unref(self);
}

static int
termios_choose_baud_rate(PsyBaudRate rate)
{
    switch (rate) {
    case PSY_BAUD_RATE_50:
        return B50;
    case PSY_BAUD_RATE_75:
        return B75;
    case PSY_BAUD_RATE_110:
        return B110;
    case PSY_BAUD_RATE_134:
        return B134;
    case PSY_BAUD_RATE_150:
        return B150;
    case PSY_BAUD_RATE_200:
        return B200;
    case PSY_BAUD_RATE_300:
        return B300;
    case PSY_BAUD_RATE_600:
        return B600;
    case PSY_BAUD_RATE_1200:
        return B1200;
    case PSY_BAUD_RATE_1800:
        return B1800;
    case PSY_BAUD_RATE_2400:
        return B2400;
    case PSY_BAUD_RATE_4800:
        return B4800;
    case PSY_BAUD_RATE_9600:
        return B9600;
    case PSY_BAUD_RATE_19200:
        return B19200;
    case PSY_BAUD_RATE_38400:
        return B38400;
    case PSY_BAUD_RATE_57600:
        return B57600;
    case PSY_BAUD_RATE_115200:
        return B115200;
    case PSY_BAUD_RATE_230400:
        return B230400;
    default:
        return B0;
    }
}

static guint8
find_suitable_timeout(const PsyDuration *dur)
{
    double seconds      = psy_duration_get_seconds((PsyDuration *) dur);
    double deci_seconds = round(seconds * 10);

    g_assert(seconds >= 0);

    guint8 n_deci_seconds = MIN((guint) deci_seconds, G_MAXUINT8);
    return n_deci_seconds;
}
