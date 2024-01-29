
#include "psy-stepping-stones.h"

/**
 * PsySteppingStones:
 *
 * The `PsySteppingStones` object is a `PsyStep` instance that may
 * be used to contain substeps. The contained steps may be processed in
 * order, by index or by the name of the steps.
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
psy_stepping_stones_set_property(GObject      *object,
                                 guint         property_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
    PsySteppingStones *self = PSY_STEPPING_STONES(object);
    (void) self;
    (void) value;

    switch ((PsySteppingStonesProperty) property_id) {
    default:
        /* We don't have any other property... */
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

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
psy_stepping_stones_finalize(GObject *gobject)
{
    PsySteppingStonesPrivate *priv = psy_stepping_stones_get_instance_private(
        PSY_STEPPING_STONES(gobject));

    if (priv->steps) {
        g_ptr_array_unref(priv->steps);
        priv->steps = NULL;
    }
    if (priv->step_table) {
        g_hash_table_destroy(priv->step_table);
        priv->step_table = NULL;
    }

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

    obj_class->set_property = psy_stepping_stones_set_property;
    obj_class->get_property = psy_stepping_stones_get_property;
    obj_class->finalize     = psy_stepping_stones_finalize;

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
 * @step: The `PsyStep` to add to @self
 *
 * To a `PsySteppingStones` multiple steps/stones may be added when
 * added like this, this allows to step through them in the order
 * in which the are added.
 */
void
psy_stepping_stones_add_step(PsySteppingStones *self, PsyStep *step)
{
    g_return_if_fail(PSY_IS_STEPPING_STONES(self));
    g_return_if_fail(PSY_IS_STEP(step));
    g_return_if_fail(psy_step_get_parent(step) == NULL
                     || psy_step_get_parent(step) == PSY_STEP(self));

    PsySteppingStonesPrivate *priv
        = psy_stepping_stones_get_instance_private(self);
    g_ptr_array_add(priv->steps, g_object_ref(step));
    psy_step_set_parent(step, PSY_STEP(self));
}

/**
 * psy_stepping_stones_add_step_by_name:
 * @self: The instance self
 * @name: The name for the step to add this name may be used to jump back
 * or skip subsequent steps. The name should be unique.
 * @step: The step to add
 * @error: optional errors may be returned here.
 */
void
psy_stepping_stones_add_step_by_name(PsySteppingStones *self,
                                     const gchar       *name,
                                     PsyStep           *step,
                                     GError           **error)
{
    g_return_if_fail(PSY_IS_STEPPING_STONES(self));
    g_return_if_fail(name != NULL);
    g_return_if_fail(PSY_IS_STEP(step));
    g_return_if_fail(error == NULL || *error == NULL);

    PsySteppingStonesPrivate *priv
        = psy_stepping_stones_get_instance_private(self);

    if (g_hash_table_contains(priv->step_table, name)) {
        g_set_error(error,
                    PSY_STEPPING_STONES_ERROR,
                    PSY_STEPPING_STONES_ERROR_KEY_EXISTS,
                    "The name '%s', already exists",
                    name);
        return;
    }

    g_hash_table_add(priv->step_table, g_strdup(name));
    psy_stepping_stones_add_step(self, step);
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
    g_return_if_fail(name == NULL);
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
