
#include "ddd-stepping-stones.h"

typedef struct _DddSteppingStonesPrivate {
    GPtrArray  *steps;
    GHashTable *step_table;
    guint       next;
} DddSteppingStonesPrivate;

G_DEFINE_QUARK(ddd-stepping-stones-error-quark, ddd_stepping_stones_error)

G_DEFINE_TYPE_WITH_PRIVATE(DddSteppingStones, ddd_stepping_stones, DDD_TYPE_STEP)

//typedef enum {
//    NUM_SIGNALS
//} DddSteppingStonesSignal;

typedef enum {
    PROP_NULL,
    PROP_NUM_STEPS,
    NUM_PROPERTIES
} DddSteppingStonesProperty;

static GParamSpec *obj_properties[NUM_PROPERTIES] = {NULL};
//static guint       signals[NUM_SIGNALS]           = {0};

static void
ddd_stepping_stones_set_property (GObject      *object,
                                  guint         property_id,
                                  const GValue *value,
                                  GParamSpec   *pspec
                                  )
{
    DddSteppingStones *self = DDD_STEPPING_STONES(object);
    (void) self;
    (void) value;

    switch ((DddSteppingStonesProperty) property_id)
    {
        default:
            /* We don't have any other property... */
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
            break;
    }
}

static void
ddd_stepping_stones_get_property (GObject    *object,
                                  guint       property_id,
                                  GValue     *value,
                                  GParamSpec *pspec
                                  )
{
    DddSteppingStones *self = DDD_STEPPING_STONES(object);
    DddSteppingStonesPrivate *priv = ddd_stepping_stones_get_instance_private(self);

    switch ((DddSteppingStonesProperty) property_id)
    {
        case PROP_NUM_STEPS:
            g_value_set_uint(value, priv->steps->len);
            break;
        default:
            /* We don't have any other property... */
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
            break;
    }
}

static void
ddd_stepping_stones_dispose (GObject *gobject)
{
    DddSteppingStonesPrivate *priv = ddd_stepping_stones_get_instance_private(
            DDD_STEPPING_STONES(gobject)
            );

    if (priv->steps) {
        g_ptr_array_unref(priv->steps);
        priv->steps = NULL;
    }
    if (priv->step_table) {
        g_hash_table_unref(priv->step_table);
        priv->step_table = NULL;
    }
    G_OBJECT_CLASS (ddd_stepping_stones_parent_class)->dispose (gobject);
}

static void
ddd_stepping_stones_finalize (GObject *gobject)
{
    DddSteppingStonesPrivate *priv = ddd_stepping_stones_get_instance_private (
            DDD_STEPPING_STONES(gobject)
            );

    (void) priv;

    G_OBJECT_CLASS (ddd_stepping_stones_parent_class)->finalize (gobject);
}

static void
ddd_stepping_stones_init(DddSteppingStones* self)
{
    DddSteppingStonesPrivate *priv = ddd_stepping_stones_get_instance_private(
            self
            );
    priv->steps = g_ptr_array_new_full(
            64,
            g_object_unref
            );
    priv->step_table = g_hash_table_new_full(
            g_str_hash,
            g_str_equal,
            g_free,
            g_object_unref
            );
}

static void
stepping_stones_activate(DddStep* step, gint64 timestamp)
{
    DddSteppingStones *self = DDD_STEPPING_STONES(step);
    DddSteppingStonesPrivate* priv = ddd_stepping_stones_get_instance_private(
            self
            );

    if (priv->next >= ddd_stepping_stones_get_num_steps(self)) {
        ddd_step_leave(DDD_STEP(self), timestamp);
    }
    else {
        ddd_step_enter(priv->steps->pdata[priv->next], timestamp);
        priv->next++;
    }
}

static void
ddd_stepping_stones_class_init(DddSteppingStonesClass* klass)
{
    GObjectClass *obj_class = G_OBJECT_CLASS(klass);

    obj_class->set_property = ddd_stepping_stones_set_property;
    obj_class->get_property = ddd_stepping_stones_get_property;
    obj_class->dispose      = ddd_stepping_stones_dispose;
    obj_class->finalize     = ddd_stepping_stones_finalize;

    DddStepClass *step_klass = DDD_STEP_CLASS(klass);
    step_klass->activate     = stepping_stones_activate;

    obj_properties[PROP_NUM_STEPS] = g_param_spec_uint(
            "num-steps",
            "NumSteps",
            "The number of steps in the stepping stones object",
            0,
            G_MAXUINT,
            0,
            G_PARAM_READABLE
            );

    g_object_class_install_properties(
            obj_class,
            NUM_PROPERTIES,
            obj_properties
            );

}

/* ************** public functions *********************/

DddSteppingStones*
ddd_stepping_stones_new()
{
    return g_object_new(DDD_TYPE_STEPPING_STONES, NULL);
}

void
ddd_stepping_stones_destroy(DddSteppingStones* self)
{
    g_return_if_fail(DDD_IS_STEPPING_STONES(self));
    g_object_unref(self);
}

void
ddd_stepping_stones_add_step(DddSteppingStones* self, DddStep* step)
{
    g_return_if_fail(DDD_IS_STEPPING_STONES(self));
    g_return_if_fail(DDD_IS_STEP(step));
    g_return_if_fail(ddd_step_get_parent(step) == NULL ||
                     ddd_step_get_parent(step) == DDD_STEP(self));

    DddSteppingStonesPrivate *priv = ddd_stepping_stones_get_instance_private(self);
    g_ptr_array_add(priv->steps, g_object_ref(step));
    ddd_step_set_parent(step, DDD_STEP(self));
}

void
ddd_stepping_stones_add_step_by_name(DddSteppingStones *self,
                                     const gchar       *name,
                                     DddStep           *step,
                                     GError           **error
                                     )
{
    g_return_if_fail(DDD_IS_STEPPING_STONES(self));
    g_return_if_fail(name != NULL);
    g_return_if_fail(DDD_IS_STEP(step));
    g_return_if_fail(error == NULL || *error == NULL);

    DddSteppingStonesPrivate *priv = ddd_stepping_stones_get_instance_private(self);

    if (g_hash_table_contains(priv->step_table, name)) {
        g_set_error(error,
                    DDD_STEPPING_STONES_ERROR,
                    DDD_STEPPING_STONES_ERROR_KEY_EXISTS,
                    "The name '%s', already exists",
                    name
                    );
        return;
    }

    g_hash_table_add(priv->step_table, g_strdup(name));
    ddd_stepping_stones_add_step(self, step);
}

void
ddd_stepping_stones_activate_next_by_index(DddSteppingStones  *self,
                                           guint               index,
                                           GError            **error)
{
    g_return_if_fail(DDD_IS_STEPPING_STONES(self));
    g_return_if_fail(error == NULL || *error == NULL);

    DddSteppingStonesPrivate *priv = ddd_stepping_stones_get_instance_private(self);

    if (index >= priv->steps->len) {
        g_set_error(error,
                    DDD_STEPPING_STONES_ERROR,
                    DDD_STEPPING_STONES_ERROR_INVALID_INDEX,
                    "The index %d is larger than the number of steps (%d)",
                    index,
                    priv->steps->len
                    );
        return;
    }
    priv->next = index;
}

/**
 *
 * @param self
 * @param name
 * @param error
 */
void
ddd_stepping_stones_activate_next_by_name(DddSteppingStones *self,
                                          const gchar       *name,
                                          GError           **error
                                          )
{
    g_return_if_fail(DDD_IS_STEPPING_STONES(self));
    g_return_if_fail(name == NULL);
    g_return_if_fail(error == NULL || *error == NULL);

    DddSteppingStonesPrivate *priv = ddd_stepping_stones_get_instance_private(self);

    DddStep* step = g_hash_table_lookup(priv->step_table, name);
    if (!step) {
        g_set_error(error,
                    DDD_STEPPING_STONES_ERROR,
                    DDD_STEPPING_STONES_ERROR_NO_SUCH_KEY,
                    "There is no step with the name: %s",
                    name
                    );
    }
    guint index;
    gboolean found = g_ptr_array_find(priv->steps, step, &index);
    if (!found) {
        g_assert_not_reached();
    }

    ddd_stepping_stones_activate_next_by_index(self, index, error);
}

/**
 * ddd_stepping_stones_get_num_steps:
 * @self::The instance self
 *
 * @returns::The number of steps in the stepping stones.
 */
guint
ddd_stepping_stones_get_num_steps(DddSteppingStones* self)
{
    g_return_val_if_fail(DDD_IS_STEPPING_STONES(self), 0);
    DddSteppingStonesPrivate *priv = ddd_stepping_stones_get_instance_private(self);

    return priv->steps->len;
}
