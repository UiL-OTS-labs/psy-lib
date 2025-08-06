
#include "psy-serial-port.h"
#include "enum-types.h"
#include "psy-config.h"
#if defined(HAVE_TERMIOS_H)
    #include "psy-termios.h"
#elif _WIN32
    #include "psy-com-port.h"
#else
    #error "Oops no suitable SerialPort implementations defined"
#endif

// clang-format off
G_DEFINE_QUARK(psy-serial-port-error-quark, psy_serial_port_error)

// clang-format on

/**
 * PsySerialPort:
 *
 * PsySerialPort is a base class for serialports. It is abstract class as
 * different OS use different means of communicating with them. Hence,
 * although, you'll be using the features of this class, you instantiate a
 * class derived from this class.
 * This class does provide the full API of communicating with a serial port.
 *
 * TODO Most of these function work synchronous, hence, a class
 * needs to be designed that can read, write, open, close in an async fashion.
 *
 * PsySerialPort is implemented fully by the [class@Termios] (Linux) and
 * PsyComPort(windows). Using [ctor@SerialPort.new], you'll get
 * the device that is appropriate on your os, or NULL when not available.
 *
 * A serial port of psylib is setup to transmit raw bytes, it is not
 * designed for setting up your own (controlling) terminal. In Linux termios
 * speak, the SerialPort is setup in non-canonical mode. By default the serial
 * device does reads with a timeout of 100ms.
 */

typedef struct {
    gchar        port_name[64];
    gint         is_open;
    PsyBaudRate  baud_rate;
    PsyDuration *timeout;
    guint        min_chars;
} PsySerialPortPrivate;

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE(PsySerialPort,
                                    psy_serial_port,
                                    G_TYPE_OBJECT)

typedef enum PsySerialPortProperty {
    PROP_NULL,
    PROP_NAME,
    PROP_IS_OPEN,

    PROP_BAUD_RATE, // PsyBaudRate
    PROP_TIMEOUT,   // PsyDuration*
    PROP_MIN_CHARS, // guint

    NUM_PROPS,
} PsySerialPortProperty;

static GParamSpec *port_properties[NUM_PROPS];

static void
psy_serial_port_set_property(GObject      *object,
                             guint         property_id,
                             const GValue *value,
                             GParamSpec   *spec)
{
    PsySerialPort *self = PSY_SERIAL_PORT(object);

    switch ((PsySerialPortProperty) property_id) {
    case PROP_NAME:
        psy_serial_port_set_name(self, g_value_get_string(value));
        break;
    case PROP_BAUD_RATE:
        psy_serial_port_set_baud_rate(self, g_value_get_enum(value));
        break;
    case PROP_TIMEOUT:
        psy_serial_port_set_timeout(self, g_value_get_boxed(value));
        break;
    case PROP_MIN_CHARS:
        psy_serial_port_set_num_min_chars(self, g_value_get_uint(value));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, spec);
    }
}

static void
psy_serial_port_get_property(GObject    *object,
                             guint       property_id,
                             GValue     *value,
                             GParamSpec *spec)
{
    PsySerialPort        *self = PSY_SERIAL_PORT(object);
    PsySerialPortPrivate *priv = psy_serial_port_get_instance_private(self);

    switch ((PsySerialPortProperty) property_id) {
    case PROP_NAME:
        g_value_set_string(value, priv->port_name);
        break;
    case PROP_IS_OPEN:
        g_value_set_boolean(value, psy_serial_port_get_is_open(self));
        break;
    case PROP_BAUD_RATE:
        g_value_set_enum(value, psy_serial_port_get_baud_rate(self));
        break;
    case PROP_TIMEOUT:
        g_value_set_boxed(value, psy_serial_port_get_timeout(self));
        break;
    case PROP_MIN_CHARS:
        g_value_set_uint(value, psy_serial_port_get_num_min_chars(self));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, spec);
    }
}

static void
psy_serial_port_init(PsySerialPort *self)
{
    PsySerialPortPrivate *priv = psy_serial_port_get_instance_private(self);

    // Set a default timeout of 0.1 second.
    priv->timeout = psy_duration_new(0.1);
}

static void
serial_port_dispose(GObject *obj)
{
    PsySerialPort *self = PSY_SERIAL_PORT(obj);

    psy_serial_port_close(self);

    G_OBJECT_CLASS(psy_serial_port_parent_class)->dispose(obj);
}

static void
serial_port_finalize(GObject *obj)
{
    PsySerialPort        *self = PSY_SERIAL_PORT(obj);
    PsySerialPortPrivate *priv = psy_serial_port_get_instance_private(self);

    psy_duration_free(priv->timeout);
}

static gboolean
serial_port_open(PsySerialPort *self, GError **error)
{
    (void) error;
    PsySerialPortPrivate *priv = psy_serial_port_get_instance_private(self);
    priv->is_open              = 1;
    return TRUE;
}

static void
serial_port_close(PsySerialPort *self)
{
    PsySerialPortPrivate *priv = psy_serial_port_get_instance_private(self);
    priv->is_open              = 0;
}

static void
serial_port_set_port_name(PsySerialPort *self, const gchar *name)
{
    PsySerialPortPrivate *priv = psy_serial_port_get_instance_private(self);

    g_snprintf(priv->port_name, sizeof(priv->port_name), "%s", name);
}

static void
psy_serial_port_class_init(PsySerialPortClass *cls)
{
    GObjectClass *obj_cls = G_OBJECT_CLASS(cls);

    obj_cls->set_property = psy_serial_port_set_property;
    obj_cls->get_property = psy_serial_port_get_property;
    obj_cls->dispose      = serial_port_dispose;
    obj_cls->finalize     = serial_port_finalize;

    cls->open          = serial_port_open;
    cls->close         = serial_port_close;
    cls->set_port_name = serial_port_set_port_name;

    /**
     * PsySerialPort:name:
     *
     * This is the name of the device at the os level, at linux it might be
     * "/dev/parport0" and at windows "0x378". It should be set when the
     * device is open and should result in an empty string otherwise.
     */
    port_properties[PROP_NAME] = g_param_spec_string(
        "name",
        "Name",
        "The (file/device) name that corresponds with the serial port",
        "",
        G_PARAM_READWRITE);

    /**
     * PsySerialPort:is-open:
     *
     * Returns true when the device is open.
     */
    port_properties[PROP_IS_OPEN]
        = g_param_spec_boolean("is-open",
                               "IsOpen",
                               "Whether or not the port is open",
                               FALSE,
                               G_PARAM_READABLE);

    /**
     * PsySerialPort:baud-rate:
     *
     * The desired in- and output baud rate of the device
     */
    port_properties[PROP_BAUD_RATE] = g_param_spec_enum(
        "baud-rate",
        "BaudRate",
        "The desired in- and output baudrate of the serial port",
        PSY_TYPE_BAUD_RATE,
        PSY_BAUD_RATE_9600,
        G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

    /**
     * PsySerialPort:timeout:
     *
     * The timeout of a read operation. If a read operation is initiated it
     * might block forever, the timeout value makes it return after a limited
     * amount of time.
     */
    port_properties[PROP_TIMEOUT]
        = g_param_spec_boxed("timeout",
                             "Timeout",
                             "The timeout of a read(or write) operation",
                             PSY_TYPE_DURATION,
                             G_PARAM_READWRITE);

    /**
     * PsySerialPort:min-chars:
     *
     * This value reflects the minimal number of bytes that should be received
     * before the read terminates.
     */
    port_properties[PROP_MIN_CHARS] = g_param_spec_uint(
        "min-chars",
        "minchars",
        "the minimal number of bytes to receive before the read terminates",
        0,
        4096,
        0,
        G_PARAM_CONSTRUCT | G_PARAM_READWRITE);

    g_object_class_install_properties(obj_cls, NUM_PROPS, port_properties);
}

/**
 * psy_serial_port_new:(constructor)
 *
 * This constructs a serial device for opening. When termios.h is available
 * it will construct an instance of [class@Termios] otherwise and instance of
 * [class@ComPort]
 * This will return an unopened serial device, for which the name still needs
 * to be set before opening.
 *
 * Returns: an instance of [class@SerialPort]
 */
PsySerialPort *
psy_serial_port_new(void)
{
    PsySerialPort *ret = NULL;

#if defined(HAVE_TERMIOS_H)
    ret = PSY_SERIAL_PORT(psy_termios_new());
#elif _WIN32
    ret = PSY_SERIAL_PORT(psy_com_port_new());
#else
    g_warning("No Serial port available for this platform");
#endif
    return ret;
}

/**
 * psy_serial_port_new_with_name:(constructor)
 * @device_name: the name for the device e.g. COM1 or /dev/ttyACM0
 *
 * This constructs a serial device for opening. When termios.h is available
 * it will construct an instance of [class@Termios] otherwise and instance of
 * [class@ComPort]
 *
 * Returns: an instance of [class@SerialPort]
 */
PsySerialPort *
psy_serial_port_new_with_name(const gchar *device_name)
{
    g_return_val_if_fail(device_name, NULL);
    PsySerialPort *ret = NULL;

#if defined(HAVE_TERMIOS_H)
    ret = PSY_SERIAL_PORT(psy_termios_new_with_name(device_name));
#elif _WIN32
    ret = PSY_SERIAL_PORT(psy_com_port_new_with_name(device_name));
#else
    #warning "No SerialPort available for this platform"
#endif
    return ret;
}

/**
 * psy_serial_port_free:(skip)
 *
 * Frees instance created with [ctor@SerialPort.new]
 */
void
psy_serial_port_free(PsySerialPort *self)
{
    g_return_if_fail(PSY_IS_SERIAL_PORT(self));
    g_object_unref(self);
}

/**
 * psy_serial_port_open:
 * @self: an instance of PsySerialPort
 * @error:(out): Errors are returned here.
 *
 * Opens the device with the, make sure you have set the port name, the
 * device file is backed by the actual device.
 *
 * Returns: TRUE if the serial port is opened, false otherwise
 */
gboolean
psy_serial_port_open(PsySerialPort *self, GError **error)
{
    g_return_val_if_fail(PSY_IS_SERIAL_PORT(self), FALSE);
    g_return_val_if_fail(error == NULL || *error == NULL, FALSE);

    PsySerialPortClass *klass = PSY_SERIAL_PORT_GET_CLASS(self);
    g_return_val_if_fail(klass->open != NULL, FALSE);

    return klass->open(self, error);
}

/**
 * psy_serial_port_close:
 * @self: an instance of PsySerialPort
 *
 * Closes the device when it's opened. This releases some of the resources
 * related to opening the device. When the device is destroyed, it will
 * also be closed.
 */
void
psy_serial_port_close(PsySerialPort *self)
{
    g_return_if_fail(PSY_IS_SERIAL_PORT(self));
    PsySerialPortPrivate *priv = psy_serial_port_get_instance_private(self);

    if (!priv->is_open) {
        return;
    }

    PsySerialPortClass *klass = PSY_SERIAL_PORT_GET_CLASS(self);
    g_return_if_fail(klass->close != NULL);

    klass->close(self);
}

/**
 * psy_serial_port_get_is_open:
 * @self: an instance of PsySerialPort
 *
 * This function may be used to check whether the device is open.
 *
 * Returns: #TRUE when the devices is open, #FALSE otherwise
 */
gboolean
psy_serial_port_get_is_open(PsySerialPort *self)
{
    g_return_val_if_fail(PSY_IS_SERIAL_PORT(self), FALSE);

    PsySerialPortPrivate *priv = psy_serial_port_get_instance_private(self);

    return priv->is_open;
}

/**
 * psy_serial_port_set_name:
 * @self: an instance of `PsySerialPort`
 * @name: The name of the file or com port to open
 *
 * Set the device name of the serial port. The device name is set when
 * the device is opened with an id. E.g. COM1 on windows or /dev/ttyACM0 or
 * /dev/ttyUSB0 on Linux
 */
void
psy_serial_port_set_name(PsySerialPort *self, const gchar *name)
{
    PsySerialPortPrivate *priv = psy_serial_port_get_instance_private(self);

    g_return_if_fail(PSY_IS_SERIAL_PORT(self));

    g_snprintf(priv->port_name, sizeof(priv->port_name), "%s", name);
}

/**
 * psy_serial_port_get_name:
 * @self: an instance of `PsySerialPort`
 *
 * Get the device name of the serial port. The device name is set when
 * the device is opened with an id. E.g. COM1 on windows or /dev/ttyACM0 or
 * /dev/ttyUSB0 on Linux
 *
 * Returns: a string with the name of the device that will be used to open the
 * file
 */
const gchar *
psy_serial_port_get_name(PsySerialPort *self)
{
    PsySerialPortPrivate *priv = psy_serial_port_get_instance_private(self);

    g_return_val_if_fail(PSY_IS_SERIAL_PORT(self), NULL);

    return priv->port_name;
}

/**
 * psy_serial_port_write:
 * @self: an instance of `PsySerialPort`
 * @bytes:(in)(array length=num_bytes) (element-type guint8): The bytes that
 * need to be send over the serial connection
 * @num_bytes: the number of bytes that can be written/send.
 * @error: Errors are returned here.
 *
 * This function sends a number of bytes over the serial connection.
 * In practice 0 or more bytes can be send.
 *
 * Returns: The number of bytes written.
 */
gssize
psy_serial_port_write(PsySerialPort *self,
                      const guint8  *bytes,
                      gsize          num_bytes,
                      GError       **error)
{
    PsySerialPortClass *cls;
    g_return_val_if_fail(PSY_IS_SERIAL_PORT(self), 0);
    g_return_val_if_fail(error == NULL || *error == NULL, 0);

    cls = PSY_SERIAL_PORT_GET_CLASS(self);
    g_return_val_if_fail(cls->write, 0);
    return cls->write(self, bytes, num_bytes, error);
}

/**
 * psy_serial_port_read:
 * @self: an instance of `PsySerialPort`
 * @bytes: (array length=num_bytes_read)(out callee-allocates)(transfer full):
 *      An array of bytes in in which the read bytes are returned
 * @num_bytes: The number of bytes that should be read ideally.
 * @num_bytes_read: (out caller-allocates): The number of bytes that are read.
 * @error: Errors are returned here.
 *
 * This function read a number of bytes from an opened serial connection. So in
 * order to call this function make sure the device has been opened and
 * configured in the right way and check the number of bytes that are actually
 * read.
 */
void
psy_serial_port_read(PsySerialPort *self,
                     guint8       **bytes,
                     gsize          num_bytes,
                     gsize         *num_bytes_read,
                     GError       **error)
{
    PsySerialPortClass   *cls;
    PsySerialPortPrivate *priv;
    g_return_if_fail(bytes != NULL && *bytes == NULL);
    g_return_if_fail(PSY_IS_SERIAL_PORT(self));
    g_return_if_fail(error == NULL || *error == NULL);
    g_return_if_fail(num_bytes_read != NULL);

    cls = PSY_SERIAL_PORT_GET_CLASS(self);
    g_return_if_fail(cls->read);

    priv = psy_serial_port_get_instance_private(self);
    if (!priv->is_open) {
        g_set_error(error,
                    PSY_SERIAL_PORT_ERROR,
                    PSY_SERIAL_PORT_ERROR_CLOSED,
                    "Unable to read from closed serial device");
        *num_bytes_read = 0;
        return;
    }

    cls->read(self, bytes, num_bytes, num_bytes_read, error);
}

/**
 * psy_serial_port_read_raw:(skip)
 * @bytes:(out caller-allocates):The bytes are going to be read into this
 *      buffer, hence it should be at least num_bytes large
 * @num_bytes: The number of bytes you'd like to read.
 *
 * Read num_bytes from the serial port. This function is currently not exported
 * to the language bindings as then is hard to transfer the bytes to a bytes
 * like object in the language from which psylib is called.
 *
 * Returns: the number of bytes read successfully from the serial port or a
 * negative number on failure in which case errno should be set.
 */
gssize
psy_serial_port_read_raw(PsySerialPort *self, guint8 *bytes, gsize num_bytes)
{
    PsySerialPortClass *cls;
    g_return_val_if_fail(bytes != NULL, -1);
    g_return_val_if_fail(PSY_IS_SERIAL_PORT(self), -1);

    PsySerialPortPrivate *priv = psy_serial_port_get_instance_private(self);
    g_return_val_if_fail(priv->is_open, -1);

    cls = PSY_SERIAL_PORT_GET_CLASS(self);
    g_return_val_if_fail(cls->read_raw, -1);

    if (num_bytes > G_MAXSSIZE)
        num_bytes = G_MAXSSIZE;

    return cls->read_raw(self, bytes, num_bytes);
}

/**
 * psy_serial_port_set_baud_rate:
 * @self: an instance of [class@SerialPort]
 * @rate: the desired baudrate for the port
 *
 * Sets the desired baudrate for the in- and output for the serial port.
 * These settings are used to open the serial port with. On Linux when
 * setting the baudrate to [enum@PsyBaudRate.PSY_BAUD_RATE_0] means to
 * close the connection.
 */
void
psy_serial_port_set_baud_rate(PsySerialPort *self, PsyBaudRate rate)
{
    g_return_if_fail(PSY_IS_SERIAL_PORT(self));

    PsySerialPortPrivate *priv = psy_serial_port_get_instance_private(self);
    priv->baud_rate            = rate;
}

/**
 * psy_serial_port_get_baud_rate:
 * @self: An instance of [class@SerialPort]
 *
 * The baudrate on which the device operates. The actual bitrate
 * is slightly lower, as some bits are necessary for the protocol
 *
 * Returns: the baud rate used to open the connection
 */
PsyBaudRate
psy_serial_port_get_baud_rate(PsySerialPort *self)
{
    g_return_val_if_fail(PSY_IS_SERIAL_PORT(self), PSY_BAUD_RATE_0);

    PsySerialPortPrivate *priv = psy_serial_port_get_instance_private(self);
    return priv->baud_rate;
}

/**
 * psy_serial_port_set_timeout:
 * @self: an instance of [class@SerialPort]
 * @duration:(transfer none): The maximum duration before returning even if less
 *            than the specified number of bytes has been read. The duration
 * should be larger or equal to 0.
 *
 * You can use this value to specify the maximum time before returning. This
 * method will probably always succeed, but you might want to check the value
 * you have been actually obtained. By checking [property@SerialPort:timeout]
 * or calling [method@SerialPort.get_timeout] after the port has been opened.
 */
void
psy_serial_port_set_timeout(PsySerialPort *self, PsyDuration *duration)
{
    g_return_if_fail(PSY_IS_SERIAL_PORT(self));
    g_return_if_fail(duration != NULL);
    g_return_if_fail(psy_duration_get_seconds(duration) >= 0);

    PsySerialPortPrivate *priv = psy_serial_port_get_instance_private(self);

    g_clear_pointer(&priv->timeout, psy_duration_free);
    priv->timeout = psy_duration_copy(duration);
}

/**
 * psy_serial_port_get_timeout:
 * @self: an instance of [class@SerialPort]
 *
 * You can use this value to specify the maximum time before returning. This
 * method will probably always succeed, but you might want to check the value
 * you have been actually obtained. By checking [property@SerialPort:timeout]
 * or calling [method@SerialPort.get_timeout] after the port has been opened.
 *
 * Returns: (transfer none): The duration of the read timeout. The duration on
 * the Serial port might change after opening the serial port, as during opening
 * the serial port chooses a value that is usable by the device and backend.
 */
PsyDuration *
psy_serial_port_get_timeout(PsySerialPort *self)
{
    g_return_val_if_fail(PSY_IS_SERIAL_PORT(self), NULL);
    PsySerialPortPrivate *priv = psy_serial_port_get_instance_private(self);

    return priv->timeout;
}

/**
 * psy_serial_port_set_num_min_chars:
 * @self
 * @num_chars: the minimal number of char to receive before read terminates
 *      the maximum value for this is 4096,
 *
 */
void
psy_serial_port_set_num_min_chars(PsySerialPort *self, guint num_chars)
{
    g_return_if_fail(PSY_IS_SERIAL_PORT(self));
    PsySerialPortPrivate *priv = psy_serial_port_get_instance_private(self);

    if (G_UNLIKELY(num_chars > 4096)) {
        num_chars = 4096;
    }

    priv->min_chars = num_chars;
}

/**
 * psy_serial_port_get_num_min_chars:
 * @self: an instance of [class@SerialPort]
 *
 */
guint
psy_serial_port_get_num_min_chars(PsySerialPort *self)
{
    g_return_val_if_fail(PSY_IS_SERIAL_PORT(self), -1);
    PsySerialPortPrivate *priv = psy_serial_port_get_instance_private(self);

    return priv->min_chars;
}
