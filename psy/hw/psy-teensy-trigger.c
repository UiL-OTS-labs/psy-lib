#include "psy-teensy-trigger.h"
#include "psy-serial-port.h"
#include "psy-timer.h"

#define TEENSY_MSG_HEADER_SIZE (sizeof(guint8[2]))
#define TEENSY_MSG_CONNECT "client connect"

enum {
    TEENSY_MSG_TYPE_IDX,
    TEENSY_MSG_SIZE_IDX,
    TEENSY_MSG_PAYLOAD_IDX,
};

typedef enum TeensyMsgType {
    NOT_INIT = -1,
    CONNECT  = 1,
    ACK,
    ERROR,
    PIN_OUT,
    CLOSE,
} TeensyMsgType;

typedef struct TeensyMsg {
    guint8 buffer[256];
} TeensyMsg;

static TeensyMsg
teensy_msg_create_connect(void)
{
    TeensyMsg ret;

    ret.buffer[0] = TEENSY_MSG_HEADER_SIZE + strlen(TEENSY_MSG_CONNECT);
    ret.buffer[1] = CONNECT;
    memcpy(&ret.buffer[2], TEENSY_MSG_CONNECT, strlen(TEENSY_MSG_CONNECT) + 1);

    return ret;
}

static TeensyMsg
teensy_msg_create_close(void)
{
    TeensyMsg ret;
    ret.buffer[0] = TEENSY_MSG_HEADER_SIZE;
    ret.buffer[1] = CLOSE;

    return ret;
}

static TeensyMsg
teensy_msg_create_pin(guint8 pin_mask)
{
    TeensyMsg ret;
    ret.buffer[0] = TEENSY_MSG_HEADER_SIZE + 1;
    ret.buffer[1] = PIN_OUT;
    ret.buffer[2] = pin_mask;

    return ret;
}

static TeensyMsg
serial_port_read_msg(PsySerialPort *port)
{
    TeensyMsg ret;
    g_assert(PSY_IS_SERIAL_PORT(port));

    gssize num_read_tot = 0;
    gssize num_to_read  = TEENSY_MSG_HEADER_SIZE;

    while (num_read_tot < num_to_read) {
        gssize num_read = psy_serial_port_read_raw(
            port, &ret.buffer[num_read_tot], num_to_read - num_read_tot);
        if (num_read < 0) {
            int err = errno;
            if (err == EINTR) {
                continue;
            }
            else {
                g_critical("Unable to receive msg: %s", g_strerror(errno));
                return ret;
            }
        }
        num_read_tot += num_read;
    }

    num_to_read = ret.buffer[0];
    while (num_read_tot < num_to_read) {
        gssize num_read = psy_serial_port_read_raw(
            port, &ret.buffer[num_read_tot], num_to_read - num_read_tot);
        if (num_read < 0) {
            int err = errno;
            if (err == EINTR) {
                continue;
            }
            else {
                g_critical("Unable to receive msg: %s", g_strerror(errno));
                return ret;
            }
        }
        num_read_tot += num_read;
    }

    return ret;
}

static void
serial_port_write_msg(PsySerialPort *port, const TeensyMsg *msg, GError **error)
{
    g_assert(PSY_IS_SERIAL_PORT(port));
    g_assert(msg != NULL);

    gsize num_to_write = msg->buffer[0];
    gsize num_written  = 0;

    while (num_written < num_to_write) {
        gssize n = psy_serial_port_write(
            port, &msg->buffer[num_written], num_to_write - num_written, error);
        if (n < 0) {
            return;
        }
        num_written += n;
    }
}

/**
 * TeensyTrigger:
 *
 * The teensy trigger is an inteface to a Teensy device with specific firmware
 * loaded. The firmware allows to write a trigger on specific pins of
 * portD, it doesn't use digitalWrite, because it should be able to write
 * to all pins at exactly the same time, whereas digtalWrite can toggle one
 * line at a time.
 * The firmware can be programmed using Arduino IDE with the Teensyduino
 * extensions. In order to use the board, you should know on which serial
 * device the board is connected. On linux this is e.g /dev/ttyACM0 and
 * on windows this might be COM1.
 * The firmware is currently tested on a Teensy 3.2 which is unfortunately
 * not orderable anymore.
 */

static void
teensy_trigger_timer_fired(PsyTeensyTrigger *self,
                           PsyTimePoint     *tp_trigger,
                           PsyTimer         *timer);

// async trigger cb
static void
teensy_trigger_cb(PsyTimePoint *tp, gpointer data);

struct _PsyTeensyTrigger {
    GObject        parent;
    PsySerialPort *port;
    PsyTimer      *timer;
    PsyDuration   *trig_dur;
    guint8         pin_mask;
};

typedef enum {
    PROP_NULL,
    PROP_NAME,
    PROP_IS_OPEN,
    PROP_TRIG_DUR,
    NUM_PROPS, // KEEP this last
} PsyTeensyTriggerProperty;

static GParamSpec *teensy_trigger_properties[NUM_PROPS];

G_DEFINE_TYPE(PsyTeensyTrigger, psy_teensy_trigger, G_TYPE_OBJECT)

static void
psy_teensy_trigger_init(PsyTeensyTrigger *self)
{
    self->port     = psy_serial_port_new();
    self->timer    = psy_timer_new();
    self->trig_dur = psy_duration_new_ms(5);

    g_signal_connect_swapped(
        self->timer, "fired", G_CALLBACK(teensy_trigger_timer_fired), self);
}

static void
teensy_trigger_set_property(GObject      *object,
                            guint         property_id,
                            const GValue *value,
                            GParamSpec   *spec)
{
    PsyTeensyTrigger *self = PSY_TEENSY_TRIGGER(object);

    switch ((PsyTeensyTriggerProperty) property_id) {
    case PROP_NAME:
        psy_serial_port_set_name(self->port, g_value_get_string(value));
        break;
    case PROP_TRIG_DUR:
        psy_teensy_trigger_set_trig_dur(self, g_value_get_boxed(value));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, spec);
    }
}

static void
teensy_trigger_get_property(GObject    *object,
                            guint       property_id,
                            GValue     *value,
                            GParamSpec *spec)
{
    PsyTeensyTrigger *self = PSY_TEENSY_TRIGGER(object);

    switch ((PsyTeensyTriggerProperty) property_id) {
    case PROP_NAME:
        g_value_set_string(value, psy_serial_port_get_name(self->port));
        break;
    case PROP_IS_OPEN:
        g_value_set_boolean(value, psy_serial_port_get_is_open(self->port));
        break;
    case PROP_TRIG_DUR:
        g_value_set_boxed(value, psy_teensy_trigger_get_trig_dur(self));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, spec);
    }
}

static gboolean
teensy_trigger_open(PsyTeensyTrigger *self, GError **error)
{
    gboolean ret = psy_serial_port_open(self->port, error);
    if (ret) {
        TeensyMsg msg = teensy_msg_create_connect();

        serial_port_write_msg(self->port, &msg, error);

        msg = serial_port_read_msg(self->port);
        if (msg.buffer[0] != 2 || msg.buffer[1] != ACK) {
            // It's not a teensy trigger
            g_critical(
                "Not communicating to a device that is a teensy trigger, "
                "did you set the proper device_name");
            g_set_error(
                error,
                PSY_SERIAL_PORT_ERROR,
                PSY_SERIAL_PORT_ERROR_FAILED,
                "Connected device doesn't respond like a TeensyTrigger");
            psy_serial_port_close(self->port);
        }
        ret = FALSE;
    }
    return ret;
}

static void
teensy_trigger_close(PsyTeensyTrigger *self)
{
    psy_timer_cancel(self->timer); // cancel ongoing operations

    TeensyMsg msg = teensy_msg_create_close();

    serial_port_write_msg(self->port, &msg, NULL);
    msg = serial_port_read_msg(self->port);
    if (msg.buffer[TEENSY_MSG_SIZE_IDX] != TEENSY_MSG_HEADER_SIZE
        || msg.buffer[TEENSY_MSG_TYPE_IDX] != ACK) {
        g_critical("Something failed while closing the device");
    }

    psy_serial_port_close(self->port);
}

static void
teensy_trigger_dispose(GObject *obj)
{
    PsyTeensyTrigger *self = PSY_TEENSY_TRIGGER(obj);
    g_clear_object(&self->timer);
    psy_serial_port_close(self->port);
    g_clear_object(&self->port);
}

static void
teensy_trigger_finalize(GObject *obj)
{
    PsyTeensyTrigger *self = PSY_TEENSY_TRIGGER(obj);

    g_clear_pointer(&self->trig_dur, psy_duration_free);
}

void
psy_teensy_trigger_class_init(PsyTeensyTriggerClass *cls)
{
    GObjectClass *obj_cls = G_OBJECT_CLASS(cls);

    obj_cls->set_property = teensy_trigger_set_property;
    obj_cls->get_property = teensy_trigger_get_property;
    obj_cls->dispose      = teensy_trigger_dispose;
    obj_cls->finalize     = teensy_trigger_finalize;

    /**
     * TeensyTrigger:name:
     *
     * The name of the file object that represents the serial port
     * to use in order to communicate with the Teensy device. e.g.
     * "/dev/ttyACM?" on Linux or "COM?" on Windows, where the "?"
     * is an int >= 0
     */
    teensy_trigger_properties[PROP_NAME] = g_param_spec_string(
        "name",
        "Name",
        "The name of the file/com port of the serial device",
        "",
        G_PARAM_READWRITE);

    /**
     * TeensyTrigger:is-open:
     *
     * If there is no connection to the Teensy device, this should
     * read as false, true otherwise.
     */
    teensy_trigger_properties[PROP_IS_OPEN]
        = g_param_spec_string("is-open",
                              "IsOpen",
                              "Whether or not the serial connection to the "
                              "teensy trigger device has been opened",
                              FALSE,
                              G_PARAM_READABLE);

    /**
     * TeensyTrigger:trig-dur:
     *
     * The trigger duration of the teensy trigger, if you write a value, the
     * value is cleared after trig-dur. You can change this value, when
     * a trigger is scheduled, but it might only have effect when that trigger
     * finishes, or might have an effect right away.
     */
    teensy_trigger_properties[PROP_TRIG_DUR]
        = g_param_spec_boxed("trig-dur",
                             "trigger duration",
                             "The duration of the pulse when triggering",
                             PSY_TYPE_DURATION,
                             G_PARAM_READWRITE);

    g_object_class_install_properties(
        obj_cls, NUM_PROPS, teensy_trigger_properties);
}

/**
 * psy_teensy_trigger_new:(constructor)
 * @name: The name used for opening the serial port
 *
 * Create a new trigger device in order to trigger a teensy device that runs
 * the firmware in order to trigger on the d-port.
 */
PsyTeensyTrigger *
psy_teensy_trigger_new(const gchar *name)
{
    PsyTeensyTrigger *ret;
    g_return_val_if_fail(name != NULL, NULL);

    ret = g_object_new(PSY_TYPE_TEENSY_TRIGGER, "name", name, NULL);
    return ret;
}

/**
 * psy_teensy_trigger_open:
 * @self an instance of [class@TeensyTrigger]
 * @error:(out):
 *
 * Opens the internal serial port for communicating with the teensy
 * device. This should be called before trying to trigger. This or the
 * _async version should be called and completed before trying to write
 * triggers.
 *
 * Returns: TRUE when the device is successfully opened, FALSE otherwise.
 */
gboolean
psy_teensy_trigger_open(PsyTeensyTrigger *self, GError **error)
{
    g_return_val_if_fail(PSY_IS_TEENSY_TRIGGER(self), FALSE);
    g_return_val_if_fail(error || *error != NULL, FALSE);

    if (psy_serial_port_get_is_open(self->port))
        return TRUE;

    return teensy_trigger_open(self, error);
}

/**
 * psy_teensy_trigger_close:
 *
 * closes the connection with the teensy trigger in a appropriate fashion
 */
void
psy_teensy_trigger_close(PsyTeensyTrigger *self)
{
    g_return_if_fail(PSY_IS_TEENSY_TRIGGER(self));

    if (psy_serial_port_get_is_open(self->port)) {
        teensy_trigger_close(self);
    }
}

/**
 * psy_teensy_trigger_write:
 * @self: A pointer to a TeensyTrigger instance
 * @mask: The bits in the mask will be put high on the lines of port D of
 * the teensy device.
 * @tstart: the time-point on which we expect the trigger to turn on
 * @error: An error is returned here, e.g. when the trigger device isn't
 * open yet.
 *
 * Set a timer to trigger at a specific moment in time. When the time point
 * has passed the trigger will be send as quickly as possible. Before
 * writing triggers open the device
 */
void
psy_teensy_trigger_write(PsyTeensyTrigger *self,
                         guint8            mask,
                         PsyTimePoint     *tstart,
                         GError          **error)
{
    g_return_if_fail(PSY_IS_TEENSY_TRIGGER(self));
    g_return_if_fail(tstart != NULL);
    g_return_if_fail(error == NULL || *error == NULL);

    if (!psy_serial_port_get_is_open(self->port)) {
        g_set_error(error,
                    PSY_SERIAL_PORT_ERROR,
                    PSY_SERIAL_PORT_ERROR_CLOSED,
                    "Unable to write teensy trigger, when the trigger "
                    "device hasn't been opened yet.");
        return;
    }
    self->pin_mask = mask;

    psy_timer_set_async_fire_cb(self->timer, teensy_trigger_cb, self);
    psy_timer_set_fire_time(self->timer, tstart);
}

static void
teensy_trigger_timer_fired(PsyTeensyTrigger *self,
                           PsyTimePoint     *tp,
                           PsyTimer         *timer)
{

    if (self->pin_mask != 0) {
        PsyTimePoint *tp_off = psy_time_point_add(tp, self->trig_dur);
        self->pin_mask       = 0;
        psy_timer_set_async_fire_cb(self->timer, teensy_trigger_cb, self);
        psy_timer_set_fire_time(timer, tp_off);
        psy_time_point_free(tp_off);
    }
}

static void
teensy_trigger_cb(PsyTimePoint *tp, gpointer data)
{
    (void) tp;
    GError           *error = NULL;
    PsyTeensyTrigger *self  = PSY_TEENSY_TRIGGER(data);
    TeensyMsg         msg   = teensy_msg_create_pin(self->pin_mask);

    g_assert(PSY_IS_TEENSY_TRIGGER(self));

    serial_port_write_msg(self->port, &msg, &error);

    if (G_UNLIKELY(error)) {
        g_critical("Oops unable to send msg: %s", error->message);
        g_clear_error(&error);
    }
}

/**
 * psy_teensy_trigger_set_trig_dur:
 * @dur:(transfer none): The new duration of the start and stop of the trigger
 *                       This should be more than 0s;
 *
 * Set the new trigger duration for this trigger. The value written will be
 * clear after a duration of dur.
 */
void
psy_teensy_trigger_set_trig_dur(PsyTeensyTrigger *self, PsyDuration *dur)
{
    g_return_if_fail(PSY_IS_TEENSY_TRIGGER(self));
    g_return_if_fail(dur || psy_duration_get_us(dur) > 0);

    g_clear_pointer(&self->trig_dur, psy_duration_free);
    self->trig_dur = psy_duration_copy(dur);
}

/**
 * psy_teensy_trigger_get_trig_dur:
 *
 * Get the current trigger duration
 *
 * Returns: The current trigger duration
 */

PsyDuration *
psy_teensy_trigger_get_trig_dur(PsyTeensyTrigger *self)
{
    g_return_val_if_fail(PSY_IS_TEENSY_TRIGGER(self), NULL);

    return self->trig_dur;
}
