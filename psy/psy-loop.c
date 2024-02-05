
#include "psy-loop.h"
#include "enum-types.h"

typedef struct PsyLoopPrivate {
    gint64 index;
    gint64 stop;
    gint64 increment;
    gint   condition;
} PsyLoopPrivate;

typedef struct Iteration {
    PsyLoop      *loop;
    PsyTimePoint *timestamp;
} Iteration;

static void
iteration_free(gpointer iteration)
{
    Iteration *iter = iteration;
    g_clear_object(&iter->loop);
    g_clear_pointer(&iter->timestamp, psy_time_point_free);
    g_free(iter);
}

G_DEFINE_TYPE_WITH_PRIVATE(PsyLoop, psy_loop, PSY_TYPE_STEP)

typedef enum { ITERATION, NUM_SIGNALS } PsyStepSignal;

typedef enum {
    PROP_NULL,
    PROP_INDEX,
    PROP_STOP,
    PROP_INCREMENT,
    PROP_CONDITION,
    NUM_PROPERTIES
} PsyLoopProperty;

static void
psy_loop_set_property(GObject      *object,
                      guint         property_id,
                      const GValue *value,
                      GParamSpec   *pspec)
{
    PsyLoop *self = PSY_LOOP(object);

    switch ((PsyLoopProperty) property_id) {
    case PROP_INDEX:
        psy_loop_set_index(self, g_value_get_int64(value));
        break;
    case PROP_INCREMENT:
        psy_loop_set_increment(self, g_value_get_int64(value));
        break;
    case PROP_STOP:
        psy_loop_set_stop(self, g_value_get_int64(value));
        break;
    case PROP_CONDITION:
        psy_loop_set_condition(self, g_value_get_enum(value));
        break;
    default:
        /* We don't have any other property... */
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static void
psy_loop_get_property(GObject    *object,
                      guint       property_id,
                      GValue     *value,
                      GParamSpec *pspec)
{
    PsyLoop        *self = PSY_LOOP(object);
    PsyLoopPrivate *priv = psy_loop_get_instance_private(self);

    switch ((PsyLoopProperty) property_id) {
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
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static GParamSpec *obj_properties[NUM_PROPERTIES] = {NULL};
static guint       loop_signals[NUM_SIGNALS];

static gboolean
iter_cb(gpointer data)
{
    Iteration *iter = data;

    gboolean kontinue = psy_loop_test(iter->loop);
    if (kontinue) {
        g_signal_emit(iter->loop,
                      loop_signals[ITERATION],
                      0,
                      psy_loop_get_index(iter->loop),
                      iter->timestamp);
    }
    else {
        psy_step_leave(PSY_STEP(iter->loop), iter->timestamp);
    }
    return G_SOURCE_REMOVE;
}

static void
psy_loop_activate(PsyStep *step, PsyTimePoint *timestamp)
{
    PsyLoop *self = PSY_LOOP(step);

    psy_loop_iterate(self, timestamp);

    PSY_STEP_CLASS(psy_loop_parent_class)->activate(step, timestamp);
}

static void
psy_loop_iteration(PsyLoop *self, gint64 index, PsyTimePoint *timestamp)
{
    (void) index;
    (void) timestamp;
    PsyLoopPrivate *priv = psy_loop_get_instance_private(self);
    priv->index += priv->increment;
}

static void
psy_loop_class_init(PsyLoopClass *klass)
{
    GObjectClass *obj_class  = G_OBJECT_CLASS(klass);
    PsyStepClass *step_class = PSY_STEP_CLASS(klass);

    obj_class->set_property = psy_loop_set_property;
    obj_class->get_property = psy_loop_get_property;

    step_class->activate = psy_loop_activate;

    klass->iteration = psy_loop_iteration;

    /**
     * PsyLoop:index:
     *
     * The current index of the loop. It can be used to index other arrays
     * to obtain useful information.
     */
    obj_properties[PROP_INDEX]
        = g_param_spec_int64("index",
                             "Index",
                             "The index at which the loop currently is.",
                             G_MININT64,
                             G_MAXINT64,
                             0,
                             G_PARAM_READWRITE);

    /**
     * PsyLoop:stop:
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
        G_PARAM_READWRITE);

    /**
     * PsyLoop:increment:
     *
     * The amount that index is incremented with at the end of each iteration
     * of the loop.
     */
    obj_properties[PROP_INCREMENT]
        = g_param_spec_int64("increment",
                             "Increment",
                             "The value used to increment the index with after "
                             "each iteration of the loop",
                             G_MININT64,
                             G_MAXINT64,
                             1,
                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

    /**
     * PsyLoop:condition:
     *
     * The logical operation that is performed to check whether the loop
     * should terminate.PSY_LOOP_CONDITION_LESS is comparable to
     * ```c
     * for(index; index < stop; index += increment) {
     *     //
     * }
     * ```
     * and PSY_LOOP_CONDITION_EQUAL is comparable to
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
        PSY_TYPE_LOOP_CONDITION,
        PSY_LOOP_CONDITION_LESS,
        G_PARAM_READWRITE);

    g_object_class_install_properties(
        obj_class, NUM_PROPERTIES, obj_properties);

    /**
     * PsyLoop::iteration
     * @loop: The loop that received the signal
     * @index:The current index for this iteration.
     * @timestamp:The timestamp of the leave event when previous step/iteration/
     *            event stopped.
     *
     * This signal is emitted on each iteration of the loop. The class handler
     * will increment the index in the 3rd stage of signal emission.
     */
    loop_signals[ITERATION]
        = g_signal_new("iteration",
                       PSY_TYPE_LOOP,
                       G_SIGNAL_RUN_LAST,
                       G_STRUCT_OFFSET(PsyLoopClass, iteration),
                       NULL,
                       NULL,
                       NULL,
                       G_TYPE_NONE,
                       2,
                       G_TYPE_INT64,
                       PSY_TYPE_TIME_POINT);
}

static void
psy_loop_init(PsyLoop *self)
{
    (void) self;
}

/* ***************** public functions ******************* */

/**
 * psy_loop_new:(constructor)
 *
 * Returns a new loop.
 *
 * Returns: a new loop with its default - not so useful - parameters. The
 *          instance may be freed with g_object_unref or psy_loop_free
 */
PsyLoop *
psy_loop_new(void)
{
    return g_object_new(PSY_TYPE_LOOP, NULL);
}

/**
 * psy_loop_new_full:(constructor):
 *
 * Creates a new loop with all parameters specified.
 *
 * Returns: A new loop with all parameters specified.
 *          May be freed with g_object_unref or psy_loop_free
 */
PsyLoop *
psy_loop_new_full(gint64           index,
                  gint64           stop,
                  gint64           increment,
                  PsyLoopCondition condition)
{
    // clang-format off
     return g_object_new(PSY_TYPE_LOOP,
                         "index", index,
                         "stop", stop,
                         "increment", increment,
                         "condition", condition,
                         NULL
                         );
    // clang-format on
}

/**
 * psy_loop_free:
 * @self:The loop to destroy
 */
void
psy_loop_free(PsyLoop *loop)
{
    g_return_if_fail(PSY_IS_LOOP(loop));
    g_object_unref(loop);
}

/**
 * psy_loop_iterate:
 * @self:the `PsyLoop` to iterate.
 * @timestamp: A timestamp of an event in the past that causes an iteration
 *             of this loop.
 *
 * Calling this function will iterate the loop, if the final iteration has been
 * reached, we will step out of the loop and continue the steps of the parent.
 */
void
psy_loop_iterate(PsyLoop *self, PsyTimePoint *timestamp)
{
    GSource   *source;
    Iteration *iter = g_new(Iteration, 1);

    iter->loop      = g_object_ref(self);
    iter->timestamp = psy_time_point_copy(timestamp);

    /*
     * g_main_context_invoke and friends might recurse into stackoverflow, when
     * calling the main context, hence we create an idle source manually. If
     * we use another GThread's GMainContext this wouldn't be the case.
     */
    source = g_idle_source_new();
    g_source_set_priority(source, G_PRIORITY_DEFAULT);
    g_source_set_callback(source, iter_cb, iter, iteration_free);
    g_source_attach(source, psy_step_get_main_context(PSY_STEP(self)));
    g_source_unref(source);
}

/**
 * psy_loop_set_index:
 * @self: a `PsyLoop` instance
 * @index: the index that will be used for the next iteration of the loop.
 */
void
psy_loop_set_index(PsyLoop *self, gint64 index)
{
    g_return_if_fail(PSY_IS_LOOP(self));
    PsyLoopPrivate *priv = psy_loop_get_instance_private(self);

    priv->index = index;
}

/**
 * psy_loop_get_index:
 * @self: a `PsyLoop` instance.
 *
 * Returns: The current index of the loop.
 */
gint64
psy_loop_get_index(PsyLoop *self)
{
    g_return_val_if_fail(PSY_IS_LOOP(self), 0);
    PsyLoopPrivate *priv = psy_loop_get_instance_private(self);

    return priv->index;
}

/**
 * psy_loop_set_stop:
 * @self: A `PsyLoop` instance
 * @stop: The value to compare index with after each iteration of the loop.
 */
void
psy_loop_set_stop(PsyLoop *self, gint64 stop)
{
    g_return_if_fail(PSY_IS_LOOP(self));
    PsyLoopPrivate *priv = psy_loop_get_instance_private(self);

    priv->stop = stop;
}

/**
 * psy_get_stop:
 * @self: a `PsyLoop` instance
 * @return The amount index is compared with to determine whether
 *         to stop the loop.
 */
gint64
psy_loop_get_stop(PsyLoop *self)
{
    g_return_val_if_fail(PSY_IS_LOOP(self), 0);
    PsyLoopPrivate *priv = psy_loop_get_instance_private(self);

    return priv->stop;
}

/**
 * psy_loop_set_increment:
 * @self: a `PsyLoop` instance.
 * @increment: The new increment value
 *
 * Sets the PsyLoop:increment property; the amount that is added to the index
 * after each iteration of the loop.
 */
void
psy_loop_set_increment(PsyLoop *self, gint64 increment)
{
    g_return_if_fail(PSY_IS_LOOP(self));
    PsyLoopPrivate *priv = psy_loop_get_instance_private(self);

    priv->increment = increment;
}

/**
 * psy_loop_get_increment:
 * @self: A `PsyLoop` instance.
 *
 * Returns:The amount that is added to the PsyLoop:index after each iteration.
 */
gint64
psy_loop_get_increment(PsyLoop *self)
{
    g_return_val_if_fail(PSY_IS_LOOP(self), 0);
    PsyLoopPrivate *priv = psy_loop_get_instance_private(self);

    return priv->increment;
}

/**
 * psy_loop_set_condition:
 * @self:The #PsyLoop to give a new loop condition.
 * @condition:The PsyLoopCondition to use for testing if @self should continue
 *       to iterate.
 *
 * Set the loop condition.
 */
void
psy_loop_set_condition(PsyLoop *self, PsyLoopCondition condition)
{
    g_return_if_fail(PSY_IS_LOOP(self));
    PsyLoopPrivate *priv = psy_loop_get_instance_private(self);

    priv->condition = condition;
}

/**
 * psy_lo
 * @self: a #PsyLoop Instance
 *
 * Returns: The loop condition used to test whether the loop should continue to
 *          iterate.
 */
PsyLoopCondition
psy_loop_get_condition(PsyLoop *self)
{
    g_return_val_if_fail(PSY_IS_LOOP(self), 0);
    PsyLoopPrivate *priv = psy_loop_get_instance_private(self);

    return priv->condition;
}

/**
 * psy_loop_test:
 * @self: The loop instance
 *
 * This function is mainly for internal use, it used to determine whether
 * the loop should continue to iterate or stop.
 *
 * Returns: TRUE if the loop should continue, FALSE otherwise.
 */
gboolean
psy_loop_test(PsyLoop *self)
{
    g_return_val_if_fail(PSY_IS_LOOP(self), FALSE);
    PsyLoopPrivate *priv = psy_loop_get_instance_private(self);

    switch (priv->condition) {
    case PSY_LOOP_CONDITION_LESS:
        return priv->index < priv->stop;
    case PSY_LOOP_CONDITION_LESS_EQUAL:
        return priv->index <= priv->stop;
    case PSY_LOOP_CONDITION_EQUAL:
        return priv->index == priv->stop;
    case PSY_LOOP_CONDITION_GREATER_EQUAL:
        return priv->index >= priv->stop;
    case PSY_LOOP_CONDITION_GREATER:
        return priv->index > priv->stop;
    default:
        g_assert_not_reached();
        return FALSE;
    }
}
