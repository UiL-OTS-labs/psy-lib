
#include "psy-serial-port.h"
#include "enum-types.h"
#include "psy-config.h"

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
 * This class does provide the full API of communicating with a serial port
 * SerialPorts in psylib are identified by there number, the id 0 might be
 * mapped to "/dev/parport0/" on linux but "0x378" on windows.
 *
 * TODO Most of these function work synchronous, hence, a class
 * needs to be designed that can read, write, open, close in an async fashion.
 *
 * PsySerial is implemented fully by the classes PsyParport (Linux) and
 * PsyInpoutPort (windows). Using [ctor@PsyParrallelPort.new], you'll get
 * the device that is appropriate on your os, or NULL when not available.
 */

typedef struct {
    gchar port_name[64];
    gint  is_open;
} PsySerialPortPrivate;

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE(PsySerialPort,
                                    psy_serial_port,
                                    G_TYPE_OBJECT)

typedef enum PsySerialPortProperty {
    PROP_NULL,
    PROP_NAME,
    PROP_IS_OPEN,

    PROP_BAUDRATE, // int

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
        g_value_set_boolean(value, psy_serial_port_is_open(self));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, spec);
    }
}

static void
psy_serial_port_init(PsySerialPort *self)
{
    PsySerialPortPrivate *priv = psy_serial_port_get_instance_private(self);

    (void) priv; // Everything should be 0 by default, which is fine.
}

static void
serial_port_finalize(GObject *obj)
{
    PsySerialPort *self = PSY_SERIAL_PORT(obj);

    psy_serial_port_close(self);

    G_OBJECT_CLASS(psy_serial_port_parent_class)->finalize(obj);
}

static void
serial_port_open(PsySerialPort *self, GError **error)
{
    (void) error;
    PsySerialPortPrivate *priv = psy_serial_port_get_instance_private(self);
    priv->is_open              = 1;
}

static void
serial_port_close(PsySerialPort *self)
{
    PsySerialPortPrivate *priv = psy_serial_port_get_instance_private(self);
    priv->port_name[0]         = '\0';
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
    obj_cls->finalize     = serial_port_finalize;

    cls->open          = serial_port_open;
    cls->close         = serial_port_close;
    cls->set_port_name = serial_port_set_port_name;

    /**
     * PsySerialPort:port-name:
     *
     * This is the name of the device at the os level, at linux it might be
     * "/dev/parport0" and at windows "0x378". It should be set when the
     * device is open and should result in an empty string otherwise.
     */
    port_properties[PROP_NAME] = g_param_spec_string(
        "port-name",
        "PortName",
        "The (file/device) name that corresponds with the serial port",
        "",
        G_PARAM_READABLE);

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

    g_object_class_install_properties(obj_cls, NUM_PROPS, port_properties);
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
 */
void
psy_serial_port_open(PsySerialPort *self, GError **error)
{
    g_return_if_fail(PSY_IS_SERIAL_PORT(self));
    g_return_if_fail(error == NULL || *error == NULL);

    PsySerialPortClass *klass = PSY_SERIAL_PORT_GET_CLASS(self);
    g_return_if_fail(klass->open != NULL);

    klass->open(self, error);
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
 * psy_serial_is_open:
 * @self: an instance of PsySerialPort
 *
 * This function may be used to check whether the device is open.
 *
 * Returns: #TRUE when the devices is open, #FALSE otherwise
 */
gboolean
psy_serial_port_is_open(PsySerialPort *self)
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
 * psy_serial_write:
 * @self: an instance of `PsySerialPort`
 * @data:(in)(array length=num_bytes): The bytes that need to be send over
 *                                     the serial connection
 * @num_bytes: the number of bytes that can be written/send.
 * @error: Errors are returned here.
 *
 * This function sends a number of bytes over the serial connection.
 * In practice 0 or more bytes can be send.
 *
 * Returns: The number of bytes written.
 */
gsize
psy_serial_port_write(PsySerialPort *self,
                      const void    *data,
                      gsize          num_bytes,
                      GError       **error)
{
    PsySerialPortClass *cls;
    g_return_val_if_fail(PSY_IS_SERIAL_PORT(self), 0);
    g_return_val_if_fail(error == NULL || *error == NULL, 0);

    cls = PSY_SERIAL_PORT_GET_CLASS(self);
    g_return_val_if_fail(cls->write, 0);
    return cls->write(self, data, num_bytes, error);
}

/**
 * psy_serial_read:
 * @self: an instance of `PsySerialPort`
 * @data: (array length=num_bytes)(out callee-allocates): An array of bytes in
 *        in which the read bytes are returned
 * @num_bytes: The number of bytes that should be read ideally.
 * @error: Errors are returned here.
 *
 * This function read a number of bytes from an opened serial connection. So in
 * order to call this function make sure the device has been opened and
 * configured in the right way and check the number of bytes that are actually
 * read.
 *
 * Returns: The number of bytes read
 */
gsize
psy_serial_port_read(PsySerialPort *self,
                     void          *data,
                     gsize          num_bytes,
                     GError       **error)
{
    PsySerialPortClass *cls;
    g_return_val_if_fail(PSY_IS_SERIAL_PORT(self), 0);
    g_return_val_if_fail(error == NULL || *error == NULL, 0);

    cls = PSY_SERIAL_PORT_GET_CLASS(self);
    g_return_val_if_fail(cls->read, 0);

    return cls->read(self, data, num_bytes, error);
}
