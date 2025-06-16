

#include "psy-teensy-trigger.h"
#include "psy-serial-port.h"
#include "psy-timer.h"

struct _PsyTeensyTrigger {
    GObject        parent;
    PsySerialPort *port;
    PsyTimer      *timer;
    PsyDuration   *trigger_dur;
};

G_DEFINE_TYPE(PsyTeensyTrigger, psy_teensy_trigger, G_TYPE_OBJECT)

static void
psy_teensy_trigger_init(PsyTeensyTrigger *self)
{
    self->port        = psy_serial_port_new();
    self->timer       = psy_timer_new();
    self->trigger_dur = psy_duration_new_ms(5);
}

static void
teensy_trigger_dispose(GObject *obj)
{
    PsyTeensyTrigger *self = PSY_TEENSY_TRIGGER(obj);
    g_clear_object(&self->timer);
    g_clear_object(&self->port);
}

static void
teensy_trigger_finalize(GObject *obj)
{
    PsyTeensyTrigger *self = PSY_TEENSY_TRIGGER(obj);

    g_clear_pointer(&self->trigger_dur, psy_duration_free);
}

void
psy_teensy_trigger_class_init(PsyTeensyTriggerClass *cls)
{
    GObjectClass *obj_cls = G_OBJECT_CLASS(cls);

    obj_cls->dispose  = teensy_trigger_dispose;
    obj_cls->finalize = teensy_trigger_finalize;
}

/**
 * psy_teensy_trigger_new:(constructor)
 * @name: The name used for opening the serial port
 *
 * Create a new trigger device in order to trigger a teensy device that runs the
 * firmware in order to trigger on the d-port.
 */
PsyTeensyTrigger *
psy_teensy_trigger_new(const gchar *name)
{
    PsyTeensyTrigger *ret;
    g_return_val_if_fail(name != NULL, NULL);

    ret = g_object_new(PSY_TYPE_TEENSY_TRIGGER, "name", name, NULL);
    return ret;
}
