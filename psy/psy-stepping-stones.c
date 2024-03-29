
#include "psy-stepping-stones.h"

/**
 * PsySteppingStones:
 *
 *
 * Instances of [class@SteppingStones] are designed to to structure your
 * flow of the experiment. You can add substeps using the
 * [method@SteppingStones.add_step], in this case the steps are added, the
 * first step will get index 0, then 1, etc. Additionally, you can add
 * step using [method@SteppingStones.add_step_by_name], in this case
 * you'll add a step, by index, but also you'll register a name for this
 * specific substep. This allows to jump for- or backward to this step
 * by calling the [method@SteppingStones.activate_next_by_name], another
 * way is calling [method@SteppingStones.activate_next_by_index]. However,
 * the one using the name is probably more flexible.
 *
 * An example experiment might look like:
 *
 * ```
 * SteppingStones:experiment
 *     |
 *     |-- Trial:practice-instruction
 *     |
 *     |-- loop:practice
 *         |
 *         |-- Trial:practice_trial
 *     |
 *     |-- SideStep: test whether participant has understood the instructions
 *     |
 *     |-- Trial: test-instruction
 *     |
 *     |-- loop:test
 *         |
 *         |-- Trial:test_trial
 *     |
 *     | -- Trial:thank-you instructions.
 * ```
 *
 * In the experiment above, a SteppingStones object is created, that contains
 * a number of sub parts:
 *
 * 1. Practice instructions
 * 1. Loop
 *     - A number of practice trials
 * 1. A sidestep where we check whether the participant has understood the
 *    instructions. If **yes**, we do nothing, the test will advance to the
 *    pretest instructions, the next part. If **no**, we call one of
 *    [method@SteppingStones.activate_next_by_index] or
 *    [method@SteppingStones.activate_next_by_name] on the Experiment stepping
 *    stones instance, so that when we leave the sidestep, we step back to
 *    the practice instruction
 * 1. Loop of test trials
 * 1. Thank the participant for participating.
 *
 * We could also split up the practice trial in a new stepping stones object
 * that exists of two parts:
 *
 * ```
 * SteppingStones:PracTrial:
 *    |
 *    | -- Trial:PresentStimuli
 *    | -- Trial:Feedback
 * ```
 *
 * This might be a nice setup to add an extra feedback to after a practice
 * trial and skip this in the actual experiment. So in order to do that
 * you would replace the `Trial:practice_trial` in the example above with the
 * `SteppingStones:PracTrial` instead.
 */

typedef struct _PsySteppingStonesPrivate {
    GPtrArray  *steps;
    GHashTable *step_table;
    guint       next;
} PsySteppingStonesPrivate;

// clang-format off
G_DEFINE_QUARK(psy-stepping-stones-error-quark, psy_stepping_stones_error)
// clang-format on

G_DEFINE_TYPE_WITH_PRIVATE(PsySteppingStones,
                           psy_stepping_stones,
                           PSY_TYPE_STEP)

// typedef enum {
//     NUM_SIGNALS
// } PsySteppingStonesSignal;

typedef enum {
    PROP_NULL,
    PROP_NUM_STEPS,
    NUM_PROPERTIES
} PsySteppingStonesProperty;

static GParamSpec *obj_properties[NUM_PROPERTIES] = {NULL};

// static guint       signals[NUM_SIGNALS]           = {0};

static void
psy_stepping_stones_get_property(GObject    *object,
                                 guint       property_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
    PsySteppingStones        *self = PSY_STEPPING_STONES(object);
    PsySteppingStonesPrivate *priv
        = psy_stepping_stones_get_instance_private(self);

    switch ((PsySteppingStonesProperty) property_id) {
    case PROP_NUM_STEPS:
        g_value_set_uint(value, priv->steps->len);
        break;
    default:
        /* We don't have any other property... */
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static void
psy_stepping_stones_dispose(GObject *gobject)
{
    PsySteppingStones        *self = PSY_STEPPING_STONES(gobject);
    PsySteppingStonesPrivate *priv
        = psy_stepping_stones_get_instance_private(self);

    if (priv->steps) {
        g_ptr_array_unref(priv->steps);
        priv->steps = NULL;
    }

    if (priv->step_table) {
        g_hash_table_destroy(priv->step_table);
        priv->step_table = NULL;
    }
    G_OBJECT_CLASS(psy_stepping_stones_parent_class)->dispose(gobject);
}

static void
psy_stepping_stones_finalize(GObject *gobject)
{
    PsySteppingStonesPrivate *priv = psy_stepping_stones_get_instance_private(
        PSY_STEPPING_STONES(gobject));

    (void) priv;

    G_OBJECT_CLASS(psy_stepping_stones_parent_class)->finalize(gobject);
}

static void
psy_stepping_stones_init(PsySteppingStones *self)
{
    PsySteppingStonesPrivate *priv
        = psy_stepping_stones_get_instance_private(self);

    priv->steps = g_ptr_array_new_full(64, g_object_unref);

    priv->step_table = g_hash_table_new_full(
        g_str_hash, g_str_equal, g_free, g_object_unref);
}

static void
stepping_stones_activate(PsyStep *step, PsyTimePoint *timestamp)
{
    PsySteppingStones        *self = PSY_STEPPING_STONES(step);
    PsySteppingStonesPrivate *priv
        = psy_stepping_stones_get_instance_private(self);

    if (priv->next >= psy_stepping_stones_get_num_steps(self)) {
        psy_step_leave(PSY_STEP(self), timestamp);
    }
    else {
        psy_step_enter(priv->steps->pdata[priv->next], timestamp);
        priv->next++;
    }
}

static void
psy_stepping_stones_class_init(PsySteppingStonesClass *klass)
{
    GObjectClass *obj_class = G_OBJECT_CLASS(klass);

    obj_class->get_property = psy_stepping_stones_get_property;
    obj_class->finalize     = psy_stepping_stones_finalize;
    obj_class->dispose      = psy_stepping_stones_dispose;

    PsyStepClass *step_klass = PSY_STEP_CLASS(klass);
    step_klass->activate     = stepping_stones_activate;

    obj_properties[PROP_NUM_STEPS]
        = g_param_spec_uint("num-steps",
                            "NumSteps",
                            "The number of steps in the stepping stones object",
                            0,
                            G_MAXUINT,
                            0,
                            G_PARAM_READABLE);

    g_object_class_install_properties(
        obj_class, NUM_PROPERTIES, obj_properties);
}

/* ************** public functions *********************/
/**
 * psy_stepping_stones_new:(constructor)
 *
 * Creates a new `PsySteppingStones` object
 *
 * Create a new instance of PsySteppingStones. You can use
 * psy_stepping_stones_free or g_object_unref on the instance in order to free
 * it. psy_stepping_stones_free does a type check before calling g_object_unref
 * on the object, so it is slightly more safe.
 *
 * Returns: Instance of [class@SteppingStones], may be freed with
 * [method@SteppingStones.free]
 */
PsySteppingStones *
psy_stepping_stones_new(void)
{
    return g_object_new(PSY_TYPE_STEPPING_STONES, NULL);
}

/**
 * psy_stepping_stones_free:(skip)
 *
 * Frees an instance of [class@SteppingStones] previously allocated with
 * [ctor@PsySteppingStones.new]
 */
void
psy_stepping_stones_free(PsySteppingStones *self)
{
    g_return_if_fail(PSY_IS_STEPPING_STONES(self));
    g_object_unref(self);
}

/**
 * psy_stepping_stones_add_step:
 * @self: The `PsySteppingStones` instance to which a `PsyStep` @step
 * @step:(transfer full): The `PsyStep` to add to @self The step should not
 *                        already have a parent.
 *
 * To an instance of [class@SteppingStones] multiple steps/stones may be added
 * when added like this, this allows to step through them in the order
 * in which the are added.
 * When adding an instance of [class@Step], @self, becomes its parent.
 * So you have to make sure that the @step instance doesn't already have a
 * parent.
 *
 * Returns: True when successfully adding the step as child, false otherwise.
 */
gboolean
psy_stepping_stones_add_step(PsySteppingStones *self, PsyStep *step)
{
    g_return_val_if_fail(PSY_IS_STEPPING_STONES(self), FALSE);
    g_return_val_if_fail(PSY_IS_STEP(step), FALSE);
    g_return_val_if_fail(psy_step_get_parent(step) == NULL
                             || psy_step_get_parent(step) == PSY_STEP(self),
                         FALSE);

    PsySteppingStonesPrivate *priv
        = psy_stepping_stones_get_instance_private(self);
    g_ptr_array_add(priv->steps, g_object_ref(step));
    psy_step_set_parent(step, PSY_STEP(self));
    return TRUE;
}

/**
 * psy_stepping_stones_add_step_by_name:
 * @self: The instance self
 * @name: The name for the step to add this name may be used to jump back
 *        or skip subsequent steps. The name should be unique.
 * @step:(transfer full): The step to add
 * @error: optional errors may be returned here.
 *
 * Using this function you can add steps with a name that identifies one
 * specific step. You can select this step for the next activation of
 * the SteppingStones instance when using
 * [method@SteppingStones.activate_next_by_name], then the next time @self
 * is activated @step will be entered, hence activated too.
 * Every name, for this step must be unique, other wise an error will
 * be returned.
 *
 * Returns: TRUE when the step was successfully added as child, false otherwise.
 */
gboolean
psy_stepping_stones_add_step_by_name(PsySteppingStones *self,
                                     const gchar       *name,
                                     PsyStep           *step,
                                     GError           **error)
{
    g_return_val_if_fail(PSY_IS_STEPPING_STONES(self), FALSE);
    g_return_val_if_fail(name != NULL, FALSE);
    g_return_val_if_fail(PSY_IS_STEP(step), FALSE);
    g_return_val_if_fail(error == NULL || *error == NULL, FALSE);
    g_return_val_if_fail(psy_step_get_parent(step) == NULL, FALSE);

    g_debug("Adding step %p under name %s", (void *) step, name);

    PsySteppingStonesPrivate *priv
        = psy_stepping_stones_get_instance_private(self);

    if (g_hash_table_contains(priv->step_table, name)) {
        g_set_error(error,
                    PSY_STEPPING_STONES_ERROR,
                    PSY_STEPPING_STONES_ERROR_KEY_EXISTS,
                    "The name '%s', already exists",
                    name);
        return FALSE;
    }

    g_hash_table_insert(priv->step_table, g_strdup(name), step);
    psy_stepping_stones_add_step(self, step);
    return TRUE;
}

/**
 * psy_stepping_stones_activate_next_by_index:
 * @self:
 * @index: The index of the next step to be activated. It should be
 *         less than the number of steps added to this step.
 * @error:(out): An optional error may be returned here.
 */
void
psy_stepping_stones_activate_next_by_index(PsySteppingStones *self,
                                           guint              index,
                                           GError           **error)
{
    g_return_if_fail(PSY_IS_STEPPING_STONES(self));
    g_return_if_fail(error == NULL || *error == NULL);

    PsySteppingStonesPrivate *priv
        = psy_stepping_stones_get_instance_private(self);

    if (index >= priv->steps->len) {
        g_set_error(error,
                    PSY_STEPPING_STONES_ERROR,
                    PSY_STEPPING_STONES_ERROR_INVALID_INDEX,
                    "The index %d is larger than the number of steps (%d)",
                    index,
                    priv->steps->len);
        return;
    }
    priv->next = index;
}

/**
 * psy_stepping_stones_activate_next_by_name:
 * @self: an `PsySteppingStones` instance
 * @name: A name of another other `PsyStep` in PsySteppingStones:steps
 * @error: An error may be returned here
 *
 * Allows for activating the next step.
 */
void
psy_stepping_stones_activate_next_by_name(PsySteppingStones *self,
                                          const gchar       *name,
                                          GError           **error)
{
    g_return_if_fail(PSY_IS_STEPPING_STONES(self));
    g_return_if_fail(name != NULL);
    g_return_if_fail(error == NULL || *error == NULL);

    PsySteppingStonesPrivate *priv
        = psy_stepping_stones_get_instance_private(self);

    PsyStep *step = g_hash_table_lookup(priv->step_table, name);
    if (!step) {
        g_set_error(error,
                    PSY_STEPPING_STONES_ERROR,
                    PSY_STEPPING_STONES_ERROR_NO_SUCH_KEY,
                    "There is no step with the name: %s",
                    name);
        return;
    }

    guint    index;
    gboolean found = g_ptr_array_find(priv->steps, step, &index);
    if (!found) {
        g_assert_not_reached();
    }

    psy_stepping_stones_activate_next_by_index(self, index, error);
}

/**
 * psy_stepping_stones_get_num_steps:
 * @self: An `PsySteppingStones` instance self
 *
 * Returns::The number of steps in the stepping stones.
 */
guint
psy_stepping_stones_get_num_steps(PsySteppingStones *self)
{
    g_return_val_if_fail(PSY_IS_STEPPING_STONES(self), 0);
    PsySteppingStonesPrivate *priv
        = psy_stepping_stones_get_instance_private(self);

    return priv->steps->len;
}
