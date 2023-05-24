
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <string>

#include "psy-matrix4-private.h"
#include "psy-vector4.h"

typedef struct _PsyVector4 {
    GObject    obj;
    glm::vec4 *vector;
} PsyVector4;

G_DEFINE_TYPE(PsyVector4, psy_vector4, G_TYPE_OBJECT)

typedef enum {
    PROP_NULL,
    PROP_X,
    PROP_Y,
    PROP_Z,
    PROP_W,
    PROP_MAGNITUDE,
    PROP_UNIT,
    //    PROP_VALUES,
    N_PROPERTIES
} PsyVector4Property;

static GParamSpec *obj_properties[N_PROPERTIES] = {
    NULL,
};

static void
psy_vector4_init(PsyVector4 *self)
{
    self->vector = new glm::vec4();
}

static void
psy_vector4_finalize(GObject *obj)
{
    PsyVector4 *self = PSY_VECTOR4(obj);

    delete self->vector;

    G_OBJECT_CLASS(psy_vector4_parent_class)->finalize(obj);
}

static void
psy_vector4_set_property(GObject      *object,
                         guint         prop_id,
                         const GValue *value,
                         GParamSpec   *pspec)
{
    PsyVector4 *self = PSY_VECTOR4(object);

    switch ((PsyVector4Property) prop_id) {
    case PROP_X:
        (*self->vector)[0] = g_value_get_float(value);
        break;
    case PROP_Y:
        (*self->vector)[1] = g_value_get_float(value);
        break;
    case PROP_Z:
        (*self->vector)[2] = g_value_get_float(value);
        break;
    case PROP_W:
        (*self->vector)[3] = g_value_get_float(value);
        break;
        //        case PROP_VALUES:
        //            g_assert(G_VALUE_HOLDS_BOXED(value));
        //            psy_vector4_set_values(self,
        //            (GArray*)g_value_get_boxed(value)); break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void
psy_vector4_get_property(GObject    *object,
                         guint       prop_id,
                         GValue     *value,
                         GParamSpec *pspec)
{
    PsyVector4 *self = PSY_VECTOR4(object);

    switch ((PsyVector4Property) prop_id) {
    case PROP_X:
        g_value_set_float(value, (*self->vector)[0]);
        break;
    case PROP_Y:
        g_value_set_float(value, (*self->vector)[1]);
        break;
    case PROP_Z:
        g_value_set_float(value, (*self->vector)[2]);
        break;
    case PROP_W:
        g_value_set_float(value, (*self->vector)[3]);
        break;
    case PROP_MAGNITUDE:
        g_value_set_float(value, psy_vector4_get_magnitude(self));
        break;
    case PROP_UNIT:
        g_value_take_object(value, psy_vector4_unit(self));
        break;
        //        case PROP_VALUES:
        //            g_value_take_boxed(value, psy_vector4_get_values(self));
        //            break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void
psy_vector4_class_init(PsyVector4Class *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
    (void) gobject_class;

    gobject_class->set_property = psy_vector4_set_property;
    gobject_class->get_property = psy_vector4_get_property;
    gobject_class->finalize     = psy_vector4_finalize;

    obj_properties[PROP_X] = g_param_spec_float("x",
                                                "X",
                                                "The x value of the vector",
                                                -G_MAXFLOAT,
                                                G_MAXFLOAT,
                                                0,
                                                G_PARAM_READWRITE);

    obj_properties[PROP_Y] = g_param_spec_float("y",
                                                "Y",
                                                "The y value of the vector",
                                                -G_MAXFLOAT,
                                                G_MAXFLOAT,
                                                0,
                                                G_PARAM_READWRITE);

    obj_properties[PROP_Z] = g_param_spec_float("z",
                                                "Z",
                                                "The z value of the vector",
                                                -G_MAXFLOAT,
                                                G_MAXFLOAT,
                                                0,
                                                G_PARAM_READWRITE);

    obj_properties[PROP_W] = g_param_spec_float("w",
                                                "W",
                                                "The w value of the vector",
                                                -G_MAXFLOAT,
                                                G_MAXFLOAT,
                                                0,
                                                G_PARAM_READWRITE);

    obj_properties[PROP_MAGNITUDE]
        = g_param_spec_float("magnitude",
                             "magnitude",
                             "The magnitude of the vector.",
                             0,
                             G_MAXFLOAT,
                             0,
                             G_PARAM_READABLE);

    obj_properties[PROP_UNIT]
        = g_param_spec_object("unit",
                              "unit-vector",
                              "The unit vector in the same direction as self",
                              PSY_TYPE_VECTOR4,
                              G_PARAM_READABLE);
    // TODO figure out whether a values property is possible
    // this works for C/C++ but not for python
    // the property seems to contain garbage or segfault occurs

    //    /**
    //     * PsyVector4:values:(skip):
    //     *
    //     * This property represents the values of the 4 dimensions of an
    //     * `PsyVector4` instance
    //     */
    //    obj_properties[PROP_VALUES] = g_param_spec_boxed(
    //            "values",
    //            "Values",
    //            "Initialize the vector with 4 floating point values",
    //            G_TYPE_ARRAY,
    //            G_PARAM_READWRITE
    //            );

    g_object_class_install_properties(
        gobject_class, N_PROPERTIES, obj_properties);
}

/* ************* public functions ************* */

PsyVector4 *
psy_vector4_new()
{
    return PSY_VECTOR4(g_object_new(PSY_TYPE_VECTOR4, NULL));
}

/**
 * psy_vector4_new_data:
 * @n:      length of @values n should be 0 < n <= 4
 * @values: (array length=n): list of values
 *
 * Set a new vector from data points
 *
 * Returns::A vector initialized with values, if @n < 4, the remaining elements
 *          will be set to 0.0, if n > 0 the values after 4 will be ignored.
 */
PsyVector4 *
psy_vector4_new_data(gsize n, gfloat *values)
{
    PsyVector4 *ret = psy_vector4_new();
    g_warn_if_fail(n <= 4);

    gint min = glm::min((int) n, ret->vector->length());

    for (auto i = 0; i < min; i++)
        (*ret->vector)[i] = values[i];

    return ret;
}

/**
 * psy_vector4_destroy:
 * @self: An instance to destroy
 */
void
psy_vector4_destroy(PsyVector4 *self)
{
    g_object_unref(self);
}

/**
 * psy_vector4_set_null:
 * @self: an instance of `PsyVector4`
 *
 * Sets all elements to zero turning @self into a null vector.
 */
void
psy_vector4_set_null(PsyVector4 *self)
{
    g_return_if_fail(PSY_IS_VECTOR4(self));

    *self->vector = glm::vec4(0);
}

/**
 * psy_vector4_is_null:
 * @self: an instance of `PsyVector4`
 *
 * tests whether @self is a null vector
 *
 * Returns: #TRUE if @self is a null vector #FALSE otherwise.
 */
gboolean
psy_vector4_is_null(PsyVector4 *self)
{
    g_return_val_if_fail(PSY_IS_VECTOR4(self), FALSE);

    return *self->vector == glm::vec4(0);
}

/**
 * psy_vector4_get_magnitude:
 * @self: an instance of `PsyVector4`
 *
 * Obtain the magnitude/length of the vector
 *
 * Returns: the maginitude of the vector
 */

gfloat
psy_vector4_get_magnitude(PsyVector4 *self)
{
    g_return_val_if_fail(PSY_IS_VECTOR4(self), 0);

    return glm::length(*self->vector);
}

/**
 * psy_vector4_unit:
 * @self: an instance of `PsyVector4` and should not be a null vector
 *
 * Returns a normalized version of it self, a vector with the same direction,
 * but a magnitude of 1.0
 *
 * Returns: (transfer full): a vector with the same direction as @self, but
 * with a magnitude of 1.
 */
PsyVector4 *
psy_vector4_unit(PsyVector4 *self)
{
    g_return_val_if_fail(PSY_IS_VECTOR4(self), nullptr);

    PsyVector4 *ret = psy_vector4_new();
    g_return_val_if_fail(ret, nullptr);
    if (psy_vector4_is_null(self)) {
        g_object_unref(ret);
        return nullptr;
    }

    *ret->vector = glm::normalize(*self->vector);
    return ret;
}

/**
 * psy_vector4_negate:
 * @self: an instance of `PsyVec4`
 *
 * Returns:(transfer full): a vector with the opposite direction and magnitude
 * of
 *                          @self.
 */
PsyVector4 *
psy_vector4_negate(PsyVector4 *self)
{
    g_return_val_if_fail(PSY_IS_VECTOR4(self), nullptr);

    PsyVector4 *ret = psy_vector4_new();
    g_return_val_if_fail(ret, nullptr);

    *ret->vector = -(*self->vector);
    return ret;
}

/**
 * psy_vector4_add_s:
 * @self: an instance of `PsyVec4`
 * @scalar: an value to add to each element of @self
 *
 * Returns:(transfer full): a vector with @scalar.
 */
PsyVector4 *
psy_vector4_add_s(PsyVector4 *self, gfloat scalar)
{
    g_return_val_if_fail(PSY_IS_VECTOR4(self), NULL);

    PsyVector4 *ret = PSY_VECTOR4(psy_vector4_new());

    *ret->vector = (*self->vector) + scalar;

    return ret;
}

/**
 * psy_vector4_add:
 * @self: an instance of `PsyVec4`
 * @other: an value to add to each element of @self
 *
 * Returns:(transfer full): the result of @self + @other
 */
PsyVector4 *
psy_vector4_add(PsyVector4 *self, PsyVector4 *other)
{
    g_return_val_if_fail(PSY_IS_VECTOR4(self), NULL);
    g_return_val_if_fail(PSY_IS_VECTOR4(other), NULL);

    PsyVector4 *ret = PSY_VECTOR4(psy_vector4_new());

    *ret->vector = (*self->vector) + (*other->vector);

    return ret;
}

/**
 * psy_vector4_sub_s:
 * @self: an instance of `PsyVec4`
 * @scalar: an value to subtract from each element of @self
 *
 * Returns:(transfer full): the result of @self - @scalar
 */
PsyVector4 *
psy_vector4_sub_s(PsyVector4 *self, gfloat scalar)
{
    g_return_val_if_fail(PSY_IS_VECTOR4(self), NULL);
    PsyVector4 *ret = PSY_VECTOR4(psy_vector4_new());

    *ret->vector = (*self->vector) - scalar;

    return ret;
}

/**
 * psy_vector4_sub:
 * @self: an instance of `PsyVec4`
 * @other: an instance `PsyVector` to subtract from @self
 *
 * Returns:(transfer full): the result from @self - @other
 */
PsyVector4 *
psy_vector4_sub(PsyVector4 *self, PsyVector4 *other)
{
    g_return_val_if_fail(PSY_IS_VECTOR4(self), NULL);
    g_return_val_if_fail(PSY_IS_VECTOR4(other), NULL);

    PsyVector4 *ret = PSY_VECTOR4(psy_vector4_new());

    *ret->vector = (*self->vector) - (*other->vector);
    return ret;
}

/**
 * psy_vector4_mul_s:
 * @self: A instance of `PsyVector4` to be scaled by @scalar
 * @scalar: The factor to scale @self with
 *
 * Returns:(transfer full): The result of @self * @scalar
 */
PsyVector4 *
psy_vector4_mul_s(PsyVector4 *self, gfloat scalar)
{
    g_return_val_if_fail(PSY_IS_VECTOR4(self), NULL);
    PsyVector4 *ret = PSY_VECTOR4(psy_vector4_new());

    *ret->vector = (*self->vector) * scalar;

    return ret;
}

/**
 * psy_vector4_mul:
 * @self: A instance of `PsyVector4` to be scaled by @scalar
 * @other: The factor to scale @self with
 *
 * Get the dot product between @self and @other.
 *
 * Returns:(transfer full): The result of @self * @other
 */
PsyVector4 *
psy_vector4_mul(PsyVector4 *self, PsyVector4 *other)
{
    g_return_val_if_fail(PSY_IS_VECTOR4(self), NULL);
    g_return_val_if_fail(PSY_IS_VECTOR4(other), NULL);

    PsyVector4 *ret = PSY_VECTOR4(psy_vector4_new());

    *ret->vector = (*self->vector) * (*other->vector);
    return ret;
}

/**
 * psy_vector4_mul_matrix4:
 * @self: an instance of [class@Vector4]
 * @other: an instance of [class@Matrix4]
 *
 * performs vector matrix multiplication of a vector with length = 4 with a
 * matrix with a 4*4 size.
 *
 * Returns:(transfer full): The result of
 */
PsyVector4 *
psy_vector4_mul_matrix4(PsyVector4 *self, PsyMatrix4 *other)
{
    g_return_val_if_fail(PSY_IS_VECTOR4(self), NULL);
    g_return_val_if_fail(PSY_IS_MATRIX4(other), NULL);

    PsyVector4 *ret = PSY_VECTOR4(psy_vector4_new());

    *ret->vector = (*self->vector) * (psy_matrix4_get_priv_reference(other));
    return ret;
}

/**
 * psy_vector4_dot:
 * @self: A instance of `PsyVector4` to be scaled by @scalar
 * @other: The factor to scale @self with
 *
 * Get the dot product between @self and @other.
 *
 * Returns: The result of @self . @other
 */
gfloat
psy_vector4_dot(PsyVector4 *self, PsyVector4 *other)
{
    g_return_val_if_fail(PSY_IS_VECTOR4(self), 0.0);
    g_return_val_if_fail(PSY_IS_VECTOR4(other), 0.0);

    return glm::dot(*self->vector, *other->vector);
}

/**
 * psy_vector4_equals:
 * @self: An instance to compare with @other
 * @other: An instance to compare with @self
 *
 * Compares whether all vector elements are equal
 *
 * Returns: the result of @self == @other
 */
gboolean
psy_vector4_equals(PsyVector4 *self, PsyVector4 *other)
{
    g_return_val_if_fail(PSY_IS_VECTOR4(self), FALSE);
    g_return_val_if_fail(PSY_IS_VECTOR4(other), FALSE);

    if (self == other)
        return TRUE;

    return *self->vector == *other->vector;
}

/**
 * psy_vector4_not_equals:
 * @self: An instance to compare with @other
 * @other: An instance to compare with @self
 *
 * Compares whether not all vector equal
 *
 * Returns: the result of @self != @other
 */
gboolean
psy_vector4_not_equals(PsyVector4 *self, PsyVector4 *other)
{
    return !psy_vector4_equals(self, other);
}

/**
 * psy_vector4_ptr:(skip)
 * @self: an instance of `PsyVector4`
 *
 * This function is used internally to get to the data efficiently.
 *
 * Returns: a pointer to the data
 */
const gfloat *
psy_vector4_ptr(PsyVector4 *self)
{
    g_return_val_if_fail(PSY_IS_VECTOR4(self), NULL);

    return glm::value_ptr(*self->vector);
}

/**
 * psy_vector4_get_values:
 * @self: An instance of `PsyVector` whose elements you'd like to get
 *
 * Returns:(transfer full)(element-type gfloat): the elements of the vector
 */
GArray *
psy_vector4_get_values(PsyVector4 *self)
{
    g_return_val_if_fail(PSY_IS_VECTOR4(self), NULL);

    GArray *ret = g_array_sized_new(FALSE, FALSE, sizeof(gfloat), 4);
    g_array_append_vals(ret, psy_vector4_ptr(self), 4);

    return ret;
}

/**
 * psy_vector4_set_values:
 * @self: an instance of `PsyVector4` whose values to set
 * @array:(transfer none)(element-type gfloat): an
 *        array with 4 value to set this array with
 *
 * Set the values of @self
 */
void
psy_vector4_set_values(PsyVector4 *self, GArray *array)
{
    g_return_if_fail(PSY_IS_VECTOR4(self));
    g_return_if_fail(array);

    gfloat *in  = reinterpret_cast<gfloat *>(array->data);
    guint   min = array->len > 4 ? 4 : array->len;

    for (guint i = 0; i < min; i++)
        (*self->vector)[i] = in[i];
}

/**
 * psy_vector4_as_string:
 * @self: an instance of [class@Vector4]
 *
 * Returns: (transfer full): a string representation of @self
 */
gchar *
psy_vector4_as_string(PsyVector4 *self)
{
    g_return_val_if_fail(PSY_IS_VECTOR4(self), NULL);

    std::string output = glm::to_string(*self->vector);
    return g_strdup(output.c_str());
}

/*
 * private functions
 *
 * see psy-vector4-private.h
 */

/**
 * psy_vector4_get_priv_reference:(skip)
 *
 * Returns: the private implementation of the vector a glm::vec4
 */
glm::vec4&
psy_vector4_get_priv_reference(PsyVector4 *self)
{
    g_assert(PSY_IS_VECTOR4(self));

    return *self->vector;
}

/**
 * psy_vector4_get_priv_pointer:(skip)
 */
glm::vec4 *
psy_vector4_get_priv_pointer(PsyVector4 *self)
{
    g_assert(PSY_IS_VECTOR4(self));

    return self->vector;
}
