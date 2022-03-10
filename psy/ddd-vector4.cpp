
#include "ddd-vector4.h"
#include <glm/detail/type_vec4.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <string>

typedef struct _DddVector4 {
    GObject obj;
    glm::dvec4* vector;
} DddVector4;


G_DEFINE_TYPE(DddVector4, ddd_vector4, G_TYPE_OBJECT)

typedef enum {
    PROP_NULL,
    PROP_X,
    PROP_Y,
    PROP_Z,
    PROP_W,
    PROP_MAGNITUDE,
    PROP_UNIT,
    N_PROPERTIES
}DddVector4Property;

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

static void
ddd_vector4_init(DddVector4* self)
{
    self->vector = new glm::dvec4();
}

static void
ddd_vector4_finalize(GObject* obj)
{
    DddVector4 *self = DDD_VECTOR4(obj);

    delete self->vector;

    G_OBJECT_CLASS(ddd_vector4_parent_class)->finalize(obj);
}

static void
ddd_vector4_set_property(GObject      *object,
                         guint         prop_id,
                         const GValue *value,
                         GParamSpec   *pspec)
{
    DddVector4 *self = DDD_VECTOR4(object);

    switch ((DddVector4Property)prop_id)
    {
        case PROP_X:
            (*self->vector)[0] = g_value_get_double(value);
            break;
        case PROP_Y:
            (*self->vector)[1] = g_value_get_double(value);
            break;
        case PROP_Z:
            (*self->vector)[2] = g_value_get_double(value);
            break;
        case PROP_W:
            (*self->vector)[3] = g_value_get_double(value);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
ddd_vector4_get_property(GObject      *object,
                         guint         prop_id,
                         GValue       *value,
                         GParamSpec   *pspec)
{
    DddVector4 *self = DDD_VECTOR4(object);

    switch ((DddVector4Property)prop_id)
    {
        case PROP_X:
            g_value_set_double(value,(*self->vector)[0]);
            break;
        case PROP_Y:
            g_value_set_double(value,(*self->vector)[1]);
            break;
        case PROP_Z:
            g_value_set_double(value,(*self->vector)[2]);
            break;
        case PROP_W:
            g_value_set_double(value,(*self->vector)[3]);
            break;
        case PROP_MAGNITUDE:
            g_value_set_double(value, ddd_vector4_get_magnitude(self));
            break;
        case PROP_UNIT:
            g_value_set_object(value, ddd_vector4_normalize(self));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
ddd_vector4_class_init(DddVector4Class* klass)
{
    GObjectClass* gobject_class = G_OBJECT_CLASS(klass);
    (void) gobject_class;

    gobject_class->set_property = ddd_vector4_set_property;
    gobject_class->get_property = ddd_vector4_get_property;
    gobject_class->finalize = ddd_vector4_finalize;

    obj_properties[PROP_X] = g_param_spec_double(
            "x",
            "X",
            "The x value of the vector",
            -G_MAXDOUBLE,
            G_MAXDOUBLE,
            0,
            G_PARAM_READWRITE
            );

    obj_properties[PROP_Y] = g_param_spec_double(
            "y",
            "Y",
            "The y value of the vector",
            -G_MAXDOUBLE,
            G_MAXDOUBLE,
            0,
            G_PARAM_READWRITE
            );

    obj_properties[PROP_Z] = g_param_spec_double(
            "z",
            "Z",
            "The z value of the vector",
            -G_MAXDOUBLE,
            G_MAXDOUBLE,
            0,
            G_PARAM_READWRITE
            );

    obj_properties[PROP_W] = g_param_spec_double(
            "w",
            "W",
            "The w value of the vector",
            -G_MAXDOUBLE,
            G_MAXDOUBLE,
            0,
            G_PARAM_READWRITE
            );

    obj_properties[PROP_MAGNITUDE] = g_param_spec_double(
            "magnitude",
            "magnitude",
            "The magnitude of the vector.",
            0,
            G_MAXDOUBLE,
            0,
            G_PARAM_READABLE
            );

    obj_properties[PROP_UNIT] = g_param_spec_object (
            "unit",
            "unit-vector",
            "The unit vector in the same direction as self",
            DDD_TYPE_VECTOR4,
            G_PARAM_READABLE
            );

    g_object_class_install_properties(
            gobject_class,
            N_PROPERTIES,
            obj_properties
            );
}


/* ************* public functions ************* */

DddVector4*
ddd_vector4_new()
{
    return DDD_VECTOR4(g_object_new(DDD_TYPE_VECTOR4, NULL));
}

/**
 * Set a new vector from data points
 *
 * @n:      length of @values
 * @values: (array length=n): list of values
 *
 * Returns::A vector initialized with values, if n < 4, the remaining elements
 * will be
 */
DddVector4*
ddd_vector4_new_data(gsize n, gdouble* values)
{
    DddVector4 * ret = ddd_vector4_new();
    g_return_val_if_fail(n <= 4, nullptr);

    gint min = glm::min((int)n, ret->vector->length());

    for (auto i = 0; i < min; i++)
        (*ret->vector)[i] = values[i];

    return ret;
}

void
ddd_vector4_destroy(DddVector4* self)
{
    g_object_unref(self);
}

void
ddd_vector4_set_null(DddVector4* self)
{
    g_return_if_fail(DDD_IS_VECTOR4(self));

    *self->vector = glm::dvec4(0);
}

gboolean
ddd_vector4_is_null(DddVector4* self)
{
    g_return_val_if_fail(DDD_IS_VECTOR4(self), FALSE);

    return *self->vector == glm::dvec4(0);
}

gdouble
ddd_vector4_get_magnitude(DddVector4* self)
{
    g_return_val_if_fail(DDD_IS_VECTOR4(self), 0);

    return glm::length(*self->vector);
}

DddVector4*
ddd_vector4_normalize(DddVector4* self)
{
    g_return_val_if_fail(DDD_IS_VECTOR4(self), nullptr);

    DddVector4* ret = ddd_vector4_new();
    g_return_val_if_fail(ret , nullptr);
    if (ddd_vector4_is_null(self))
        return nullptr;

    *ret->vector = glm::normalize(*self->vector);
    return ret;
}

DddVector4*
ddd_vector4_negate(DddVector4* self)
{
    g_return_val_if_fail(DDD_IS_VECTOR4(self), nullptr);

    DddVector4* ret = ddd_vector4_new();
    g_return_val_if_fail(ret , nullptr);

    *ret->vector = -(*self->vector);
    return ret;
}

DddVector4*
ddd_vector4_add_s(DddVector4* self, gdouble s)
{
    g_return_val_if_fail(DDD_IS_VECTOR4(self), NULL);

    DddVector4* ret = DDD_VECTOR4(ddd_vector4_new());

    *ret->vector = (*self->vector) + s;

    return ret;
}

DddVector4*
ddd_vector4_add(DddVector4* v1, DddVector4 *v2)
{
    g_return_val_if_fail(DDD_IS_VECTOR4(v1), NULL);
    g_return_val_if_fail(DDD_IS_VECTOR4(v2), NULL);

    DddVector4* ret = DDD_VECTOR4(ddd_vector4_new());

    *ret->vector = (*v1->vector) + (*v2->vector);

    return ret;
}

DddVector4*
ddd_vector4_sub_s(DddVector4* self, gdouble s)
{
    g_return_val_if_fail(DDD_IS_VECTOR4(self), NULL);
    DddVector4* ret = DDD_VECTOR4(ddd_vector4_new());

    *ret->vector = (*self->vector) - s;

    return ret;
}

DddVector4*
ddd_vector4_sub(DddVector4* v1, DddVector4 *v2)
{
    g_return_val_if_fail(DDD_IS_VECTOR4(v1), NULL);
    g_return_val_if_fail(DDD_IS_VECTOR4(v2), NULL);

    DddVector4* ret = DDD_VECTOR4(ddd_vector4_new());

    *ret->vector = (*v1->vector) - (*v2->vector);
    return ret;
}

DddVector4*
ddd_vector4_mul_s(DddVector4* self, gdouble scalar)
{
    g_return_val_if_fail(DDD_IS_VECTOR4(self), NULL);
    DddVector4* ret = DDD_VECTOR4(ddd_vector4_new());

    *ret->vector = (*self->vector) * scalar;

    return ret;
}

DddVector4*
ddd_vector4_mul(DddVector4* v1, DddVector4* v2)
{
    g_return_val_if_fail(DDD_IS_VECTOR4(v1), NULL);
    g_return_val_if_fail(DDD_IS_VECTOR4(v2), NULL);

    DddVector4* ret = DDD_VECTOR4(ddd_vector4_new());

    *ret->vector = (*v1->vector) * (*v2->vector);
    return ret;
}

gdouble
ddd_vector4_dot(DddVector4* v1, DddVector4* v2)
{
    g_return_val_if_fail(DDD_IS_VECTOR4(v1), 0.0);
    g_return_val_if_fail(DDD_IS_VECTOR4(v2), 0.0);

    return glm::dot(*v1->vector, *v2->vector);
}

gboolean
ddd_vector4_equals(DddVector4* v1, DddVector4* v2)
{
    g_return_val_if_fail(DDD_IS_VECTOR4(v1), FALSE);
    g_return_val_if_fail(DDD_IS_VECTOR4(v2), FALSE);

    if (v1 == v2)
        return TRUE;

    return  *v1->vector == *v2->vector;
}

gboolean
ddd_vector4_not_equals(DddVector4* v1, DddVector4* v2)
{
    return !ddd_vector4_equals(v1, v2);
}

const gdouble*
ddd_vector4_ptr(DddVector4* self)
{
    g_return_val_if_fail(DDD_IS_VECTOR4(self), NULL);

    return glm::value_ptr(*self->vector);
}

gchar*
ddd_vector4_as_string(DddVector4* self)
{
    g_return_val_if_fail(DDD_IS_VECTOR4(self), NULL);

    std::string output = glm::to_string(*self->vector);
    return g_strdup(output.c_str());
}
