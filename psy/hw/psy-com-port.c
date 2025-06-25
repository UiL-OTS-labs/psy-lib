
#include <errno.h>
#include <math.h>

#include "enum-types.h"
#include "psy-com-port.h"
#include "psy-config.h"
#include "psy-utils.h"

#if defined(HAVE_TERMIOS_H)
    #include <termios.h>
    // let assume we have to headers below too
    #include <fcntl.h>
    #include <sys/stat.h>
    #include <sys/types.h>
    #include <unistd.h>
#elif _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include "windows.h"
#else
    #error                                                                     \
        "Oops, trying to compile psy-com_port.c on a system without com_port.h";
#endif

/**
 * PsyComPort:
 *
 * This is an implementation of a Serial port on Windows that has
 *
 * Suitable devices for this class are typically: COMx where x
 * is an integer larger than 0
 */

// forward declaration
static DWORD
com_port_choose_baud_rate(PsyBaudRate rate);

typedef struct _PsyComPort {
    PsySerialPort parent;
    HANDLE        h_file;

    DCB existing_config;
    DCB config;
} PsyComPort;

G_DEFINE_FINAL_TYPE(PsyComPort, psy_com_port, PSY_TYPE_SERIAL_PORT)

typedef enum PsyComPortProperty {
    PROP_NULL,
    NUM_PROPS,
} PsyComPortProperty;

// // Currently PsyComPort hasn't got any properties
// static GParamSpec *com_port_props[NUM_PROPS];
//
// static void
// psy_com_port_set_property(GObject      *object,
//                          guint         property_id,
//                          const GValue *value,
//                          GParamSpec   *spec)
// {
//     PsyComPort *self = PSY_COM_PORT(object);
//     (void) self;
//     (void) value;
//
//     switch ((PsyComPortProperty) property_id) {
//     default:
//         G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, spec);
//     }
// }
//
// static void
// psy_com_port_get_property(GObject    *object,
//                          guint       property_id,
//                          GValue     *value,
//                          GParamSpec *spec)
// {
//     PsyComPort *self = PSY_COM_PORT(object);
//     (void) self;
//     (void) value;
//
//     switch ((PsyComPortProperty) property_id) {
//     default:
//         G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, spec);
//     }
// }

static void
psy_com_port_init(PsyComPort *self)
{
    (void) self;
}

/*
 * Try to open the serial device's file. Stores the current configuration of the
 * serial port, in order to restore it on close, so it appears that psylib
 * hasn't touched the device. Then it sets the serial it tries to open the
 * serial port.
 */
static gboolean
com_port_open(PsySerialPort *serial, GError **error)
{
    PsyComPort *self = PSY_COM_PORT(serial);

    memset(&self->config, 0, sizeof(self->config));
    memset(&self->existing_config, 0, sizeof(self->existing_config));
    self->config.DCBlength          = sizeof(self->config);
    self->existing_config.DCBlength = sizeof(self->existing_config);

    self->h_file = CreateFile(psy_serial_port_get_name(serial),
                              GENERIC_READ | GENERIC_WRITE,
                              0,
                              NULL,
                              OPEN_EXISTING,
                              0,
                              NULL);

    if (self->h_file == INVALID_HANDLE_VALUE) {
        char error_buf[1024];
        int  errsave = GetLastError();

        psy_strerr(errsave, error_buf, sizeof(error_buf));

        PsySerialPortError serial_error;
        switch (errsave) {
        case ERROR_FILE_NOT_FOUND:
        case ERROR_PATH_NOT_FOUND:
            serial_error = PSY_SERIAL_PORT_ERROR_NO_SUCH_DEVICE;
            break;
        case ERROR_ACCESS_DENIED:
            serial_error = PSY_SERIAL_PORT_ERROR_NO_PERMISSION;
            break;
        default:
            serial_error = PSY_SERIAL_PORT_ERROR_FAILED;
        }

        g_set_error(error,
                    PSY_SERIAL_PORT_ERROR,
                    serial_error,
                    "Unable to open %s: %s",
                    psy_serial_port_get_name(serial),
                    error_buf);
        return FALSE;
    }

    BOOL success1, success2;
    success1 = GetCommState(self->h_file, &self->existing_config);
    success2 = GetCommState(self->h_file, &self->config);

    if (!success1 || !success2) {
        int  errsave = GetLastError();
        char errstr[1024];
        psy_strerr(errsave, errstr, sizeof(errstr));
        g_set_error(error,
                    PSY_SERIAL_PORT_ERROR,
                    PSY_SERIAL_PORT_ERROR_FAILED,
                    "Unable to get current CommState: %s",
                    errstr);
        goto failure;
    }

    self->config.BaudRate
        = com_port_choose_baud_rate(psy_serial_port_get_baud_rate(serial));
    self->config.ByteSize = 8;
    self->config.Parity   = NOPARITY;
    self->config.StopBits = ONESTOPBIT;

    if (!SetCommState(self->h_file, &self->config)) {
        int  errsave = GetLastError();
        char errstr[1024];
        psy_strerr(errsave, errstr, sizeof(errstr));
        g_set_error(error,
                    PSY_SERIAL_PORT_ERROR,
                    PSY_SERIAL_PORT_ERROR_FAILED,
                    "Unable to set CommState: %s",
                    errstr);
        goto failure;
    }

    PsyDuration *timeout_dur           = psy_serial_port_get_timeout(serial);
    COMMTIMEOUTS timeout               = {0};
    timeout.ReadIntervalTimeout        = psy_duration_get_ms(timeout_dur);
    timeout.ReadTotalTimeoutMultiplier = 0;
    timeout.ReadTotalTimeoutConstant   = psy_duration_get_ms(timeout_dur);

    SetCommTimeouts(self->h_file, &timeout);

    return PSY_SERIAL_PORT_CLASS(psy_com_port_parent_class)
        ->open(serial, error);

failure:

    CloseHandle(self->h_file);
    self->h_file = NULL;
    return FALSE;
}

static void
com_port_close(PsySerialPort *serial)
{
    PsyComPort *self = PSY_COM_PORT(serial);

    if (self->h_file != NULL) {

        // Reset com configuration
        if (!SetCommState(self->h_file, &self->existing_config)) {
            char errstr[1024];
            int  errsave = GetLastError();
            psy_strerr(errsave, errstr, sizeof(errstr));
            g_warning("Unable to reset serial device config: %s", errstr);
        }

        if (!CloseHandle(self->h_file)) {
            char errstr[1024];
            int  errsave = GetLastError();
            psy_strerr(errsave, errstr, sizeof(errstr));
            g_critical("Unable to close pid: %s", errstr);
        }
        self->h_file = NULL;
    }

    PSY_SERIAL_PORT_CLASS(psy_com_port_parent_class)->close(serial);
}

static gssize
com_port_write(PsySerialPort *serial,
               const guint8  *bytes,
               gsize          num_bytes,
               GError       **error)
{
    PsyComPort *self = PSY_COM_PORT(serial);

    if (!psy_serial_port_get_is_open(serial)) {
        g_set_error(error,
                    PSY_TYPE_SERIAL_PORT_ERROR,
                    PSY_SERIAL_PORT_ERROR_CLOSED,
                    "Unable to write to a closed serial port");
        return 0;
    }

    DWORD nwritten = 0;

    BOOL ret = WriteFile(self->h_file, bytes, num_bytes, &nwritten, NULL);
    if (!ret) {
        char errstr[1024];
        int  errsave = GetLastError();
        psy_strerr(errsave, errstr, sizeof(errstr));
        g_set_error(error,
                    PSY_TYPE_SERIAL_PORT_ERROR,
                    PSY_SERIAL_PORT_ERROR_FAILED,
                    "Unable to write to serial port: %s",
                    errstr);
    }

    return nwritten;
}

static void
com_port_read(PsySerialPort *serial,
              guint8       **bytes,
              gsize          num_bytes,
              gsize         *num_bytes_read,
              GError       **error)
{
    PsyComPort *self = PSY_COM_PORT(serial);

    // from man com_port on Linux:
    // When in canonical mode:
    // The read buffer will only accept 4095 chars; this
    // provides the necessary space for a newline char if the input mode
    // is switched to canonical.
    guint8 buffer[4096];
    DWORD  nread;

    BOOL ret = ReadFile(
        self->h_file, buffer, MIN(num_bytes, sizeof(buffer)), &nread, NULL);
    if (!ret) {
        char errstr[1024];
        int  errsave = GetLastError();
        psy_strerr(errsave, errstr, sizeof(errstr));
        g_set_error(error,
                    PSY_TYPE_SERIAL_PORT_ERROR,
                    PSY_SERIAL_PORT_ERROR_FAILED,
                    "Unable to read from serial port: %s",
                    errstr);
        *num_bytes_read = 0;
        return;
    }
    else {
        *bytes = g_memdup2(buffer, nread);
    }

    *num_bytes_read = nread;
}

static gssize
com_port_read_raw(PsySerialPort *serial, guint8 *bytes, gsize num_bytes)
{
    PsyComPort *self  = PSY_COM_PORT(serial);
    DWORD       nread = 0;

    BOOL ret = ReadFile(self->h_file, bytes, num_bytes, &nread, NULL);

    return ret ? (gssize) nread : -1;
}

static void
psy_com_port_class_init(PsyComPortClass *cls)
{
    // GObjectClass *obj_cls = G_OBJECT_CLASS(cls);
    //
    // obj_cls->set_property = psy_com_port_set_property;
    // obj_cls->get_property = psy_com_port_get_property;

    PsySerialPortClass *serial_cls = PSY_SERIAL_PORT_CLASS(cls);

    serial_cls->open     = com_port_open;
    serial_cls->close    = com_port_close;
    serial_cls->write    = com_port_write;
    serial_cls->read     = com_port_read;
    serial_cls->read_raw = com_port_read_raw;

    // g_object_class_install_properties(obj_cls, NUM_PROPS,
    // com_port_props);
}

/**
 * psy_com_port_new:(constructor)
 *
 * Returns: a new PsyComPort instance of a PsySerialPort
 */
PsyComPort *
psy_com_port_new(void)
{
    return g_object_new(PSY_TYPE_COM_PORT, NULL);
}

/**
 * psy_com_port_new_with_name:(constructor)
 * @device_name: the name for the device e.g. /dev/ttyACM0
 *
 * Returns: a new PsyComPort instance of a PsySerialPort
 */
PsyComPort *
psy_com_port_new_with_name(const gchar *device_name)
{
    return g_object_new(PSY_TYPE_COM_PORT, "name", device_name, NULL);
}

/**
 * psy_com_port_free:(skip)
 *
 * Frees instance created with [ctor@ComPort.new]
 */
void
psy_com_port_free(PsyComPort *self)
{
    g_return_if_fail(PSY_IS_COM_PORT(self));
    g_object_unref(self);
}

DWORD
com_port_choose_baud_rate(PsyBaudRate rate)
{
    switch (rate) {
    case PSY_BAUD_RATE_50:
        return 50;
    case PSY_BAUD_RATE_75:
        return 75;
    case PSY_BAUD_RATE_110:
        return CBR_110;
    case PSY_BAUD_RATE_134:
        return 134;
    case PSY_BAUD_RATE_150:
        return 150;
    case PSY_BAUD_RATE_200:
        return 200;
    case PSY_BAUD_RATE_300:
        return CBR_300;
    case PSY_BAUD_RATE_600:
        return CBR_600;
    case PSY_BAUD_RATE_1200:
        return CBR_1200;
    case PSY_BAUD_RATE_1800:
        return 1800;
    case PSY_BAUD_RATE_2400:
        return CBR_2400;
    case PSY_BAUD_RATE_4800:
        return CBR_4800;
    case PSY_BAUD_RATE_9600:
        return CBR_9600;
    case PSY_BAUD_RATE_19200:
        return CBR_19200;
    case PSY_BAUD_RATE_38400:
        return CBR_38400;
    case PSY_BAUD_RATE_57600:
        return CBR_57600;
    case PSY_BAUD_RATE_115200:
        return CBR_115200;
    case PSY_BAUD_RATE_230400:
        return 230400;
    default:
        return 0;
    }
}
