
#include "psy-parallel-trigger.h"
#include "enum-types.h"
#include "psy-clock.h"
#include "psy-config.h"
#include "psy-duration.h"
#include "psy-parallel-port.h"
#include "psy-time-point.h"

static void
wait_until(PsyTimePoint *tp, GCancellable *cancellable)
{
    PsyDuration *one_ms  = psy_duration_new_ms(1);
    PsyDuration *null_ms = psy_duration_new_ms(0);
    PsyClock    *clock   = psy_clock_new();

    gint loop = 1;

    PsyDuration  *dur_test = NULL;
    PsyTimePoint *now      = NULL;

    while (loop) {
        if (g_cancellable_is_cancelled(cancellable)) {
            break;
        }
        now      = psy_clock_now(clock);
        dur_test = psy_time_point_subtract(tp, now);

        if (psy_duration_less(dur_test, one_ms)) {
            loop = 0;
        }
        else {
            g_usleep(1000);
        }

        g_clear_object(&dur_test);
        g_clear_object(&now);
    }

    g_assert(!dur_test);
    g_assert(!now);

    loop = 1;

    while (loop) {
        if (g_cancellable_is_cancelled(cancellable)) {
            break;
        }
        now      = psy_clock_now(clock);
        dur_test = psy_time_point_subtract(tp, now);

        if (psy_duration_less(dur_test, null_ms)) {
            loop = 0;
        }
        else {
            g_thread_yield();
        }

        g_clear_object(&now);
        g_clear_object(&dur_test);
    }

    g_assert(!dur_test);
    g_assert(!now);

    g_object_unref(clock);
    g_object_unref(one_ms);
    g_object_unref(null_ms);
}

// clang-format off
G_DEFINE_QUARK(psy-parallel-trigger-error-quark, psy_parallel_trigger_error)

// clang-format on

typedef struct TriggerData {
    PsyTimePoint *trigger_start;
    PsyDuration  *trigger_dur;
    guint         mask;
} TriggerData;

static void
trigger_data_free(gpointer data)
{
    TriggerData *d = data;
    g_object_unref(d->trigger_start);
    g_object_unref(d->trigger_dur);
    g_slice_free(TriggerData, data);
}

/**
 * PsyParallelTrigger:
 *
 * PsyParallelTrigger is a device that is able to send triggers using a
 * parallel port. You can specify a start time and optionally a duration.
 * The PsyParallel trigger will start an asynchronous operation to trigger
 * the device, so the mainloop can just continue. Unless the trigger is cancled
 * it is not possible to trigger again.
 * Signals will be delivered when the stimulus is triggered or optionally when
 * the trigger stops.
 */

typedef struct {
    PsyParallelPort *port;
    GTask           *trigger_task;
} PsyParallelTriggerPrivate;

G_DEFINE_TYPE_WITH_PRIVATE(PsyParallelTrigger,
                           psy_parallel_trigger,
                           G_TYPE_OBJECT)

typedef enum PsyParallelTriggerProperty {
    PROP_NULL,
    PORT_NUM,
    PORT_NAME,
    PORT_IS_OPEN,
    NUM_PROPS,
} PsyParallelTriggerProperty;

typedef enum {
    SIG_FINISHED,
    NUM_SIGNALS,
} PsyParallelTriggerSignal;

static GParamSpec *trigger_props[NUM_PROPS];
static guint       trigger_signals[NUM_SIGNALS];

// There are currently no settable properties
// static void
// psy_parallel_trigger_set_property(GObject      *object,
//                                   guint         property_id,
//                                   const GValue *value,
//                                   GParamSpec   *spec)
// {
//     PsyParallelTrigger *self = PSY_PARALLEL_TRI?GGER(object);
//     // PsyParallelTriggerPrivate *priv
//     //     = psy_parallel_trigger_get_instance_private(self);
//
//     switch ((PsyParallelTriggerProperty) property_id) {
//     default:
//         G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, spec);
//     }
// }

static void
psy_parallel_trigger_get_property(GObject    *object,
                                  guint       property_id,
                                  GValue     *value,
                                  GParamSpec *spec)
{
    PsyParallelTrigger        *self = PSY_PARALLEL_TRIGGER(object);
    PsyParallelTriggerPrivate *priv
        = psy_parallel_trigger_get_instance_private(self);

    switch ((PsyParallelTriggerProperty) property_id) {
    case PORT_NUM:
        g_value_set_int(value, psy_parallel_port_get_port_num(priv->port));
        break;
    case PORT_NAME:
        g_value_set_string(value, psy_parallel_port_get_port_name(priv->port));
        break;
    case PORT_IS_OPEN:
        g_value_set_boolean(value, psy_parallel_trigger_is_open(self));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, spec);
    }
}

static void
psy_parallel_trigger_init(PsyParallelTrigger *self)
{
    PsyParallelTriggerPrivate *priv
        = psy_parallel_trigger_get_instance_private(self);

    priv->port = psy_parallel_port_new();
}

static void
psy_parallel_trigger_dispose(GObject *self)
{
    PsyParallelTriggerPrivate *priv
        = psy_parallel_trigger_get_instance_private(PSY_PARALLEL_TRIGGER(self));

    g_clear_object(&priv->port);

    G_OBJECT_CLASS(psy_parallel_trigger_parent_class)->dispose(self);
}

static void
psy_parallel_trigger_class_init(PsyParallelTriggerClass *cls)
{
    GObjectClass *obj_cls = G_OBJECT_CLASS(cls);

    // Currently there are no settable properties
    // obj_cls->set_property = psy_parallel_trigger_set_property;
    obj_cls->get_property = psy_parallel_trigger_get_property;
    obj_cls->dispose      = psy_parallel_trigger_dispose;

    /**
     * PsyParallelTrigger:port-num:
     *
     * The number of device used for opening the parallel port. It is
     * specified using `psy_parallel_trigger_open`, hence it is read only
     * and should be -1 when closed.
     */
    trigger_props[PORT_NUM]
        = g_param_spec_int("port-num",
                           "PortNumber",
                           "The number of the port device to use",
                           -1,
                           16,
                           -1,
                           G_PARAM_READABLE);

    /**
     * PsyParallelTrigger:port-name:
     *
     * This is the name of the device at the os level, at linux it might be
     * "/dev/parport0" and at windows "LPT1". It should be set when the
     * device is open and should result in an empty string otherwise.
     */
    trigger_props[PORT_NAME] = g_param_spec_string(
        "port-name",
        "PortName",
        "The (file/device) name that corresponds with the parallel port",
        "",
        G_PARAM_READABLE);

    /**
     * PsyParallelTrigger:is-open:
     *
     * Returns true when the device is open.
     */
    trigger_props[PORT_IS_OPEN]
        = g_param_spec_boolean("is-open",
                               "IsOpen",
                               "Whether or not the port is open",
                               FALSE,
                               G_PARAM_READABLE);

    g_object_class_install_properties(obj_cls, NUM_PROPS, trigger_props);

    /**
     * PsyParallelTrigger::finished:
     * @self: an instance of `PsyParallelTrigger`
     * @mask: the mask that was send to the trigger.
     * @tstart: the timepoint on which the trigger started.
     * @tstop: the timepoint on which the trigger stopped.
     *
     * This signal is triggered once a write operation has finished.
     */
    trigger_signals[SIG_FINISHED] = g_signal_new("finished",
                                                 G_TYPE_FROM_CLASS(obj_cls),
                                                 G_SIGNAL_RUN_LAST,
                                                 0,
                                                 NULL,
                                                 NULL,
                                                 NULL,
                                                 G_TYPE_NONE,
                                                 3,
                                                 G_TYPE_UINT,
                                                 PSY_TYPE_TIME_POINT,
                                                 PSY_TYPE_TIME_POINT);
}

/**
 * psy_parallel_trigger_new:(constructor)
 *
 * Creates a new PsyParallelTrigger and returns it.
 *
 * Returns: an instance of `PsyParallelTrigger`
 */
PsyParallelTrigger *
psy_parallel_trigger_new(void)
{
    PsyParallelTrigger *trigger = NULL;

    trigger = g_object_new(PSY_TYPE_PARALLEL_TRIGGER, NULL);

    return trigger;
}

/**
 * psy_parallel_trigger_open:
 * @self: an instance of PsyParallelTrigger
 * @dev_num: The nth device to open.
 * @error:(out): Errors are returned here.
 *
 * Opens the device with the nth_device num, numbering starts at 0 until
 * the number of parallel ports known to the os.
 */
void
psy_parallel_trigger_open(PsyParallelTrigger *self,
                          gint                dev_num,
                          GError            **error)
{
    PsyParallelTriggerPrivate *priv
        = psy_parallel_trigger_get_instance_private(self);

    g_return_if_fail(PSY_IS_PARALLEL_TRIGGER(self));
    g_return_if_fail(error == NULL || *error == NULL);

    psy_parallel_port_open(priv->port, dev_num, error);
}

/**
 * psy_parallel_trigger_close:
 * @self: an instance of PsyParallelTrigger
 *
 * Closes the device when it's opened. This releases some of the resources
 * related to opening the parallel port. When the device is destroyed, it will
 * also be closed.
 */
void
psy_parallel_trigger_close(PsyParallelTrigger *self)
{
    PsyParallelTriggerPrivate *priv
        = psy_parallel_trigger_get_instance_private(self);

    g_return_if_fail(PSY_IS_PARALLEL_TRIGGER(self));

    psy_parallel_port_close(priv->port);
}

/**
 * psy_parallel_trigger_is_open:
 * @self: an instance of PsyParallelTrigger
 *
 * This function may be used to check whether the device is open.
 *
 * Returns: #TRUE when the devices is open, #FALSE otherwise
 */
gboolean
psy_parallel_trigger_is_open(PsyParallelTrigger *self)
{
    g_return_val_if_fail(PSY_IS_PARALLEL_TRIGGER(self), FALSE);

    PsyParallelTriggerPrivate *priv
        = psy_parallel_trigger_get_instance_private(self);

    return psy_parallel_port_is_open(priv->port);
}

/**
 * psy_parallel_trigger_get_port_name:
 * @self: an instance of `PsyParallelTrigger`
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
psy_parallel_trigger_get_port_name(PsyParallelTrigger *self)
{
    PsyParallelTriggerPrivate *priv
        = psy_parallel_trigger_get_instance_private(self);

    g_return_val_if_fail(PSY_IS_PARALLEL_TRIGGER(self), NULL);

    return psy_parallel_port_get_port_name(priv->port);
}

void
write_thread(GTask        *task,
             GObject      *source_obj,
             gpointer      task_data,
             GCancellable *cancellable);

/**
 * psy_parallel_trigger_write_async:
 *
 * Starts the thread that arranges the triggering.
 *
 * Stability:Private
 */
static void
psy_parallel_trigger_write_async(PsyParallelTrigger *self,
                                 guint8              mask,
                                 PsyTimePoint       *tstart,
                                 PsyDuration        *dur,
                                 GCancellable       *cancellable,
                                 GAsyncReadyCallback callback,
                                 gpointer            data)
{
    // PsyParallelTriggerPrivate *priv
    //    = psy_parallel_trigger_get_instance_private(self);

    TriggerData *tdata   = g_slice_new(TriggerData);
    tdata->trigger_start = g_object_ref(tstart);
    tdata->trigger_dur   = g_object_ref(dur);
    tdata->mask          = mask;

    GTask *trigger_task = g_task_new(self, cancellable, callback, data);
    g_task_set_task_data(trigger_task, tdata, trigger_data_free);
    g_task_run_in_thread(trigger_task, (GTaskThreadFunc) write_thread);

    g_object_unref(trigger_task);
}

void
write_thread(GTask        *task,
             GObject      *source_obj,
             gpointer      task_data,
             GCancellable *cancellable)
{
    g_assert(G_IS_TASK(task));
    g_assert(PSY_IS_PARALLEL_TRIGGER(source_obj));

    TriggerData               *data    = task_data;
    PsyParallelTrigger        *trigger = PSY_PARALLEL_TRIGGER(source_obj);
    PsyParallelTriggerPrivate *priv
        = psy_parallel_trigger_get_instance_private(trigger);

    GError *error = NULL;

    PsyTimePoint *tstart = data->trigger_start;
    PsyDuration  *dur    = data->trigger_dur;
    PsyTimePoint *end    = psy_time_point_add(tstart, dur);

    wait_until(tstart, cancellable);

    if (g_cancellable_is_cancelled(cancellable)) {
        g_task_return_error_if_cancelled(task);
        goto end;
    }

    psy_parallel_port_write(PSY_PARALLEL_PORT(priv->port), data->mask, &error);
    if (error) {
        g_critical("ParallelTrigger write failed: %s\n", error->message);
    }

    wait_until(end, cancellable);

    if (g_cancellable_is_cancelled(cancellable)) {
        g_task_return_error_if_cancelled(task);
        goto end;
    }

    psy_parallel_port_write(PSY_PARALLEL_PORT(priv->port), 0, &error);
    if (error) {
        g_printerr("Oops write failed: %s\n", error->message);
    }

    g_task_return_boolean(task, TRUE);

end:

    g_object_unref(end);
}

/**
 * psy_parallel_trigger_finish:
 * @self: an instance of `PsyParallelPort`
 * @result: the result of the operation
 * @mask:(out): The trigger written to the port
 * @tstart:(out) (transfer full): The trigger written to the port
 * @tfinish:(out) (transfer full): The trigger written to the port
 * @error: if an error occured it returned here.
 *
 * Collect the result of the write operation.
 */
static gboolean
psy_parallel_trigger_write_finish(PsyParallelTrigger *self,
                                  GAsyncResult       *result,
                                  guint              *mask,
                                  PsyTimePoint      **tstart,
                                  PsyTimePoint      **tfinish,
                                  GError            **error)
{
    g_return_val_if_fail(g_task_is_valid(result, self), FALSE);
    g_return_val_if_fail(tstart == NULL || *tstart == NULL, FALSE);
    g_return_val_if_fail(tfinish == NULL || *tfinish == NULL, FALSE);

    GTask       *task = G_TASK(result);
    TriggerData *data = g_task_get_task_data(task);

    if (mask)
        *mask = data->mask;

    if (tstart) {
        *tstart = g_object_ref(data->trigger_start);
    }
    if (tfinish) {
        *tfinish = psy_time_point_add(data->trigger_start, data->trigger_dur);
    }

    return g_task_propagate_boolean(task, error);
}

void
trigger_finished_cb(PsyParallelTrigger *self,
                    GAsyncResult       *result,
                    GError            **error)
{
    PsyParallelTriggerPrivate *priv
        = psy_parallel_trigger_get_instance_private(self);

    g_return_if_fail(PSY_IS_PARALLEL_TRIGGER(self));
    g_return_if_fail(G_IS_TASK(result));

    PsyTimePoint *tstart = NULL, *tfinish = NULL;
    guint         mask;

    gboolean succes = psy_parallel_trigger_write_finish(
        self, result, &mask, &tstart, &tfinish, error);

    g_clear_object(&priv->trigger_task);

    if (succes)
        g_signal_emit(self, trigger_signals[SIG_FINISHED], 0, mask, tstart, tfinish);

    g_object_unref(tfinish);
    g_object_unref(tstart);
}

/**
 * psy_parallel_trigger_write:
 * @self: an instance of `PsyParallelTrigger`
 * @mask: A bitmask describing which lines should be, if the first bit is
 *        set to a one in the mask, the first list is set high, etc.
 * @tstart: The timepoint at which the trigger should start.
 * @dur: The duration of the trigger.
 * @error: Errors are returned here.
 *
 * This function may be used to set some of the lines high or low
 * simultaneously. Not that in order to use this function you should open the
 * device first. It's an error to call this function when a previous
 */
void
psy_parallel_trigger_write(PsyParallelTrigger *self,
                           guint8              mask,
                           PsyTimePoint       *tstart,
                           PsyDuration        *dur,
                           GError            **error)
{
    PsyParallelTriggerPrivate *priv
        = psy_parallel_trigger_get_instance_private(self);

    g_return_if_fail(PSY_IS_PARALLEL_TRIGGER(self));
    g_return_if_fail(error == NULL || *error == NULL);

    if (!psy_parallel_port_is_open(priv->port)) {
        g_set_error(error,
                    PSY_TYPE_PARALLEL_PORT_ERROR,
                    PSY_PARALLEL_PORT_ERROR_DEV_CLOSED,
                    "Parallel Port is closed.");
        return;
    }

    if (priv->trigger_task) {
        g_set_error(error,
                    PSY_PARALLEL_TRIGGER_ERROR,
                    PSY_PARALLEL_TRIGGER_ERROR_BUSY,
                    "There is already a write action going on");
        return;
    }

    GCancellable *cancellable = g_cancellable_new();

    psy_parallel_trigger_write_async(
        self, mask, tstart, dur, cancellable, trigger_finished_cb, NULL);

    g_assert(cancellable->parent_instance.ref_count == 2);
    g_object_unref(cancellable);
    g_assert(cancellable->parent_instance.ref_count == 1);
}

// /**
//  * psy_parallel_triggerwrite_pin:
//  * @self: an instance of `PsyParallelTrigger`
//  * @pin: the pin to write the data to [0-7]
//  * @level: whether to turn the pin high or low.
//  * @error: Errors are returned here.
//  *
//  * This function may be used to set some of the lines high or low
//  * independently. Not that in order to use this function you should configure
//  * the device as an output first.
//  */
// void
// psy_parallel_trigger_write_pin(PsyParallelTrigger *self,
//                                gint                pin,
//                                PsyIoLevel          level,
//                                GError            **error)
// {
//     PsyParallelTriggerClass *cls;
//     g_return_if_fail(PSY_IS_PARALLEL_TRIGGER(self));
//     g_return_if_fail(error == NULL || *error != NULL);
//
//     cls = PSY_PARALLEL_TRIGGER_GET_CLASS(self);
//     g_return_if_fail(cls->write_pin);
//     cls->write_pin(self, pin, level, error);
// }

/**
 * psy_parallel_trigger_cancel:
 * @self: An instance of psy_parallel_trigger
 *
 * If there is an ongoing call to send a trigger or a trigger is busy, this
 * call will try to cancel that trigger. If nothing is going on, this does
 * nothing.
 * Note that this may be inconvenient, when the parallel port has just
 * triggered, but
 *
 */
void
psy_parallel_trigger_cancel(PsyParallelTrigger *self)
{
    PsyParallelTriggerPrivate *priv
        = psy_parallel_trigger_get_instance_private(self);
    g_return_if_fail(PSY_IS_PARALLEL_TRIGGER(self));

    if (priv->trigger_task) {
        GCancellable *cancellable = g_task_get_cancellable(priv->trigger_task);
        if (cancellable) {
            g_cancellable_cancel(cancellable);
        }
        else {
            g_critical("Unable to cancel trigger task");
        }
    }
}
