
#include "psy-loop.h"
#include "enum-types.h"

// clang-format off
/**
 * PsyLoop:
 *
 * # Loops are objects that help to setup a timeline for your experiment.
 *
 * Loops allow to run fragments of code in a loop as the name suggests. When
 * you are running a Psylib application, all events are handled in an event
 * loop [struct@GLib.MainLoop]. The mainloop handles events and triggers other
 * code that is written to handle those events.
 *
 * # The loop is generally started by entering it
 *
 * You can call the [method@Step.enter] on the loop in order to start it.
 * If the loop is embedded in a [class@SteppingStones], it will enter the loop
 * on your behalf. When you enter it the first time the iteration will be
 * started until an iteration is ran that for which
 * [property@Loop:index] [enum@LoopCondition] [property@Loop:stop] no longer
 * holds, then the loop will be left, activating the parent [class@Step],
 * continuing the steps of your experiment.
 *
 * Psy.Loops help with setting up events that trigger an iteration
 * of a Psy.Loop. You may choose two ways to do something on a iteration of the
 * loop:
 *
 * 1. **Connect to the [signal@Loop::iteration] signal**. This way you
 *    register a function to be called when the "iteration" signal is emitted.
 * 2. The preferred method should be to **override the,
 *    [vfunc@Loop.iteration] method.** From the iteration override you should
 *    remember to chainup to the parent, as the [class@Loop] as the base class
 *    handler is involved, in setting up the next iteration.
 *
 * The Loops are agnostic to whether the index is 0-based or 1-based, you can
 * set this up yourself. To demonstrate using python see examples/python/loop.py
 *
 * Looping from 0 - 9 might work like this in a language such as python
 * ```python
 * def on_iteration():
 *     print(f"index = {index}")
 *
 * loop = Psy.Loop(index=0, stop=10, increment=1, condition=Psy.LoopCondition.LESS)
 * loop.enter(clock.now())
 * ```
 *
 * So in order create a loop:
 *
 * 1. you specify the **index** at which the loop starts.
 * 1. You specify the **stop** The stop is used in conjunction with the
 *    index to see wheter the loop should continue to iterate.
 * 1. You specify the **increment** to increment the index with after each
 *    iteration. This may be negative, in order to count down
 * 1. you specify the **condition** to check whether the loop should continue.
 *    This is a member from the [enum@LoopCondition]. See the documentation of
 *    that enum to see what the mean.
 *
 * And you typically to this at creation time of the loop, but you are totally
 * free to do this while running the loop.
 *
 */
// clang-format on

typedef struct PsyLoopPrivate {
    gint64   index;
    gint64   stop;
    gint64   increment;
    gint     condition;
    PsyStep *child;
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
    PROP_CHILD,
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
    case PROP_CHILD:
        psy_loop_set_child(self, g_value_get_object(value));
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
    case PROP_CHILD:
        g_value_set_object(value, psy_loop_get_child(self));
        break;
    default:
        /* We don't have any other property... */
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static void
psy_loop_dispose(GObject *self)
{
    PsyLoopPrivate *priv = psy_loop_get_instance_private(PSY_LOOP(self));

    g_clear_object(&priv->child);

    G_OBJECT_CLASS(psy_loop_parent_class)->dispose(self);
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
    if (priv->child) {
        psy_step_enter(priv->child, timestamp);
    }
    else {
        PSY_STEP_GET_CLASS(self)->post_activate(PSY_STEP(self));
    }
}

static void
psy_loop_post_activate(PsyStep* self)
{
    PsyLoopPrivate *priv = psy_loop_get_instance_private(PSY_LOOP(self));
    priv->index += priv->increment;

    PSY_STEP_CLASS(psy_loop_parent_class)->post_activate(self);
}

static void
psy_loop_class_init(PsyLoopClass *klass)
{
    GObjectClass *obj_class  = G_OBJECT_CLASS(klass);
    PsyStepClass *step_class = PSY_STEP_CLASS(klass);

    obj_class->set_property = psy_loop_set_property;
    obj_class->get_property = psy_loop_get_property;
    obj_class->dispose      = psy_loop_dispose;

    step_class->activate = psy_loop_activate;
    step_class->post_activate = psy_loop_post_activate;

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

    /**
     * PsyLoop:child:
     *
     * The child step that is entered on each iteration of the loop. This
     * step is entered on each iteration of the loop if it is set, otherwise
     * the iteration signal should handle continuation to the next
     * step/iteration.
     */
    obj_properties[PROP_CHILD] = g_param_spec_object(
        "child",
        "Child",
        "The child step that is entered on each iteration of the loop",
        PSY_TYPE_STEP,
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
 * @self:the instance of [class@Loop] to iterate.
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
 * @self: a [class@Loop] instance
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
 * psy_loop_set_child:
 * @self: The psyloop in need of a child, that can be run at every iteration
 *        of the loop.
 * @child: (nullable)(transfer full): The Step that is going to be activated on
 *         every loop. Make sure that the step is nicely reset on enter or
 *         leave, because otherwise you might inherit values inside the child
 *         step from a previous iteration.
 *
 * Set the child step of this loop. The child is activated on every activation
 * of the loop. When passing NULL the child is cleared and no step will be
 * activated on each iteration.
 */
void
psy_loop_set_child(PsyLoop *self, PsyStep *child)
{
    g_return_if_fail(PSY_IS_LOOP(self));
    g_return_if_fail(child == NULL || PSY_IS_STEP(child));

    PsyLoopPrivate *priv = psy_loop_get_instance_private(self);
    g_clear_object(&priv->child);
    priv->child = child;
    if (priv->child)
        psy_step_set_parent(priv->child, PSY_STEP(self));
}

/**
 * psy_loop_get_child:
 * @self: the loop whose child you'd like
 *
 * Get the child of this loop.
 *
 * Returns:(nullable) (transfer none): the child of this loop.
 */
PsyStep *
psy_loop_get_child(PsyLoop *self)
{
    g_return_val_if_fail(PSY_IS_LOOP(self), NULL);

    PsyLoopPrivate *priv = psy_loop_get_instance_private(self);

    return priv->child;
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
