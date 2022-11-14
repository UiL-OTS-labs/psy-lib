
#include "psy-vector.h"
#include <math.h>
#ifndef NAN
#warning "PsyVector requires math.h to export NAN"
#else

G_DEFINE_TYPE(PsyVector, psy_vector, G_TYPE_OBJECT)

typedef enum {
    PROP_0,
    PROP_X,
    PROP_Y,
    PROP_Z,
    PROP_LENGTH,

    N_PROPERTIES
}PsyVectorProperty;

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

static void psy_vector_init(PsyVector* self)
{
    (void) self;
}

static void
psy_vector_set_property(GObject      *object,
                        guint         prop_id,
                        const GValue *value,
                        GParamSpec   *pspec)
{
    PsyVector *vector = PSY_VECTOR(object);

    switch ((PsyVectorProperty)prop_id)
    {
        case PROP_X:
            vector->array[0] = g_value_get_float(value);
            break;
        case PROP_Y:
            vector->array[1] = g_value_get_float(value);
            break;
        case PROP_Z:
            vector->array[2] = g_value_get_float(value);
            break;
        case PROP_LENGTH: // get only
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
psy_vector_get_property(GObject      *object,
                        guint         prop_id,
                        GValue       *value,
                        GParamSpec   *pspec)
{
    PsyVector *vector = PSY_VECTOR(object);

    switch ((PsyVectorProperty)prop_id)
    {
        case PROP_X:
            g_value_set_float(value, vector->array[0]);
            break;
        case PROP_Y:
            g_value_set_float(value, vector->array[1]);
            break;
        case PROP_Z:
            g_value_set_float(value, vector->array[2]);
            break;
        case PROP_LENGTH:
            g_value_set_float(value, psy_vector_magnitude(vector));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
psy_vector_class_init(PsyVectorClass* klass)
{
    GObjectClass* gobject_class = G_OBJECT_CLASS(klass);

    gobject_class->set_property = psy_vector_set_property;
    gobject_class->get_property = psy_vector_get_property;

    obj_properties [PROP_X] = g_param_spec_float(
            "x",
            "X",
            "The x value of the vector.",
            -G_MAXFLOAT,
            G_MAXFLOAT,
            0.0,
            G_PARAM_READWRITE
            );

    obj_properties [PROP_Y] = g_param_spec_float(
            "y",
            "Y",
            "The y value of the vector.",
            -G_MAXFLOAT,
            G_MAXFLOAT,
            0.0,
            G_PARAM_READWRITE
    );

    obj_properties [PROP_Z] = g_param_spec_float(
            "z",
            "Z",
            "The z value of the vector.",
            -G_MAXFLOAT,
            G_MAXFLOAT,
            0.0,
            G_PARAM_READWRITE
    );

    obj_properties [PROP_LENGTH] = g_param_spec_float(
            "length",
            "Magnitude",
            "The length/magnitude of the vector",
            -G_MAXFLOAT,
            G_MAXFLOAT,
            0,
            G_PARAM_READABLE
    );

    g_object_class_install_properties(
            gobject_class,
            N_PROPERTIES,
            obj_properties
            );


}


/* ************* public functions ************* */

PsyVector*
psy_vector_new()
{
    return psy_vector_new_xyz(0,0,0);
}

PsyVector*
psy_vector_new_x(gfloat x)
{
    return psy_vector_new_xyz(x, 0 ,0);
}

PsyVector*
psy_vector_new_xy(gfloat x, gfloat y)
{
    return psy_vector_new_xyz(x, y, 0);
}

PsyVector*
psy_vector_new_xyz(gfloat x, gfloat y, gfloat z)
{
    PsyVector *v = g_object_new(PSY_TYPE_VECTOR, NULL);
    if (v) {
        v->array[0] = x;
        v->array[1] = y;
        v->array[2] = z;
    }
    return v;
}

void
psy_vector_destroy(PsyVector* self) {
    g_object_unref(self);
}

/**
 * psy_vector_magnitude:
 * @self: A PsyVector
 * 
 * Returns: the magnitude/length of @self
 */
gfloat
psy_vector_magnitude(PsyVector* self)
{
    g_return_val_if_fail(PSY_IS_VECTOR(self), NAN);
    gfloat sum = 0;
    for (size_t i = 0; i < (sizeof(self->array)/sizeof(self->array[0])); i++)
        sum += (self->array[i] * self->array[i]);
    return sqrtf(sum);
}

/**
 * psy_vector_unit:
 * @self: an instance of `PsyVector`
 *
 * Obtain a unitvector of @self; a vector with a mangitude of 1 in the
 * same direction of @self.
 *
 * Returns:(transfer full): a unit vector of @self
 */
PsyVector*
psy_vector_unit(PsyVector* self)
{
    g_return_val_if_fail(PSY_IS_VECTOR(self), NULL);
    gfloat mag = psy_vector_magnitude(self);

    if (mag == 0.0) // oops there is no unit vector of zero vector.
        return NULL;

    gfloat x = self->array[0] / mag;
    gfloat y = self->array[1] / mag;
    gfloat z = self->array[2] / mag;

    return psy_vector_new_xyz(x, y, z);
}

/**
 * psy_vector_negate:
 * @self: an instance of `PsyVector`
 *
 * Get a vector with the same magnitude but opposite direction of @self
 *
 * Returns:(transfer full): a negated version of @self
 */
PsyVector*
psy_vector_negate(PsyVector* self)
{
    g_return_val_if_fail(PSY_IS_VECTOR(self), NULL);
    gfloat x = -self->array[0];
    gfloat y = -self->array[1];
    gfloat z = -self->array[2];
    return psy_vector_new_xyz(x, y, z);
}

/**
 * psy_vector_add_s:
 * @self: an instance of `PsyVector`
 * @scalar: a scalar added to @self
 *
 * Adds a scalar to each element of @self
 * 
 * Returns:(transfer full): a vector that is the result of the vector scalar
 *                          addition
 */
PsyVector*
psy_vector_add_s(PsyVector* self, gfloat scalar)
{
    g_return_val_if_fail(PSY_IS_VECTOR(self), NULL);
    gfloat x, y, z;
    x = self->array[0] + scalar;
    y = self->array[1] + scalar;
    z = self->array[2] + scalar;
    return psy_vector_new_xyz(x, y, z);
}

/**
 * psy_vector_add:
 * @self: An instance of `PsyVector`
 * @other: An instance of `PsyVector` added to @self
 *
 * adds to vectors together:
 *
 * Returns:(transfer full): The result of @self + @other
 */
PsyVector*
psy_vector_add(PsyVector* self, PsyVector *other)
{
    g_return_val_if_fail(PSY_IS_VECTOR(self), NULL);
    g_return_val_if_fail(PSY_IS_VECTOR(other), NULL);
    gfloat x, y, z;
    x = self->array[0] + other->array[0];
    y = self->array[1] + other->array[1];
    z = self->array[2] + other->array[2];
    return psy_vector_new_xyz(x, y, z);
}

/**
 * psy_vector_sub_s:
 * @self: An instance of `PsyVector`
 * @scalar: An instance of `PsyVector` subtracted from @self
 *
 * Returns:(transfer full): The result of @self - other
 */
PsyVector*
psy_vector_sub_s(PsyVector* self, gfloat scalar)
{
    g_return_val_if_fail(PSY_IS_VECTOR(self), NULL);
    gfloat x, y, z;
    x = self->array[0] - scalar;
    y = self->array[1] - scalar;
    z = self->array[2] - scalar;
    return psy_vector_new_xyz(x, y, z);
}

/**
 * psy_vector_sub:
 * @self: An instance of `PsyVector`
 * @other: An instance of `PsyVector` subtracted from @self
 *
 * Returns:(transfer full): The result of @self - other
 */
PsyVector*
psy_vector_sub(PsyVector* self, PsyVector *other)
{
    g_return_val_if_fail(PSY_IS_VECTOR(self), NULL);
    g_return_val_if_fail(PSY_IS_VECTOR(other), NULL);
    gfloat x, y, z;
    x = self->array[0] - other->array[0];
    y = self->array[1] - other->array[1];
    z = self->array[2] - other->array[2];
    return psy_vector_new_xyz(x, y, z);
}

/**
 * psy_vector_mul_s:
 * @self: An instance of `PsyVector`
 * @scalar: A scaling factor for @self
 *
 * Scale @self by @scalar this maintains the direction of @self, but
 * changes the sign or magnitude.
 *
 * Returns:(transfer full): The result of @self * @scalar 
 */
PsyVector*
psy_vector_mul_s(PsyVector* self, gfloat scalar)
{
    g_return_val_if_fail(PSY_IS_VECTOR(self), NULL);
    gfloat x, y, z;
    x = self->array[0] * scalar;
    y = self->array[1] * scalar;
    z = self->array[2] * scalar;
    return psy_vector_new_xyz(x, y, z);
}

/**
 * psy_vector_dot:
 * @self: An instance of `PsyVector`
 * @other: A scaling factor for @self
 *
 * Returns the dotproduct between @self and @other
 *
 * Returns: The result of @self . @scalar 
 */
gfloat
psy_vector_dot(PsyVector* self, PsyVector* other)
{
    g_return_val_if_fail(PSY_IS_VECTOR(self), 0);
    g_return_val_if_fail(PSY_IS_VECTOR(other), 0);
    gfloat l1, l2;
    l1 = psy_vector_magnitude(self);
    l2 = psy_vector_magnitude(other);
    return (self->array[0] / l1) * (other->array[0] / l2)+
           (self->array[1] / l1) * (other->array[1] / l2)+
           (self->array[2] / l1) * (other->array[2] / l2);
}

/**
 * psy_vector_cross:
 * @self: An instance of `PsyVector`
 * @other: An instance of `PsyVector`
 *
 * returns the crosproduct a vector perpendicular to both @self and @other
 *
 * Returns:(transfer full): @self X @other.
 */
PsyVector*
psy_vector_cross(PsyVector* self, PsyVector* other)
{
    g_return_val_if_fail(PSY_IS_VECTOR(self), NULL);
    g_return_val_if_fail(PSY_IS_VECTOR(other), NULL);
    gfloat x, y, z;
    x = self->array[1]  * other->array[2] - self->array[2] * other->array[1];
    y = self->array[2]  * other->array[0] - self->array[0] * other->array[2];
    z = self->array[0]  * other->array[1] - self->array[1] * other->array[0];
    return psy_vector_new_xyz(x, y, z);
}

/**
 * psy_vector_equals:
 * @self: an instance of `PsyVector`
 * @other: an instance of `PsyVector`
 *
 * Returns: #true when @self == @other
 */
gboolean
psy_vector_equals(PsyVector* self, PsyVector* other)
{
    g_return_val_if_fail(PSY_IS_VECTOR(self), FALSE);
    g_return_val_if_fail(PSY_IS_VECTOR(other), FALSE);

    if (self == other)
        return TRUE;

    return self->array[0] == other->array[0] &&
           self->array[1] == other->array[1] &&
           self->array[2] == other->array[2];
}

/**
 * psy_vector_not_equals:
 * @self: an instance of `PsyVector`
 * @other: an instance of `PsyVector`
 *
 * Returns: #true when @self != @other
 */
gboolean
psy_vector_not_equals(PsyVector* self, PsyVector* other)
{
    return !psy_vector_equals(self, other);
}

#endif
