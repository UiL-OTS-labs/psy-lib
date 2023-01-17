
#include "parallel-port.h"
#include "enum-types.h"

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
} PsyParallelPortPrivate;

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE(PsyParallelPort,
                                    psy_parallel_port,
                                    G_TYPE_OBJECT)

typedef enum PsyParallelPortProperty {
    PROP_NULL,
    PORT_NUM,
    PORT_NAME,
    PORT_DIRECTION,
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
        g_value_set_enum(value, priv->direction);
        break;
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
psy_parallel_port_class_init(PsyParallelPortClass *cls)
{
    GObjectClass *obj_cls = G_OBJECT_CLASS(cls);

    obj_cls->set_property = psy_parallel_port_set_property;
    obj_cls->get_property = psy_parallel_port_get_property;
    obj_cls->finalize     = parallel_port_finalize;

    port_properties[PORT_NUM] =
        g_param_spec_int("port-num",
                         "PortNumber",
                         "The number of the port device to use",
                         -1,
                         16,
                         -1,
                         G_PARAM_READABLE);

    port_properties[PORT_NAME] = g_param_spec_string(
        "port-name",
        "PortName",
        "The (file/device) name that corresponds with the parallel port",
        "",
        G_PARAM_READABLE);

    port_properties[PORT_DIRECTION] =
        g_param_spec_enum("direction",
                          "Direction",
                          "The in- or output direction of this device",
                          PSY_TYPE_IO_DIRECTION,
                          PSY_IO_DIRECTION_OUT,
                          G_PARAM_READWRITE);

    g_object_class_install_properties(obj_cls, NUM_PROPS, port_properties);
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
