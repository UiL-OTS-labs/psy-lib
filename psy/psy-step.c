
#include "psy-step.h"

#include "psy-loop.h"

/**
 * PsyStep:
 *
 * Instances of `PsyStep` allow control flow in the context
 * of an event loop. When an instance of `PsyStep` is created it
 * will be hooked to a GMainContext. Via this context entering
 * and leaving step happens using events. This events are typically
 * accompanied by a `PsyTimePoint` that indicates when this part of
 * the
 */

G_DEFINE_QUARK(psy - step - error - quark, psy_step_error)

typedef struct _PsyStepPrivate {
    PsyStep      *parent;
    GMainContext *context;
    gboolean      active;
} PsyStepPrivate;

typedef struct {
    PsyStep      *step;
    PsyTimePoint *timestamp;
} InternalTimestamp;

static void
internal_timestamp_free(gpointer timestamp)
{
    InternalTimestamp *tstamp = timestamp;
    g_clear_object(&tstamp->step);
    g_clear_pointer(&tstamp->timestamp, psy_time_point_free);
    g_free(timestamp);
}

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE(PsyStep, psy_step, G_TYPE_OBJECT)

typedef enum { SIG_ENTER, SIG_ACTIVATE, SIG_LEAVE, NUM_SIGNALS } PsyStepSignal;

typedef enum {
    PROP_NULL,
    PROP_PARENT,
    PROP_ACTIVE,
    NUM_PROPERTIES
} PsyStepProperty;

static GParamSpec *obj_properties[NUM_PROPERTIES] = {NULL};
static guint       step_signals[NUM_SIGNALS];

static void
psy_step_post_activate(PsyStep *self);

static void
psy_step_set_property(GObject      *object,
                      guint         property_id,
                      const GValue *value,
                      GParamSpec   *pspec)
{
    PsyStep *self = PSY_STEP(object);
    // PsyStepPrivate *priv = psy_step_get_instance_private(self);

    switch ((PsyStepProperty) property_id) {
    case PROP_PARENT:
        psy_step_set_parent(self, g_value_get_object(value));
        break;
    default:
        /* We don't have any other property... */
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static void
psy_step_get_property(GObject    *object,
                      guint       property_id,
                      GValue     *value,
                      GParamSpec *pspec)
{
    PsyStep        *self = PSY_STEP(object);
    PsyStepPrivate *priv = psy_step_get_instance_private(self);

    switch ((PsyStepProperty) property_id) {
    case PROP_PARENT:
        g_value_set_object(value, priv->parent);
        break;
    case PROP_ACTIVE:
        g_value_set_boolean(value, psy_step_get_active(self));
        break;
    default:
        /* We don't have any other property... */
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static void
psy_step_dispose(GObject *gobject)
{
    PsyStepPrivate *priv = psy_step_get_instance_private(PSY_STEP(gobject));

    g_clear_object(&priv->parent);
    if (priv->context) {
        g_main_context_unref(priv->context);
        priv->context = NULL;
    }

    G_OBJECT_CLASS(psy_step_parent_class)->dispose(gobject);
}

static void
psy_step_finalize(GObject *gobject)
{
    PsyStepPrivate *priv = psy_step_get_instance_private(PSY_STEP(gobject));

    (void) priv;

    G_OBJECT_CLASS(psy_step_parent_class)->finalize(gobject);
}

static void
psy_step_init(PsyStep *self)
{
    PsyStepPrivate *priv = psy_step_get_instance_private(self);
    priv->context        = g_main_context_ref_thread_default();
}

static void
step_activate(PsyStep *self, PsyTimePoint *timestamp)
{
    (void) timestamp;
    g_debug("Activating instance of %s", G_OBJECT_TYPE_NAME(self));
    PsyStepPrivate *priv = psy_step_get_instance_private(self);
    priv->active         = TRUE;
}

static void
step_post_activate(PsyStep *self)
{
    g_debug("Post Activating instance of %s", G_OBJECT_TYPE_NAME(self));
}

static void
step_deactivate(PsyStep *self, PsyTimePoint *timestamp)
{
    g_debug("Deactivating instance of %s", G_OBJECT_TYPE_NAME(self));
    PsyStepPrivate *priv = psy_step_get_instance_private(self);
    priv->active         = FALSE;
    if (priv->parent) {
        psy_step_activate(priv->parent, timestamp);
    }
}

static void
step_on_enter(PsyStep *self, PsyTimePoint *timestamp)
{
    g_debug("Entering instance of %s", G_OBJECT_TYPE_NAME(self));
    psy_step_activate(self, timestamp);
}

static void
step_on_leave(PsyStep *self, PsyTimePoint *timestamp)
{
    PsyStepClass   *klass = PSY_STEP_GET_CLASS(self);
    PsyStepPrivate *priv  = psy_step_get_instance_private(self);
    if (priv->parent)
        psy_step_post_activate(priv->parent);
    klass->deactivate(self, timestamp);
}

static void
psy_step_class_init(PsyStepClass *klass)
{
    GObjectClass *obj_class = G_OBJECT_CLASS(klass);

    obj_class->set_property = psy_step_set_property;
    obj_class->get_property = psy_step_get_property;
    obj_class->dispose      = psy_step_dispose;
    obj_class->finalize     = psy_step_finalize;

    klass->on_enter      = step_on_enter;
    klass->activate      = step_activate;
    klass->on_leave      = step_on_leave;
    klass->post_activate = step_post_activate;
    klass->deactivate    = step_deactivate;

    /* Add properties and signals to the interface here */

    /**
     * PsyStep:parent:
     *
     * The parent is the object that gets activated when we leave the current
     * step, in such a way that structure of an experiment continues.
     * We do not own a reference to the parent, we assume the parent own this
     * step. The parent may be NULL for the top most Step object.
     */
    obj_properties[PROP_PARENT] = g_param_spec_object(
        "parent",
        "Parent",
        "The parent object that gets signaled when this step is over.",
        PSY_TYPE_STEP,
        G_PARAM_READWRITE);

    obj_properties[PROP_ACTIVE]
        = g_param_spec_boolean("active",
                               "Active",
                               "Whether we are in the current step",
                               FALSE,
                               G_PARAM_READABLE);

    g_object_class_install_properties(
        obj_class, NUM_PROPERTIES, obj_properties);

    g_assert(G_TYPE_FROM_CLASS(obj_class) == PSY_TYPE_STEP);

    /**
     * PsyStep::enter:
     * @self: the step that we are stepping into.
     * @timestamp: The timestamp that counts as a reference for the
     *             current step.
     *
     * This signal is emitted when a step is entered.
     */
    step_signals[SIG_ENTER]
        = g_signal_new("enter",
                       G_TYPE_FROM_CLASS(obj_class),
                       G_SIGNAL_RUN_LAST,
                       G_STRUCT_OFFSET(PsyStepClass, on_enter),
                       NULL,
                       NULL,
                       NULL,
                       G_TYPE_NONE,
                       1,
                       PSY_TYPE_TIME_POINT);

    /**
     * PsyStep::activate:
     * @self: the step that we are activating.
     * @timestamp: The timestamp that counts as a reference for the
     *             current step.
     *
     * This signal is emitted when a step is activated. For trials this
     * means you can start the presentation of stimuli, whereas for
     * loops or stepping stones this means to enter there children.
     */
    step_signals[SIG_ACTIVATE]
        = g_signal_new("activate",
                       G_TYPE_FROM_CLASS(obj_class),
                       G_SIGNAL_RUN_LAST,
                       G_STRUCT_OFFSET(PsyStepClass, activate),
                       NULL,
                       NULL,
                       NULL,
                       G_TYPE_NONE,
                       1,
                       PSY_TYPE_TIME_POINT);

    /**
     * PsyStep::leave:
     * @self: the step that we are stepping out of.
     * @timestamp: The timestamp of the event that finished the current
     *             step. This timestamp propagates to the enter signal
     *             of a next step.
     *
     * This signal is emitted when leaving a step.
     */
    step_signals[SIG_LEAVE]
        = g_signal_new("leave",
                       G_TYPE_FROM_CLASS(obj_class),
                       G_SIGNAL_RUN_LAST,
                       G_STRUCT_OFFSET(PsyStepClass, on_leave),
                       NULL,
                       NULL,
                       NULL,
                       G_TYPE_NONE,
                       1,
                       PSY_TYPE_TIME_POINT);
}

static gboolean
step_in_cb(gpointer data)
{
    InternalTimestamp *timestamp = data;
    PsyStepPrivate    *priv = psy_step_get_instance_private(timestamp->step);
    g_assert(g_main_context_is_owner(priv->context));
    g_assert(PSY_IS_STEP(timestamp->step));
    g_signal_emit(
        timestamp->step, step_signals[SIG_ENTER], 0, timestamp->timestamp);

    return G_SOURCE_REMOVE;
}

static gboolean
step_out_cb(gpointer data)
{
    InternalTimestamp *timestamp = data;
    PsyStepPrivate    *priv = psy_step_get_instance_private(timestamp->step);
    g_assert(g_main_context_is_owner(priv->context));
    g_signal_emit(
        timestamp->step, step_signals[SIG_LEAVE], 0, timestamp->timestamp);
    return G_SOURCE_REMOVE;
}

/**
 * psy_step_enter:
 * @self::A PsyStep object
 * @tstamp: The timestamp at which the step is entered
 *
 * To enter a step, means to activate the current object. The timestamp
 * given is a timestamp on which previously a trial ended or the start
 * of an experiment.
 */
void
psy_step_enter(PsyStep *self, PsyTimePoint *tstamp)
{
    g_return_if_fail(PSY_IS_STEP(self));
    PsyStepPrivate *priv = psy_step_get_instance_private(self);
    GSource        *source;

    InternalTimestamp *data = g_new(InternalTimestamp, 1);
    data->timestamp         = psy_time_point_copy(tstamp);
    data->step              = g_object_ref(self);

    source = g_idle_source_new();
    g_source_set_priority(source, G_PRIORITY_DEFAULT);
    g_source_set_callback(source, step_in_cb, data, internal_timestamp_free);
    g_source_attach(source, priv->context);
    g_source_unref(source);
}

/**
 * psy_step_leave:
 * @self::A PsyStep object
 * @tstamp::The reference time to pass along to a new step.
 *
 * To step out of a step, means to deactivate the current object. The timestamp
 * given is a timestamp on which e.g. a trial ends and serves as a reference
 * time to start a new step in the experiment. Deactivating an object means
 * to activate its parent(if it has one), in such a way that a loop runs its
 * next iteration or a partlist starts the next part.
 */
void
psy_step_leave(PsyStep *self, PsyTimePoint *tstamp)
{
    g_return_if_fail(PSY_IS_STEP(self));
    PsyStepPrivate *priv = psy_step_get_instance_private(self);
    GSource        *source;

    InternalTimestamp *data = g_new(InternalTimestamp, 1);
    data->timestamp         = psy_time_point_copy(tstamp);
    data->step              = g_object_ref(self);

    source = g_idle_source_new();
    g_source_set_priority(source, G_PRIORITY_DEFAULT);
    g_source_set_callback(source, step_out_cb, data, internal_timestamp_free);
    g_source_attach(source, priv->context);
    g_source_unref(source);
}

/**
 * psy_step_set_parent:
 *
 * self: a `PsyStep`
 * parent:(transfer full)The parent that gets signalled when this step is done.
 *
 * Sets the parent of the current `PsyStep` object This parent is signalled
 * when we step out of the current step, causing an interation of a loop part
 * or starts a new Stepping construct.
 */
void
psy_step_set_parent(PsyStep *self, PsyStep *parent)
{
    g_return_if_fail(PSY_IS_STEP(self));
    PsyStepPrivate *priv = psy_step_get_instance_private(self);

    g_clear_object(&priv->parent);
    if (parent)
        priv->parent = g_object_ref(parent);
}

/**
 * psy_step_get_parent:
 * @self:: a `PsyStep`
 *
 * return:(transfer none): The parent of this object
 */
PsyStep *
psy_step_get_parent(PsyStep *self)
{
    g_return_val_if_fail(PSY_IS_STEP(self), NULL);
    PsyStepPrivate *priv = psy_step_get_instance_private(self);
    return priv->parent;
}

/**
 * psy_step_activate:
 * @self:: a PsyStep instance
 *
 * emits the activate signal
 */
void
psy_step_activate(PsyStep *step, PsyTimePoint *timestamp)
{
    g_return_if_fail(PSY_IS_STEP(step));
    g_return_if_fail(timestamp != NULL);

    g_signal_emit(step, step_signals[SIG_ACTIVATE], 0, timestamp);
}

gboolean
psy_step_get_active(PsyStep *self)
{
    g_return_val_if_fail(PSY_IS_STEP(self), FALSE);

    PsyStepPrivate *priv = psy_step_get_instance_private(self);
    return priv->active;
}

/**
 * psy_step_get_loop_index_recursive:(skip)
 *
 * This is the worker function for psy_step_get_loop_index
 */
static gint64
psy_step_get_loop_index_recursive(PsyStep *self, guint nth_loop, GError **error)
{
    //    g_debug("step %p, type = %s, nth_loop %u",
    //            (void *) self,
    //            self ? G_OBJECT_TYPE_NAME(self) : "Null",
    //            nth_loop);
    if (!self) {
        g_set_error(error,
                    PSY_STEP_ERROR,
                    PSY_STEP_ERROR_NO_SUCH_LOOP,
                    "Unable to retrieve another loop.");
        return G_MININT64;
    }
    if (PSY_IS_LOOP(self)) {
        if (nth_loop == 0)
            return psy_loop_get_index(PSY_LOOP(self));
        else {
            PsyStep *parent = psy_step_get_parent(self);
            return psy_step_get_loop_index_recursive(
                parent, nth_loop - 1, error);
        }
    }
    else { // It's a Step, but not a loop.
        PsyStep *parent = psy_step_get_parent(self);
        return psy_step_get_loop_index(parent, nth_loop, error);
    }
}

/**
 * psy_step_get_loop_index:
 * @self: The step that should be embedded in a loop.
 * @nth_loop: The nth_loop for which the index should be returned. If zero
 *            the index of the first loop is returned, if 1 the index of
 *            the second loop is returned etc
 * @error: This error might be set when the step is not embedded in an
 *         instance of [class@Loop] or nth_loop is greater or equal to
 *         the number of loops this step is embedded in.
 *
 * Steps are embedded in each other in a tree like fashion. Sometimes, from
 * a trial it might be handy to retrieve the index of the loop you are in.
 * you may be able to traverse up the tree to fetch the loop, or this function
 * is able to do it for you. This function, may be called from activate, enter
 * and leave event handlers to obtain the active index of the loop we are in.
 *
 * When you call this function on a loop, its just a difficult way to obtain the
 * index :-).
 *
 * Returns: The index of the (nth_loop + 1) encountered in @self and its
 *          parents. It might return G_MININT64 when no suitable index can be
 *          found.
 */
gint64
psy_step_get_loop_index(PsyStep *self, guint nth_loop, GError **error)
{
    g_return_val_if_fail(PSY_IS_STEP(self), G_MININT64);
    g_return_val_if_fail(error == NULL || *error == NULL, G_MININT64);

    return psy_step_get_loop_index_recursive(self, nth_loop, error);
}

/**
 * psy_step_get_loop_indices:
 * @step, the step to retrieve the loop indices for
 * @result:(out callee-allocates)(array length=size)(transfer full):
 *          The indices are returned in with the outer most loop first.
 * @size:(out): the size of the returned indices array.
 *
 * Get the indices of loops this step is embedded in.
 *
 * for a regular loop
 * ``` python
 * for i in range(10):
 *     for j in range(10):
 *         indices = loop.get_loop_indices() # it's pseudo code
 *         assert indices[0] == i and indices[1] == j
 * ```
 *
 */
void
psy_step_get_loop_indices(PsyStep *step, gint64 **result, guint *size)
{
    g_return_if_fail(PSY_IS_STEP(step));
    g_return_if_fail(result != NULL && *result == NULL);
    g_return_if_fail(size != NULL);

    GPtrArray *loops = g_ptr_array_new();
    PsyStep   *iter  = step;
    while (iter) {
        if (PSY_IS_LOOP(iter)) {
            g_ptr_array_add(loops, iter);
        }
        iter = psy_step_get_parent(iter);
    }

    gint64 *ret = g_malloc(loops->len * sizeof(gint64));
    for (guint i = 0, j = loops->len - 1; i < loops->len; i++, j--) {
        ret[j] = psy_loop_get_index(loops->pdata[i]);
    }
    *result = ret;
    *size   = loops->len;
    g_ptr_array_unref(loops);
}

/**
 * psy_step_get_main_context:
 * self: a `PsyStep` instance
 *
 * Obtain the context in which the steps will queue their events
 *
 * Especially for deriving instance it might be handy to obtain the maincontext
 * so that the deriving classes can queue there events in the same context where
 * they were instantiated.
 *
 * Returns:(transfer none): `GMainContext` which was the thread default context
 *                           when this step was instantiated.
 */
GMainContext *
psy_step_get_main_context(PsyStep *step)
{
    g_return_val_if_fail(PSY_IS_STEP(step), NULL);
    PsyStepPrivate *priv = psy_step_get_instance_private(step);

    return priv->context;
}

/**
 * psy_step_post_activate:(skip)
 * @step: The step that should run it post_activate:
 *
 * Runs the post_activate virtual function. When a child of a Loop is left
 * it wants to update the index of a loop, the post activate can do that.
 */
void
psy_step_post_activate(PsyStep *self)
{
    g_return_if_fail(PSY_IS_STEP(self));

    PsyStepClass *cls = PSY_STEP_GET_CLASS(self);
    g_return_if_fail(cls->post_activate);
    cls->post_activate(self);
}
