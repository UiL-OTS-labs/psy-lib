
#include "ddd-step.h"

typedef struct _DddStepPrivate {
    DddStep* parent;
    GMainContext *context;
} DddStepPrivate;

typedef struct {
    DddStep        *step;
    gint64          timestamp;
} InternalTimestamp;

static void
internal_timestamp_free(gpointer timestamp)
{
    InternalTimestamp *tstamp = timestamp;
    g_clear_object(&tstamp->step);
    g_free(timestamp);
}

G_DEFINE_TYPE_WITH_PRIVATE(DddStep, ddd_step, G_TYPE_OBJECT)

typedef enum {
    ENTER,
    LEAVE,
    NUM_SIGNALS
} DddStepSignal;

typedef enum {
    PROP_NULL,
    PROP_PARENT,
//    PROP_ACTIVE,
    NUM_PROPERTIES
} DddStepProperty;

static GParamSpec *obj_properties[NUM_PROPERTIES] = {NULL};
static guint step_signals[NUM_SIGNALS];

static void
ddd_step_set_property (GObject      *object,
                       guint         property_id,
                       const GValue *value,
                       GParamSpec   *pspec)
{
    DddStep *self = DDD_STEP(object);
    //DddStepPrivate *priv = ddd_step_get_instance_private(self);

    switch ((DddStepProperty) property_id)
    {
        case PROP_PARENT:
            ddd_step_set_parent(self, g_value_get_object(value));
            break;
        default:
            /* We don't have any other property... */
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
            break;
    }
}

static void
ddd_step_get_property (GObject    *object,
                       guint       property_id,
                       GValue     *value,
                       GParamSpec *pspec)
{
    DddStep *self = DDD_STEP(object);
    DddStepPrivate* priv = ddd_step_get_instance_private(self);

    switch ((DddStepProperty) property_id)
    {
        case PROP_PARENT:
            g_value_set_object (value, priv->parent);
            break;
        default:
            /* We don't have any other property... */
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
            break;
    }
}

static void
ddd_step_dispose (GObject *gobject)
{
    DddStepPrivate *priv = ddd_step_get_instance_private (DDD_STEP(gobject));

    g_clear_object (&priv->parent);
    if(priv->context) {
        g_main_context_unref(priv->context);
        priv->context = NULL;
    }

    G_OBJECT_CLASS (ddd_step_parent_class)->dispose (gobject);
}

static void
ddd_step_finalize (GObject *gobject)
{
    DddStepPrivate *priv = ddd_step_get_instance_private (DDD_STEP(gobject));

    (void) priv;

    G_OBJECT_CLASS (ddd_step_parent_class)->finalize (gobject);
}

static void
ddd_step_init(DddStep* self)
{
    DddStepPrivate *priv = ddd_step_get_instance_private(self);
    priv->context = g_main_context_ref_thread_default();
}

static void
step_activate(DddStep* self, gint64 timestamp)
{
    (void) self;
    (void) timestamp;
}

static void
step_deactivate(DddStep* self, gint64 timestamp)
{
    DddStepPrivate *priv = ddd_step_get_instance_private(self);
    if (priv->parent) {
        ddd_step_activate(priv->parent, timestamp);
    }
    else {
        g_object_unref(self);
    }
    (void) self;
    (void) timestamp;
}

static void
step_on_enter(DddStep* self, gint64 timestamp)
{
    DddStepClass *klass = DDD_STEP_GET_CLASS(self);
    klass->activate(self, timestamp);
}

static void
step_on_leave(DddStep* self, gint64 timestamp)
{
    DddStepClass *klass = DDD_STEP_GET_CLASS(self);
    klass->deactivate(self, timestamp);
}

static void
ddd_step_class_init(DddStepClass* klass)
{
    GObjectClass *obj_class = G_OBJECT_CLASS(klass);

    obj_class->set_property = ddd_step_set_property;
    obj_class->get_property = ddd_step_get_property;
    obj_class->dispose      = ddd_step_dispose;
    obj_class->finalize     = ddd_step_finalize;

    klass->activate     = step_activate;
    klass->on_enter     = step_on_enter;
    klass->on_leave     = step_on_leave;
    klass->deactivate   = step_deactivate;

    /* Add properties and signals to the interface here */

    /**
     * DddStep:parent:
     *
     * The parent is the object that gets activated when we leave the current
     * step, in such a way that structure of an experiment continues.
     * We do not own a reference to the parent, we assume the parent own this
     * step. The parent may be NULL for the top most Step object.
     */
    obj_properties[PROP_PARENT] =
            g_param_spec_object(
                    "parent",
                    "Parent",
                    "The parent object that gets signalled when this step is over.",
                    DDD_TYPE_STEP,
                    G_PARAM_READWRITE
                    );

//    obj_properties[PROP_ACTIVE] =
//            g_param_spec_boolean(
//                    "active",
//                    "Active",
//                    "Whether we are in the current step",
//                    FALSE,
//                    G_PARAM_READABLE
//                    );

    g_object_class_install_properties(
            obj_class, NUM_PROPERTIES, obj_properties
            );

    g_assert(G_TYPE_FROM_CLASS(obj_class) == DDD_TYPE_STEP);

    /**
     * DddStep::enter:
     * @self: the step that we are stepping into.
     * @timestamp: The timestamp that counts as a reference for the
     *             current step.
     *
     * This signal is emitted when a step is entered.
     */
    step_signals[ENTER] = g_signal_new(
            "enter",
            G_TYPE_FROM_CLASS(obj_class),
            G_SIGNAL_RUN_FIRST | G_SIGNAL_NO_RECURSE,
            G_STRUCT_OFFSET(DddStepClass, on_enter),
            NULL,
            NULL,
            NULL,
            G_TYPE_NONE,
            1,
            G_TYPE_INT64
            );

    /**
     * DddStep::leave:
     * @self: the step that we are stepping out of.
     * @timestamp: The timestamp of the event that finished the current
     *             step. This timestamp propagates to the enter signal
     *             of a next step.
     *
     * This signal is emitted when leaving a step.
     */
    step_signals[LEAVE] = g_signal_new(
            "leave",
            G_TYPE_FROM_CLASS(obj_class),
            G_SIGNAL_RUN_LAST,
            G_STRUCT_OFFSET(DddStepClass, on_leave),
            NULL,
            NULL,
            NULL,
            G_TYPE_NONE,
            1,
            G_TYPE_INT64
            );
}

static gboolean
step_in_cb(gpointer data)
{
    InternalTimestamp* timestamp = data;
    DddStepPrivate* priv = ddd_step_get_instance_private(timestamp->step);
    g_assert(g_main_context_is_owner(priv->context));
    g_assert(DDD_IS_STEP(timestamp->step));
    g_signal_emit(
            timestamp->step, step_signals[ENTER], 0, timestamp->timestamp
            );

    return G_SOURCE_REMOVE;
}

static gboolean
step_out_cb(gpointer data)
{
    InternalTimestamp* timestamp = data;
    DddStepPrivate* priv = ddd_step_get_instance_private(timestamp->step);
    g_assert(g_main_context_is_owner(priv->context));
    g_signal_emit(
            timestamp->step, step_signals[LEAVE], 0, timestamp->timestamp
            );
    return G_SOURCE_REMOVE;
}

/**
 * ddd_step_enter:
 * @self::A DddStep object
 * @timestamp::
 *
 * To enter a step, means to activate the current object. The timestamp
 * given is a timestamp on which previously a trial ended or the start
 * of an experiment.
 */
void
ddd_step_enter (DddStep* self, gint64 tstamp)
{
    g_return_if_fail(DDD_IS_STEP(self));
    DddStepPrivate *priv = ddd_step_get_instance_private(self);
    GSource *source;

    InternalTimestamp *data = g_new(InternalTimestamp, 1);
    data->timestamp         = tstamp;
    data->step              = g_object_ref(self);

    source = g_idle_source_new();
    g_source_set_priority (source, G_PRIORITY_DEFAULT);
    g_source_set_callback (source, step_in_cb, data, internal_timestamp_free);
    g_source_attach (source, priv->context);
    g_source_unref (source);
}

/**
 * ddd_step_leave:
 * @self::A DddStep object
 * @timestamp::The reference time to pass along to a new step.
 *
 * To step out of a step, means to deactivate the current object. The timestamp
 * given is a timestamp on which e.g. a trial ends and serves as a reference
 * time to start a new step in the experiment. Deactivating an object means
 * to activate its parent(if it has one), in such a way that a loop runs its
 * next iteration or a partlist starts the next part.
 */
void
ddd_step_leave(DddStep* self, gint64 tstamp)
{
    g_return_if_fail(DDD_IS_STEP(self));
    DddStepPrivate *priv = ddd_step_get_instance_private(self);
    GSource* source;

    InternalTimestamp *data = g_new(InternalTimestamp, 1);
    data->timestamp         = tstamp;
    data->step              = g_object_ref(self);

    source = g_idle_source_new();
    g_source_set_priority (source, G_PRIORITY_DEFAULT);
    g_source_set_callback (source,
                           step_out_cb,
                           data,
                           internal_timestamp_free);
    g_source_attach (source, priv->context);
    g_source_unref (source);
}

/**
 * ddd_step_set_parent:
 * @self:: a `DddStep`
 * @parent:(transfer full)The parent that gets signalled when this step is done.
 *
 * Sets the parent of the current `DddStep` object This parent is signalled
 * when we step out of the current step, causing an interation of a loop part
 * or starts a new Stepping construct.
 */
void
ddd_step_set_parent(DddStep* self, DddStep* parent)
{
    g_return_if_fail(DDD_IS_STEP(self));
    DddStepPrivate *priv = ddd_step_get_instance_private(self);

    g_clear_object(&priv->parent);
    if (parent)
        priv->parent = g_object_ref(parent);
}

/**
 * ddd_step_get_parent:
 * @self:: a `DddStep`
 *
 * return:(transfer none): The parent of this object
 */
DddStep*
ddd_step_get_parent(DddStep*self)
{
    g_return_val_if_fail(DDD_IS_STEP(self), NULL);
    DddStepPrivate *priv = ddd_step_get_instance_private(self);
    return priv->parent;
}

/**
 * ddd_step_activate:
 * @self:: a DddStep instance
 */
 void
 ddd_step_activate(DddStep* step, gint64 timestamp)
{
     g_return_if_fail(DDD_IS_STEP(step));
     DddStepClass *klass = DDD_STEP_GET_CLASS(step);

     klass->activate(step, timestamp);
}

/**
 * ddd_step_get_main_context:Obtain the context in which the steps will queue
 * their events
 * @self:: a DddStep instance
 *
 * Especially for deriving instance it might be handy to obtain the maincontext
 * so that the deriving classes can queue there events in the same context where
 * they were instantiated.
 *
 * Returns :(transfer none): `GMainContext*` which was the thread default context
 *                           when this step was instantiated.
 */
GMainContext*
ddd_step_get_main_context(DddStep* step)
{
    g_return_val_if_fail(DDD_IS_STEP(step), NULL);
    DddStepPrivate *priv = ddd_step_get_instance_private(step);

    return priv->context;
}
