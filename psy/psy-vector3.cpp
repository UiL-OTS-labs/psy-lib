
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <string>

#include "psy-vector3.h"

typedef struct _PsyVector3 {
    GObject     obj;
    glm::dvec3 *vector;
} PsyVector3;

G_DEFINE_TYPE(PsyVector3, psy_vector3, G_TYPE_OBJECT)

typedef enum {
    PROP_NULL,
    PROP_X,
    PROP_Y,
    PROP_Z,
    PROP_MAGNITUDE,
    PROP_UNIT,
    //    PROP_VALUES,
    N_PROPERTIES
} PsyVector3Property;

static GParamSpec *obj_properties[N_PROPERTIES] = {
    NULL,
};

static void
psy_vector3_init(PsyVector3 *self)
{
    self->vector = new glm::dvec3();
}

static void
psy_vector3_finalize(GObject *obj)
{
    PsyVector3 *self = PSY_VECTOR3(obj);

    delete self->vector;

    G_OBJECT_CLASS(psy_vector3_parent_class)->finalize(obj);
}

static void
psy_vector3_set_property(GObject      *object,
                         guint         prop_id,
                         const GValue *value,
                         GParamSpec   *pspec)
{
    PsyVector3 *self = PSY_VECTOR3(object);

    switch ((PsyVector3Property) prop_id) {
    case PROP_X:
        (*self->vector)[0] = g_value_get_double(value);
        break;
    case PROP_Y:
        (*self->vector)[1] = g_value_get_double(value);
        break;
    case PROP_Z:
        (*self->vector)[2] = g_value_get_double(value);
        break;
        //        case PROP_VALUES:
        //            g_assert(G_VALUE_HOLDS_BOXED(value));
        //            psy_vector3_set_values(self,
        //            (GArray*)g_value_get_boxed(value)); break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void
psy_vector3_get_property(GObject    *object,
                         guint       prop_id,
                         GValue     *value,
                         GParamSpec *pspec)
{
    PsyVector3 *self = PSY_VECTOR3(object);

    switch ((PsyVector3Property) prop_id) {
    case PROP_X:
        g_value_set_double(value, (*self->vector)[0]);
        break;
    case PROP_Y:
        g_value_set_double(value, (*self->vector)[1]);
        break;
    case PROP_Z:
        g_value_set_double(value, (*self->vector)[2]);
        break;
    case PROP_MAGNITUDE:
        g_value_set_double(value, psy_vector3_get_magnitude(self));
        break;
    case PROP_UNIT:
        g_value_take_object(value, psy_vector3_unit(self));
        break;
        //        case PROP_VALUES:
        //            g_value_take_boxed(value, psy_vector3_get_values(self));
        //            break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void
psy_vector3_class_init(PsyVector3Class *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
    (void) gobject_class;

    gobject_class->set_property = psy_vector3_set_property;
    gobject_class->get_property = psy_vector3_get_property;
    gobject_class->finalize     = psy_vector3_finalize;

    obj_properties[PROP_X] = g_param_spec_double("x",
                                                 "X",
                                                 "The x value of the vector",
                                                 -G_MAXDOUBLE,
                                                 G_MAXDOUBLE,
                                                 0,
                                                 G_PARAM_READWRITE);

    obj_properties[PROP_Y] = g_param_spec_double("y",
                                                 "Y",
                                                 "The y value of the vector",
                                                 -G_MAXDOUBLE,
                                                 G_MAXDOUBLE,
                                                 0,
                                                 G_PARAM_READWRITE);

    obj_properties[PROP_Z] = g_param_spec_double("z",
                                                 "Z",
                                                 "The z value of the vector",
                                                 -G_MAXDOUBLE,
                                                 G_MAXDOUBLE,
                                                 0,
                                                 G_PARAM_READWRITE);

    obj_properties[PROP_MAGNITUDE]
        = g_param_spec_double("magnitude",
                              "magnitude",
                              "The magnitude of the vector.",
                              0,
                              G_MAXDOUBLE,
                              0,
                              G_PARAM_READABLE);

    obj_properties[PROP_UNIT]
        = g_param_spec_object("unit",
                              "unit-vector",
                              "The unit vector in the same direction as self",
                              PSY_TYPE_VECTOR3,
                              G_PARAM_READABLE);
    // TODO figure out whether a values property is possible
    // this works for C/C++ but not for python
    // the property seems to contain garbage or segfault occurs

    //    /**
    //     * PsyVector3:values:(skip):
    //     *
    //     * This property represents the values of the 3 dimensions of an
    //     * `PsyVector3` instance
    //     */
    //    obj_properties[PROP_VALUES] = g_param_spec_boxed(
    //            "values",
    //            "Values",
    //            "Initialize the vector with 3 floating point values",
    //            G_TYPE_ARRAY,
    //            G_PARAM_READWRITE
    //            );

    g_object_class_install_properties(
        gobject_class, N_PROPERTIES, obj_properties);
}

/* ************* public functions ************* */

PsyVector3 *
psy_vector3_new()
{
    return PSY_VECTOR3(g_object_new(PSY_TYPE_VECTOR3, NULL));
}

/**
 * psy_vector3_new_data:
 * @n:      length of @values n should be 0 < n <= 3
 * @values: (array length=n): list of values
 *
 * Set a new vector from data points
 *
 * Returns::A vector initialized with values, if @n < 3, the remaining elements
 *          will be set to 0.0, if n > 0 the values after 3 will be ignored.
 */
PsyVector3 *
psy_vector3_new_data(gsize n, gdouble *values)
{
    PsyVector3 *ret = psy_vector3_new();
    g_warn_if_fail(n <= 3);

    gint min = glm::min((int) n, ret->vector->length());

    for (auto i = 0; i < min; i++)
        (*ret->vector)[i] = values[i];

    return ret;
}

/**
 * psy_vector3_destroy:
 * @self: An instance to destroy
 */
void
psy_vector3_destroy(PsyVector3 *self)
{
    g_object_unref(self);
}

/**
 * psy_vector3_set_null:
 * @self: an instance of `PsyVector3`
 *
 * Sets all elements to zero turning @self into a null vector.
 */
void
psy_vector3_set_null(PsyVector3 *self)
{
    g_return_if_fail(PSY_IS_VECTOR3(self));

    *self->vector = glm::dvec3(0);
}

/**
 * psy_vector3_is_null:
 * @self: an instance of `PsyVector3`
 *
 * tests whether @self is a null vector
 *
 * Returns: #TRUE if @self is a null vector #FALSE otherwise.
 */
gboolean
psy_vector3_is_null(PsyVector3 *self)
{
    g_return_val_if_fail(PSY_IS_VECTOR3(self), FALSE);

    return *self->vector == glm::dvec3(0);
}

/**
 * psy_vector3_get_magnitude:
 * @self: an instance of `PsyVector3`
 *
 * Obtain the magnitude/length of the vector
 *
 * Returns: the maginitude of the vector
 */

gdouble
psy_vector3_get_magnitude(PsyVector3 *self)
{
    g_return_val_if_fail(PSY_IS_VECTOR3(self), 0);

    return glm::length(*self->vector);
}

/**
 * psy_vector3_unit:
 * @self: an instance of `PsyVector3` and should not be a null vector
 *
 * Returns a normalized version of it self, a vector with the same direction,
 * but a magnitude of 1.0
 *
 * Returns: (transfer full): a vector with the same direction as @self, but
 * with a magnitude of 1.
 */
PsyVector3 *
psy_vector3_unit(PsyVector3 *self)
{
    g_return_val_if_fail(PSY_IS_VECTOR3(self), nullptr);

    PsyVector3 *ret = psy_vector3_new();
    g_return_val_if_fail(ret, nullptr);
    if (psy_vector3_is_null(self)) {
        g_object_unref(ret);
        return nullptr;
    }

    *ret->vector = glm::normalize(*self->vector);
    return ret;
}

/**
 * psy_vector3_negate:
 * @self: an instance of `PsyVec3`
 *
 * Returns:(transfer full): a vector with the opposite direction and magnitude
 * of
 *                          @self.
 */
PsyVector3 *
psy_vector3_negate(PsyVector3 *self)
{
    g_return_val_if_fail(PSY_IS_VECTOR3(self), nullptr);

    PsyVector3 *ret = psy_vector3_new();
    g_return_val_if_fail(ret, nullptr);

    *ret->vector = -(*self->vector);
    return ret;
}

/**
 * psy_vector3_add_s:
 * @self: an instance of `PsyVec3`
 * @scalar: an value to add to each element of @self
 *
 * Returns:(transfer full): a vector with @scalar.
 */
PsyVector3 *
psy_vector3_add_s(PsyVector3 *self, gdouble scalar)
{
    g_return_val_if_fail(PSY_IS_VECTOR3(self), NULL);

    PsyVector3 *ret = PSY_VECTOR3(psy_vector3_new());

    *ret->vector = (*self->vector) + scalar;

    return ret;
}

/**
 * psy_vector3_add:
 * @self: an instance of `PsyVec3`
 * @other: an value to add to each element of @self
 *
 * Returns:(transfer full): the result of @self + @other
 */
PsyVector3 *
psy_vector3_add(PsyVector3 *self, PsyVector3 *other)
{
    g_return_val_if_fail(PSY_IS_VECTOR3(self), NULL);
    g_return_val_if_fail(PSY_IS_VECTOR3(other), NULL);

    PsyVector3 *ret = PSY_VECTOR3(psy_vector3_new());

    *ret->vector = (*self->vector) + (*other->vector);

    return ret;
}

/**
 * psy_vector3_sub_s:
 * @self: an instance of `PsyVec3`
 * @scalar: an value to subtract from each element of @self
 *
 * Returns:(transfer full): the result of @self - @scalar
 */
PsyVector3 *
psy_vector3_sub_s(PsyVector3 *self, gdouble scalar)
{
    g_return_val_if_fail(PSY_IS_VECTOR3(self), NULL);
    PsyVector3 *ret = PSY_VECTOR3(psy_vector3_new());

    *ret->vector = (*self->vector) - scalar;

    return ret;
}

/**
 * psy_vector3_sub:
 * @self: an instance of `PsyVec3`
 * @other: an instance `PsyVector` to subtract from @self
 *
 * Returns:(transfer full): the result from @self - @other
 */
PsyVector3 *
psy_vector3_sub(PsyVector3 *self, PsyVector3 *other)
{
    g_return_val_if_fail(PSY_IS_VECTOR3(self), NULL);
    g_return_val_if_fail(PSY_IS_VECTOR3(other), NULL);

    PsyVector3 *ret = PSY_VECTOR3(psy_vector3_new());

    *ret->vector = (*self->vector) - (*other->vector);
    return ret;
}

/**
 * psy_vector3_mul_s:
 * @self: A instance of `PsyVector3` to be scaled by @scalar
 * @scalar: The factor to scale @self with
 *
 * Returns:(transfer full): The result of @self * @scalar
 */
PsyVector3 *
psy_vector3_mul_s(PsyVector3 *self, gdouble scalar)
{
    g_return_val_if_fail(PSY_IS_VECTOR3(self), NULL);
    PsyVector3 *ret = PSY_VECTOR3(psy_vector3_new());

    *ret->vector = (*self->vector) * scalar;

    return ret;
}

/**
 * psy_vector3_mul:
 * @self: A instance of `PsyVector3` to be scaled by @scalar
 * @other: The factor to scale @self with
 *
 * Get the dot product between @self and @other.
 *
 * Returns:(transfer full): The result of @self * @other
 */
PsyVector3 *
psy_vector3_mul(PsyVector3 *self, PsyVector3 *other)
{
    g_return_val_if_fail(PSY_IS_VECTOR3(self), NULL);
    g_return_val_if_fail(PSY_IS_VECTOR3(other), NULL);

    PsyVector3 *ret = PSY_VECTOR3(psy_vector3_new());

    *ret->vector = (*self->vector) * (*other->vector);
    return ret;
}

/**
 * psy_vector3_dot:
 * @self: A instance of `PsyVector3` to be scaled by @scalar
 * @other: The factor to scale @self with
 *
 * Get the dot product between @self and @other.
 *
 * Returns: The result of @self . @other
 */
gdouble
psy_vector3_dot(PsyVector3 *self, PsyVector3 *other)
{
    g_return_val_if_fail(PSY_IS_VECTOR3(self), 0.0);
    g_return_val_if_fail(PSY_IS_VECTOR3(other), 0.0);

    return glm::dot(*self->vector, *other->vector);
}

/**
 * psy_vector3_equals:
 * @self: An instance to compare with @other
 * @other: An instance to compare with @self
 *
 * Compares whether all vector elements are equal
 *
 * Returns: the result of @self == @other
 */
gboolean
psy_vector3_equals(PsyVector3 *self, PsyVector3 *other)
{
    g_return_val_if_fail(PSY_IS_VECTOR3(self), FALSE);
    g_return_val_if_fail(PSY_IS_VECTOR3(other), FALSE);

    if (self == other)
        return TRUE;

    return *self->vector == *other->vector;
}

/**
 * psy_vector3_not_equals:
 * @self: An instance to compare with @other
 * @other: An instance to compare with @self
 *
 * Compares whether not all vector equal
 *
 * Returns: the result of @self != @other
 */
gboolean
psy_vector3_not_equals(PsyVector3 *self, PsyVector3 *other)
{
    return !psy_vector3_equals(self, other);
}

/**
 * psy_vector3_ptr:(skip)
 * @self: an instance of `PsyVector3`
 *
 * This function is used internally to get to the data efficiently.
 *
 * Returns: a pointer to the data
 */
const gdouble *
psy_vector3_ptr(PsyVector3 *self)
{
    g_return_val_if_fail(PSY_IS_VECTOR3(self), NULL);

    return glm::value_ptr(*self->vector);
}

/**
 * psy_vector3_get_values:
 * @self: An instance of `PsyVector` whose elements you'd like to get
 *
 * Returns:(transfer full)(element-type gdouble): the elements of the vector
 */
GArray *
psy_vector3_get_values(PsyVector3 *self)
{
    g_return_val_if_fail(PSY_IS_VECTOR3(self), NULL);

    GArray *ret = g_array_sized_new(FALSE, FALSE, sizeof(gdouble), 3);
    g_array_append_vals(ret, psy_vector3_ptr(self), 3);

    return ret;
}

/**
 * psy_vector3_set_values:
 * @self: an instance of `PsyVector3` whose values to set
 * @array:(transfer none)(element-type gdouble): an
 *        array with 3 value to set this array with
 *
 * Set the values of @self
 */
void
psy_vector3_set_values(PsyVector3 *self, GArray *array)
{
    g_return_if_fail(PSY_IS_VECTOR3(self));
    g_return_if_fail(array);

    gdouble *in  = reinterpret_cast<gdouble *>(array->data);
    guint    min = array->len > 3 ? 3 : array->len;

    for (guint i = 0; i < min; i++)
        (*self->vector)[i] = in[i];
}

gchar *
psy_vector3_as_string(PsyVector3 *self)
{
    g_return_val_if_fail(PSY_IS_VECTOR3(self), NULL);

    std::string output = glm::to_string(*self->vector);
    return g_strdup(output.c_str());
}
