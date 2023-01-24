
#include "psy-parallel-port.h"
#include "enum-types.h"
#include "psy-config.h"
#if defined(HAVE_LINUX_PARPORT_H)
    #include "psy-parport.h"
#endif

// clang-format off
G_DEFINE_QUARK(psy-parallel-port-error-quark, psy_parallel_port_error)

// clang-format on

/**
 * PsyParallelPort:
 *
 * PsyParallelPort is a base class for parallelports. It is abstract class as
 * different OS use different means of communicating with them. Hence,
 * although, you'll be using the features of this class, you instantiate a
 * class derived from this class.
 * This class does provide the full API of communicating with a parallel port
 * ParallelPorts in psylib are identified by there number, the id 0 might be
 * mapped to "/dev/parport0/" on linux but "LPT1" on windows.
 *
 * TODO Most of these function work synchronous, hence, a class
 * needs to be designed that can read, write, open, close in an async fashion.
 */

typedef struct {
    gchar          port_name[64];
    gint           port_num; // -1 closed or 0 - max_port for a open one.
    PsyIoDirection direction;
    guint8         pins; // the lines as written to or read from the device.
} PsyParallelPortPrivate;

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE(PsyParallelPort,
                                    psy_parallel_port,
                                    G_TYPE_OBJECT)

typedef enum PsyParallelPortProperty {
    PROP_NULL,
    PORT_NUM,
    PORT_NAME,
    PORT_DIRECTION,
    PORT_PINS,
    NUM_PROPS,
} PsyParallelPortProperty;

static GParamSpec *port_properties[NUM_PROPS];

static void
psy_parallel_port_set_property(GObject      *object,
                               guint         property_id,
                               const GValue *value,
                               GParamSpec   *spec)
{
    PsyParallelPort        *self = PSY_PARALLEL_PORT(object);
    PsyParallelPortPrivate *priv = psy_parallel_port_get_instance_private(self);

    switch ((PsyParallelPortProperty) property_id) {
    case PORT_DIRECTION:
        psy_parallel_port_set_direction(self, g_value_get_enum(value));
        break;
    case PORT_PINS:
    {
        GError *error = NULL;
        guint8  pins  = (guint8) g_value_get_uint(value);
        psy_parallel_port_write(self, pins, &error);
        if (error) {
            g_critical("unable to set \"pins\": %s", error->message);
            g_error_free(error);
        }
        break;
    }
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, spec);
    }
}

static void
psy_parallel_port_get_property(GObject    *object,
                               guint       property_id,
                               GValue     *value,
                               GParamSpec *spec)
{
    PsyParallelPort        *self = PSY_PARALLEL_PORT(object);
    PsyParallelPortPrivate *priv = psy_parallel_port_get_instance_private(self);

    switch ((PsyParallelPortProperty) property_id) {
    case PORT_NUM:
        g_value_set_int(value, priv->port_num);
        break;
    case PORT_NAME:
        g_value_set_string(value, priv->port_name);
        break;
    case PORT_DIRECTION:
    {
        gint dir = priv->direction;
        g_value_set_enum(value, dir);
        break;
    }
    case PORT_PINS:
    {
        gboolean is_output
            = psy_parallel_port_get_direction(self) == PSY_IO_DIRECTION_OUT;
        if (is_output) {
            g_value_set_uint(value, priv->pins);
        }
        else {
            GError *error = NULL;
            guint   pins  = psy_parallel_port_read(self, &error);
            if (error) {
                g_critical("unable to read pins: %s", error->message);
                g_error_free(error);
            }
            g_value_set_uint(value, pins);
        }
        break;
    }
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, spec);
    }
}

static void
psy_parallel_port_init(PsyParallelPort *self)
{
    PsyParallelPortPrivate *priv = psy_parallel_port_get_instance_private(self);

    priv->port_num  = -1;
    priv->direction = PSY_IO_DIRECTION_OUT;
}

static void
parallel_port_finalize(GObject *obj)
{
    PsyParallelPort *self = PSY_PARALLEL_PORT(obj);

    psy_parallel_port_close(self);

    G_OBJECT_CLASS(psy_parallel_port_parent_class)->finalize(obj);
}

static void
parallel_port_open(PsyParallelPort *self, gint port_num, GError **error)
{
    (void) error;
    PsyParallelPortPrivate *priv = psy_parallel_port_get_instance_private(self);

    priv->port_num = port_num;
}

static void
parallel_port_close(PsyParallelPort *self)
{
    PsyParallelPortPrivate *priv = psy_parallel_port_get_instance_private(self);
    priv->port_num               = -1;
    priv->port_name[0]           = '\0';
}

static void
parallel_port_set_port_name(PsyParallelPort *self, const gchar *name)
{
    PsyParallelPortPrivate *priv = psy_parallel_port_get_instance_private(self);

    g_snprintf(priv->port_name, sizeof(priv->port_name), "%s", name);
}

static void
psy_parallel_port_class_init(PsyParallelPortClass *cls)
{
    GObjectClass *obj_cls = G_OBJECT_CLASS(cls);

    obj_cls->set_property = psy_parallel_port_set_property;
    obj_cls->get_property = psy_parallel_port_get_property;
    obj_cls->finalize     = parallel_port_finalize;

    cls->open          = parallel_port_open;
    cls->close         = parallel_port_close;
    cls->set_port_name = parallel_port_set_port_name;

    /**
     * PsyParallelPort:port-num:
     *
     * The number of device used for opening the parallel port. It is
     * specified using `psy_parallel_port_open`, hence it is read only
     * and should be -1 when closed.
     */
    port_properties[PORT_NUM]
        = g_param_spec_int("port-num",
                           "PortNumber",
                           "The number of the port device to use",
                           -1,
                           16,
                           -1,
                           G_PARAM_READABLE);

    /**
     * PsyParallelPort:port-name:
     *
     * This is the name of the device at the os level, at linux it might be
     * "/dev/parport0" and at windows "LPT1". It should be set when the
     * device is open and should result in an empty string otherwise.
     */
    port_properties[PORT_NAME] = g_param_spec_string(
        "port-name",
        "PortName",
        "The (file/device) name that corresponds with the parallel port",
        "",
        G_PARAM_READABLE);

    /**
     * PsyParallelPort:direction:
     *
     * Whether or not the device is configured as an input or an output device.
     * You may also use this attribute to change the direction of the
     * parallel port.
     */
    port_properties[PORT_DIRECTION]
        = g_param_spec_enum("direction",
                            "Direction",
                            "The in- or output direction of this device",
                            PSY_TYPE_IO_DIRECTION,
                            PSY_IO_DIRECTION_OUT,
                            G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

    /**
     * PsyParallelPort:pins:
     *
     * When writing to this property, you'll have to make sure the device
     * is configured as output as you try to update the lines. When reading
     * you'll be reading from the Port it self, when the port is configured as
     * output, you'll just get the most recent value written to the port.
     */
    port_properties[PORT_PINS] = g_param_spec_uint(
        "pins",
        "Pins",
        "A bit mask representing whether the lines are HIGH or LOW",
        0,
        255,
        0,
        G_PARAM_READWRITE);

    g_object_class_install_properties(obj_cls, NUM_PROPS, port_properties);
}

/**
 * psy_parallel_port_new:(constructor)
 *
 * Creates a new PsyParallelPort and returns it. The port returned will
 * be an instance of PsyParallelPort, but it will be a derived class for
 * a specific backend. E.g. an instance of PsyParport on Linux and another
 * class on Windows.
 * You may also use `g_object_new(PSY_TYPE_PARPORT, NULL)` to create a new
 * device, but that is perhaps not so handy. This function will create a
 * ParallelPort device for any supported
 *
 * Returns: an Derived instance of `PsyParallelPort`, or NULL when there
 *    is no backend for parallel ports.
 */
PsyParallelPort *
psy_parallel_port_new(void)
{
    PsyParallelPort *port = NULL;
#if defined(HAVE_LINUX_PARPORT_H)

    port = g_object_new(PSY_TYPE_PARPORT, NULL);

#else
    #pragma message "No instance for a parallel port"
#endif

    return port;
}

/**
 * psy_parallel_port_open:
 * @self: an instance of PsyParallelPort
 * @dev_num: The nth device to open.
 * @error:(out): Errors are returned here.
 *
 * Opens the device with the nth_device num, numbering starts at 0 until
 * the number of parallel ports known to the os.
 */
void
psy_parallel_port_open(PsyParallelPort *self, gint dev_num, GError **error)
{
    g_return_if_fail(PSY_IS_PARALLEL_PORT(self));
    g_return_if_fail(error == NULL || *error == NULL);

    PsyParallelPortClass *klass = PSY_PARALLEL_PORT_GET_CLASS(self);
    g_return_if_fail(klass->open != NULL);

    klass->open(self, dev_num, error);
}

/**
 * psy_parallel_port_close:
 * @self: an instance of PsyParallelPort
 *
 * Closes the device when it's opened. This releases some of the resources
 * related to opening the device. When the device is destroyed, it will
 * also be closed.
 */
void
psy_parallel_port_close(PsyParallelPort *self)
{
    g_return_if_fail(PSY_IS_PARALLEL_PORT(self));

    PsyParallelPortClass *klass = PSY_PARALLEL_PORT_GET_CLASS(self);
    g_return_if_fail(klass->open != NULL);

    klass->close(self);
}

/**
 * psy_parallel_is_open:
 * @self: an instance of PsyParallelPort
 *
 * This function may be used to check whether the device is open.
 *
 * Returns: #TRUE when the devices is open, #FALSE otherwise
 */
gboolean
psy_parallel_port_is_open(PsyParallelPort *self)
{
    g_return_val_if_fail(PSY_IS_PARALLEL_PORT(self), FALSE);

    PsyParallelPortPrivate *priv = psy_parallel_port_get_instance_private(self);

    return priv->port_num >= 0;
}

/**
 * psy_parallel_get_port_name:
 * @self: an instance of `PsyParallelPort`
 *
 * Get the device name of the parallel port. The device name is set when the
 * device is opened with an id. For example, opening device with port_num = 0
 * will likely result in a port name of "/dev/parport0" on linux and "LPT1" on
 * windows.
 *
 * Returns: a string with the name of the device that has been opened or
 *          an empty string otherwise.
 */
const gchar *
psy_parallel_port_get_port_name(PsyParallelPort *self)
{
    PsyParallelPortPrivate *priv = psy_parallel_port_get_instance_private(self);

    g_return_val_if_fail(PSY_IS_PARALLEL_PORT(self), NULL);

    return priv->port_name;
}

/**
 * psy_parallel_set_direction:
 * @self: an instance of PsyParallelPort
 * @dir: the direction of operation, PsyLib support's writing or reading
 *       from the datalines of a parallel port.
 *
 * This function may be used to set/get the direction in which the operation
 * of the port is desired.
 */
void
psy_parallel_port_set_direction(PsyParallelPort *self, PsyIoDirection dir)
{
    g_return_if_fail(PSY_IS_PARALLEL_PORT(self));

    PsyParallelPortPrivate *priv = psy_parallel_port_get_instance_private(self);

    priv->direction = dir;
}

/**
 * psy_parallel_get_direction:
 * @self: an instance of PsyParallelPort
 *
 * This function may be used to set/get the direction in which the operation
 * of the port is desired.
 *
 * Returns: the current configuration whether the port is configured as input
 *          or an output.
 */
PsyIoDirection
psy_parallel_port_get_direction(PsyParallelPort *self)
{
    g_return_val_if_fail(PSY_IS_PARALLEL_PORT(self), PSY_IO_DIRECTION_OUT);

    PsyParallelPortPrivate *priv = psy_parallel_port_get_instance_private(self);

    return priv->direction;
}

/**
 * psy_parallel_port_is_output:
 * @self: An instance of `PsyParallelPort`
 *
 * Returns whether or not the devices is configured as output.
 *
 * Returns: #TRUE if the port is open and an output FALSE otherwise
 */
gboolean
psy_parallel_port_is_output(PsyParallelPort *self)
{
    g_return_val_if_fail(PSY_IS_PARALLEL_PORT(self), FALSE);

    return psy_parallel_port_is_open(self)
           && (psy_parallel_port_get_direction(self) == PSY_IO_DIRECTION_OUT);
}

/**
 * psy_parallel_port_is_input:
 * @self: An instance of `PsyParallelPort`
 *
 * Returns whether or not the devices is configured as input.
 *
 * Returns: #TRUE if the port is open and an input FALSE otherwise
 */
gboolean
psy_parallel_port_is_input(PsyParallelPort *self)
{
    g_return_val_if_fail(PSY_IS_PARALLEL_PORT(self), FALSE);

    return psy_parallel_port_is_open(self)
           && (psy_parallel_port_get_direction(self) == PSY_IO_DIRECTION_IN);
}

/**
 * psy_parallel_write:
 * @self: an instance of `PsyParallelPort`
 * @mask: A bitmask describing which lines should be, if the first bit is
 *        set to a one in the mask, the first list is set high, etc.
 * @error: Errors are returned here.
 *
 * This function may be used to set some of the lines high or low
 * simultaneously. Not that in order to use this function you should configure
 * the device as an output first.
 */
void
psy_parallel_port_write(PsyParallelPort *self, guint8 mask, GError **error)
{
    PsyParallelPortClass *cls;
    g_return_if_fail(PSY_IS_PARALLEL_PORT(self));
    g_return_if_fail(error == NULL || *error != NULL);

    cls = PSY_PARALLEL_PORT_GET_CLASS(self);
    g_return_if_fail(cls->write);
    cls->write(self, mask, error);
}

/**
 * psy_parallel_write_pin:
 * @self: an instance of `PsyParallelPort`
 * @pin: the pin to write the data to [0-7]
 * @level: whether to turn the pin high or low.
 * @error: Errors are returned here.
 *
 * This function may be used to set some of the lines high or low
 * independently. Not that in order to use this function you should configure
 * the device as an output first.
 */
void
psy_parallel_port_write_pin(PsyParallelPort *self,
                            gint             pin,
                            PsyIoLevel       level,
                            GError         **error)
{
    PsyParallelPortClass *cls;
    g_return_if_fail(PSY_IS_PARALLEL_PORT(self));
    g_return_if_fail(error == NULL || *error != NULL);

    cls = PSY_PARALLEL_PORT_GET_CLASS(self);
    g_return_if_fail(cls->write_pin);
    cls->write_pin(self, pin, level, error);
}

/**
 * psy_parallel_read:
 * @self: an instance of `PsyParallelPort`
 * @error: Errors are returned here.
 *
 * This function may be used to set some of the lines high or low
 * simultaneously. Not that in order to use this function you should configure
 * the device as an input first.
 *
 * Returns: A value, you can check the bits[0 - 7] to see which lines are
 *          high or low, if a bit is set it is high.
 */
guint8
psy_parallel_port_read(PsyParallelPort *self, GError **error)
{
    PsyParallelPortClass *cls;
    g_return_val_if_fail(PSY_IS_PARALLEL_PORT(self), 0);
    g_return_val_if_fail(error == NULL || *error != NULL, 0);

    cls = PSY_PARALLEL_PORT_GET_CLASS(self);
    g_return_val_if_fail(cls->read, 0);

    return cls->read(self, error);
}

/**
 * psy_parallel_read_pin:
 * @self: an instance of `PsyParallelPort`
 * @pin: the pin to read the data from [0-7]
 * @error:(out): Errors are returned here.
 *
 * This function may be used to see whether some of the lines high or low
 * independently. Note that in order to use this function you should configure
 * the device as an input first.
 *
 * Returns: PSY_IO_LEVEL_LOW if the voltage has a low logical level,
 *          PSY_IO_LEVEL_HIGH otherwise.
 */
PsyIoLevel
psy_parallel_port_read_pin(PsyParallelPort *self, gint pin, GError **error)
{
    PsyParallelPortClass *cls;
    g_return_val_if_fail(PSY_IS_PARALLEL_PORT(self), PSY_IO_LEVEL_LOW);
    g_return_val_if_fail(error == NULL || *error != NULL, PSY_IO_LEVEL_LOW);

    cls = PSY_PARALLEL_PORT_GET_CLASS(self);
    g_return_val_if_fail(cls->read, PSY_IO_LEVEL_LOW);
    return cls->read_pin(self, pin, error);
}

/**
 * psy_parallel_port_get_pins:
 *
 * This operations doesn't touch the parallel port but merely obtains
 * the value after last read or write of the port.
 *
 * Returns: the status of the pins after the last operation.
 */
guint8
psy_parallel_port_get_pins(PsyParallelPort *self)
{
    PsyParallelPortPrivate *priv = psy_parallel_port_get_instance_private(self);

    g_return_val_if_fail(PSY_IS_PARALLEL_PORT(self), 0);

    return priv->pins;
}

// private functions

/**
 * psy_parallel_port_set_pins:(skip)
 *
 * This function updates the lines this function only does administration.
 * This function is for inside psy-lib only.
 */
void
psy_parallel_port_set_pins(PsyParallelPort *self, guint8 pins)
{
    PsyParallelPortPrivate *priv = psy_parallel_port_get_instance_private(self);

    g_return_if_fail(PSY_IS_PARALLEL_PORT(self));

    priv->pins = pins;
}
