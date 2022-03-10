
#include "ddd-loop.h"
#include "enum-types.h"

typedef struct DddLoopPrivate {
    gint64  index;
    gint64  stop;
    gint64  increment;
    gint    condition;
} DddLoopPrivate;

typedef struct Iteration {
    DddLoop* loop;
    gint64   timestamp;
}Iteration;

static void
iteration_free(gpointer iteration)
{
    Iteration *iter = iteration;
    g_clear_object(&iter->loop);
    g_free(iter);
}

G_DEFINE_TYPE_WITH_PRIVATE(DddLoop, ddd_loop, DDD_TYPE_STEP)

typedef enum {
    ITERATION,
    NUM_SIGNALS
} DddStepSignal;

typedef enum {
    PROP_NULL,
    PROP_INDEX,
    PROP_STOP,
    PROP_INCREMENT,
    PROP_CONDITION,
    NUM_PROPERTIES
} DddLoopProperty;

static void
ddd_loop_set_property (GObject      *object,
                       guint         property_id,
                       const GValue *value,
                       GParamSpec   *pspec)
{
    DddLoop *self = DDD_LOOP(object);

    switch ((DddLoopProperty) property_id)
    {
        case PROP_INDEX:
            ddd_loop_set_index(self, g_value_get_int64(value));
            break;
        case PROP_INCREMENT:
            ddd_loop_set_increment(self, g_value_get_int64(value));
            break;
        case PROP_STOP:
            ddd_loop_set_stop(self, g_value_get_int64(value));
            break;
        case PROP_CONDITION:
            ddd_loop_set_condition(self, g_value_get_enum(value));
            break;
        default:
            /* We don't have any other property... */
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
            break;
    }
}

static void
ddd_loop_get_property (GObject    *object,
                       guint       property_id,
                       GValue     *value,
                       GParamSpec *pspec)
{
    DddLoop *self = DDD_LOOP(object);
    DddLoopPrivate* priv = ddd_loop_get_instance_private(self);

    switch ((DddLoopProperty) property_id)
    {
        case PROP_INDEX:
            g_value_set_int64(value, priv->index);
            break;
        case PROP_STOP:
            g_value_set_int64(value, priv->stop);
            break;
        case PROP_INCREMENT:
            g_value_set_int64(value, priv->increment);
            break;
        case PROP_CONDITION:
            g_value_set_enum(value, priv->condition);
            break;
        default:
            /* We don't have any other property... */
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
            break;
    }
}


static GParamSpec *obj_properties[NUM_PROPERTIES] = {NULL};
static guint loop_signals[NUM_SIGNALS];

static gboolean
iter_cb(gpointer data)
{
    Iteration* iter = data;

    gboolean kontinue = ddd_loop_test(iter->loop);
    if (kontinue) {
        g_signal_emit(
                iter->loop,
                loop_signals[ITERATION],
                0,
                ddd_loop_get_index(iter->loop),
                iter->timestamp
        );
    }
    else {
        ddd_step_leave(DDD_STEP(iter->loop), iter->timestamp);
    }
    return G_SOURCE_REMOVE;
}

static void
ddd_loop_activate(DddStep* step, gint64 timestamp)
{
    DddLoop *self = DDD_LOOP(step);

    ddd_loop_iterate(self, timestamp);

    DDD_STEP_CLASS(ddd_loop_parent_class)->activate(step, timestamp);
}

static void
ddd_loop_iteration(DddLoop* self, gint64 index, gint64 timestamp)
{
    (void) index;
    (void) timestamp;
    DddLoopPrivate *priv = ddd_loop_get_instance_private(self);
    priv->index += priv->increment;
}

static void
ddd_loop_class_init(DddLoopClass* klass)
{
    GObjectClass *obj_class = G_OBJECT_CLASS(klass);
    DddStepClass *step_class = DDD_STEP_CLASS(klass);

    obj_class->set_property = ddd_loop_set_property;
    obj_class->get_property = ddd_loop_get_property;

    step_class->activate = ddd_loop_activate;

    klass->iteration    = ddd_loop_iteration;

    /**
     * DddLoop:index:
     *
     * The current index of the loop. It can be used to index other arrays
     * to obtain useful information.
     */
    obj_properties[PROP_INDEX] = g_param_spec_int64(
            "index",
            "Index",
            "The index at which the loop currently is.",
            G_MININT64,
            G_MAXINT64,
            0,
            G_PARAM_READWRITE
            );

    /**
     * DddLoop:stop:
     *
     * The amount that index compared with prior to each iteration of the loop.
     */
    obj_properties[PROP_STOP] = g_param_spec_int64(
            "stop",
            "Stop",
            "The value used to check whether the loop should stop",
            G_MININT64,
            G_MAXINT64,
            0,
            G_PARAM_READWRITE
            );

    /**
     * DddLoop:increment:
     *
     * The amount that index is incremented with at the end of each iteration
     * of the loop.
     */
    obj_properties[PROP_INCREMENT] = g_param_spec_int64(
            "increment",
            "Increment",
            "The value used to increment the index with after each iteration of the loop",
            G_MININT64,
            G_MAXINT64,
            1,
            G_PARAM_READWRITE | G_PARAM_CONSTRUCT
            );

    /**
     * DddLoop:condition:
     *
     * The logical operation that is performed to check whether the loop
     * should terminate.DDD_LOOP_CONDITION_LESS is comparable to
     * ```c
     * for(index; index < stop; index += increment) {
     *     //
     * }
     * ```
     * and DDD_LOOP_CONDITION_EQUAL is comparable to
     * ```c
     * for(index; index == stop; index += increment) {
     *     //
     * }
     * ```
     * etc.
     */
    obj_properties[PROP_CONDITION] = g_param_spec_enum(
            "condition",
            "Condition",
            "The condition used to determine whether to stop the loop",
            DDD_TYPE_LOOP_CONDITION,
            DDD_LOOP_CONDITION_LESS,
            G_PARAM_READWRITE
            );

    g_object_class_install_properties(
            obj_class, NUM_PROPERTIES, obj_properties
            );

    /**
     * DddLoop::iteration
     * @index:The current index for this iteration.
     * @timestamp:The timestamp of the leave event when previous step/iteration/
     *            event stopped.
     *
     * This signal is emitted on each iteration of the loop. The class handler
     * will increment the index in the 3rd stage of signal emission.
     */
    loop_signals[ITERATION] = g_signal_new(
            "iteration",
            DDD_TYPE_LOOP,
            G_SIGNAL_RUN_LAST,
            G_STRUCT_OFFSET(DddLoopClass, iteration),
            NULL,
            NULL,
            NULL,
            G_TYPE_NONE,
            2,
            G_TYPE_INT64,
            G_TYPE_INT64
            );
}

static void
ddd_loop_init(DddLoop* self)
{
    (void) self;
}

/* ***************** public functions ******************* */

/**
 * ddd_loop_new:(constructor)
 *
 * Returns a new loop.
 * Returns:a new loop with its default - not so useful - parameters.
 */
DddLoop*
ddd_loop_new()
{
    return g_object_new(DDD_TYPE_LOOP, NULL);
}

/**
 * ddd_loop_new_full:(constructor):
 *
 * Creates a new loop with all parameters specified.
 * Returns: A new loop with all parameters specified.
 */
DddLoop*
ddd_loop_new_full(gint64 index,
                  gint64 stop,
                  gint64 increment,
                  DddLoopCondition condition
                  )
{
     return g_object_new(DDD_TYPE_LOOP,
                         "index", index,
                         "stop", stop,
                         "increment", increment,
                         "condition", condition,
                         NULL
                         );
}

/**
 * ddd_loop_destroy:(destructor):
 * @self:The loop to destroy
 */
void
ddd_loop_destroy(DddLoop* self)
{
    g_return_if_fail(DDD_IS_LOOP(self));
    g_object_unref(self);
}

/**
 * ddd_loop_iterate:
 * @self:the `DddLoop` to iterate.
 * @timestamp: A timestamp of an event in the past that causes an iteration
 *             of this loop.
 *
 * Calling this function will iterate the loop, if the final iteration has been
 * reached, we will step out of the loop and continue the steps of the parent.
 */
void
ddd_loop_iterate(DddLoop* self, gint64 timestamp)
{
    GSource *source;
    Iteration *iter = g_new(Iteration, 1);

    iter->loop = g_object_ref(self);
    iter->timestamp = timestamp;

    /*
     * g_main_context_invoke and friends might recurse into stackoverflow, when
     * calling the main context, hence we create an idle source manually. If
     * we use another GThread's GMainContext this wouldn't be the case.
     */
    source = g_idle_source_new();
    g_source_set_priority (source, G_PRIORITY_DEFAULT);
    g_source_set_callback (source, iter_cb, iter, iteration_free);
    g_source_attach (source, ddd_step_get_main_context(DDD_STEP(self)));
    g_source_unref (source);
}

/**
 * ddd_loop_set_index:
 * @self: a `DddLoop` instance
 * @index: the index that will be used for the next iteration of the loop.
 */
void
ddd_loop_set_index(DddLoop* self, gint64 index)
{
    g_return_if_fail(DDD_IS_LOOP(self));
    DddLoopPrivate *priv = ddd_loop_get_instance_private(self);

    priv->index = index;
}

/**
 * ddd_loop_get_index:
 * @self: a `DddLoop` instance.
 *
 * Returns: The current index of the loop.
 */
gint64
ddd_loop_get_index(DddLoop* self)
{
    g_return_val_if_fail(DDD_IS_LOOP(self), 0);
    DddLoopPrivate *priv = ddd_loop_get_instance_private(self);

    return priv->index;
}

/**
 * ddd_loop_set_stop:
 * @self: A `DddLoop` instance
 * @stop: The value to compare index with after each iteration of the loop.
 */
void
ddd_loop_set_stop(DddLoop* self, gint64 stop)
{
    g_return_if_fail(DDD_IS_LOOP(self));
    DddLoopPrivate *priv = ddd_loop_get_instance_private(self);

    priv->stop = stop;
}

/**
 * ddd_get_stop:
 * @self: a `DddLoop` instance
 * @return The amount index is compared with to determine whether
 *         to stop the loop.
 */
gint64
ddd_loop_get_stop(DddLoop* self)
{
    g_return_val_if_fail(DDD_IS_LOOP(self), 0);
    DddLoopPrivate *priv = ddd_loop_get_instance_private(self);

    return priv->stop;
}

/**
 * ddd_loop_set_increment:
 * @self: a `DddLoop` instance.
 * @increment: The new increment value
 *
 * Sets the DddLoop:increment property; the amount that is added to the index
 * after each iteration of the loop.
 */
void
ddd_loop_set_increment(DddLoop* self, gint64 increment)
{
    g_return_if_fail(DDD_IS_LOOP(self));
    DddLoopPrivate *priv = ddd_loop_get_instance_private(self);

    priv->increment = increment;
}

/**
 * ddd_loop_get_increment:
 * @self: A `DddLoop` instance.
 *
 * Returns:The amount that is added to the DddLoop:index after each iteration.
 */
gint64
ddd_loop_get_increment(DddLoop* self)
{
    g_return_val_if_fail(DDD_IS_LOOP(self), 0);
    DddLoopPrivate *priv = ddd_loop_get_instance_private(self);

    return priv->increment;
}

/**
 * ddd_loop_set_condition:
 * @self:The #DddLoop to give a new loop condition.
 * @cond:The DddLoopCondition to use for testing if @self should continue
 *       to iterate.
 *
 * Set the loop condition.
 */
void
ddd_loop_set_condition(DddLoop* self, DddLoopCondition cond)
{
    g_return_if_fail(DDD_IS_LOOP(self));
    DddLoopPrivate *priv = ddd_loop_get_instance_private(self);

    priv->condition = cond;
}

/**
 * ddd_lo
 * @self: a #DddLoop Instance
 *
 * Returns: The loop condition used to test whether the loop should continue to
 *          iterate.
 */
DddLoopCondition
ddd_loop_get_condition(DddLoop* self)
{
    g_return_val_if_fail(DDD_IS_LOOP(self), 0);
    DddLoopPrivate *priv = ddd_loop_get_instance_private(self);

    return priv->condition;
}

/**
 * ddd_loop_test:
 * @self: The loop instance
 *
 * This function is mainly for internal use, it used to determine whether
 * the loop should continue to iterate or stop.
 *
 * Returns: TRUE if the loop should continue, FALSE otherwise.
 */
gboolean
ddd_loop_test(DddLoop* self) {
    g_return_val_if_fail(DDD_IS_LOOP(self), FALSE);
    DddLoopPrivate *priv = ddd_loop_get_instance_private(self);

    switch (priv->condition) {
        case DDD_LOOP_CONDITION_LESS:          return priv->index <  priv->stop;
        case DDD_LOOP_CONDITION_LESS_EQUAL:    return priv->index <= priv->stop;
        case DDD_LOOP_CONDITION_EQUAL:         return priv->index == priv->stop;
        case DDD_LOOP_CONDITION_GREATER_EQUAL: return priv->index >= priv->stop;
        case DDD_LOOP_CONDITION_GREATER:       return priv->index >  priv->stop;
        default:
            g_assert_not_reached();
            return FALSE;
    }
}
