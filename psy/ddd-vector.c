
#include "ddd-vector.h"
#include <math.h>
#ifndef NAN
#warning "DddVector requires math.h to export NAN"
#else

G_DEFINE_TYPE(DddVector, ddd_vector, G_TYPE_OBJECT)

typedef enum {
    PROP_0,
    PROP_X,
    PROP_Y,
    PROP_Z,
    PROP_LENGTH,

    N_PROPERTIES
}DddVectorProperty;

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

static void ddd_vector_init(DddVector* self)
{
    (void) self;
}

static void
ddd_vector_set_property(GObject      *object,
                        guint         prop_id,
                        const GValue *value,
                        GParamSpec   *pspec)
{
    DddVector *vector = DDD_VECTOR(object);

    switch ((DddVectorProperty)prop_id)
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
ddd_vector_get_property(GObject      *object,
                        guint         prop_id,
                        GValue       *value,
                        GParamSpec   *pspec)
{
    DddVector *vector = DDD_VECTOR(object);

    switch ((DddVectorProperty)prop_id)
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
            g_value_set_float(value, ddd_vector_magnitude(vector));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
ddd_vector_class_init(DddVectorClass* klass)
{
    GObjectClass* gobject_class = G_OBJECT_CLASS(klass);

    gobject_class->set_property = ddd_vector_set_property;
    gobject_class->get_property = ddd_vector_get_property;

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

DddVector*
ddd_vector_new()
{
    return ddd_vector_new_xyz(0,0,0);
}

DddVector*
ddd_vector_new_x(gfloat x)
{
    return ddd_vector_new_xyz(x, 0 ,0);
}

DddVector*
ddd_vector_new_xy(gfloat x, gfloat y)
{
    return ddd_vector_new_xyz(x, y, 0);
}

DddVector*
ddd_vector_new_xyz(gfloat x, gfloat y, gfloat z)
{
    DddVector *v = g_object_new(DDD_TYPE_VECTOR, NULL);
    if (v) {
        v->array[0] = x;
        v->array[1] = y;
        v->array[2] = z;
    }
    return v;
}

void
ddd_vector_destroy(DddVector* v) {
    g_object_unref(v);
}

gfloat
ddd_vector_magnitude(DddVector* v)
{
    g_return_val_if_fail(DDD_IS_VECTOR(v), NAN);
    gfloat sum = 0;
    for (size_t i = 0; i < (sizeof(v->array)/sizeof(v->array[0])); i++)
        sum += (v->array[i] * v->array[i]);
    return sqrtf(sum);
}

DddVector*
ddd_vector_unit(DddVector* v)
{
    g_return_val_if_fail(DDD_IS_VECTOR(v), NULL);
    gfloat mag = ddd_vector_magnitude(v);

    if (mag == 0.0) // oops there is no unit vector of zero vector.
        return NULL;

    gfloat x = v->array[0] / mag;
    gfloat y = v->array[1] / mag;
    gfloat z = v->array[2] / mag;

    return ddd_vector_new_xyz(x, y, z);
}

DddVector*
ddd_vector_negate(DddVector* v)
{
    g_return_val_if_fail(DDD_IS_VECTOR(v), NULL);
    gfloat x = -v->array[0];
    gfloat y = -v->array[1];
    gfloat z = -v->array[2];
    return ddd_vector_new_xyz(x, y, z);
}

DddVector*
ddd_vector_add_s(DddVector* v, gfloat s)
{
    g_return_val_if_fail(DDD_IS_VECTOR(v), NULL);
    gfloat x, y, z;
    x = v->array[0] + s;
    y = v->array[1] + s;
    z = v->array[2] + s;
    return ddd_vector_new_xyz(x, y, z);
}

DddVector*
ddd_vector_add(DddVector* v1, DddVector *v2)
{
    g_return_val_if_fail(DDD_IS_VECTOR(v1), NULL);
    g_return_val_if_fail(DDD_IS_VECTOR(v2), NULL);
    gfloat x, y, z;
    x = v1->array[0] + v2->array[0];
    y = v1->array[1] + v2->array[1];
    z = v1->array[2] + v2->array[2];
    return ddd_vector_new_xyz(x, y, z);
}

DddVector*
ddd_vector_sub_s(DddVector* v, gfloat s)
{
    g_return_val_if_fail(DDD_IS_VECTOR(v), NULL);
    gfloat x, y, z;
    x = v->array[0] - s;
    y = v->array[1] - s;
    z = v->array[2] - s;
    return ddd_vector_new_xyz(x, y, z);
}

DddVector*
ddd_vector_sub(DddVector* v1, DddVector *v2)
{
    g_return_val_if_fail(DDD_IS_VECTOR(v1), NULL);
    g_return_val_if_fail(DDD_IS_VECTOR(v2), NULL);
    gfloat x, y, z;
    x = v1->array[0] - v2->array[0];
    y = v1->array[1] - v2->array[1];
    z = v1->array[2] - v2->array[2];
    return ddd_vector_new_xyz(x, y, z);
}

DddVector*
ddd_vector_mul_s(DddVector* v, gfloat s)
{
    g_return_val_if_fail(DDD_IS_VECTOR(v), NULL);
    gfloat x, y, z;
    x = v->array[0] * s;
    y = v->array[1] * s;
    z = v->array[2] * s;
    return ddd_vector_new_xyz(x, y, z);
}

gfloat
ddd_vector_dot(DddVector* v1, DddVector* v2)
{
    g_return_val_if_fail(DDD_IS_VECTOR(v1), 0);
    g_return_val_if_fail(DDD_IS_VECTOR(v2), 0);
    gfloat l1, l2;
    l1 = ddd_vector_magnitude(v1);
    l2 = ddd_vector_magnitude(v2);
    return (v1->array[0] / l1) * (v2->array[0] / l2)+
           (v1->array[1] / l1) * (v2->array[1] / l2)+
           (v1->array[2] / l1) * (v2->array[2] / l2);
}

DddVector*
ddd_vector_cross(DddVector* v1, DddVector* v2)
{
    g_return_val_if_fail(DDD_IS_VECTOR(v1), NULL);
    g_return_val_if_fail(DDD_IS_VECTOR(v2), NULL);
    gfloat x, y, z;
    x = v1->array[1]  * v2->array[2] - v1->array[2] * v2->array[1];
    y = v1->array[2]  * v2->array[0] - v1->array[0] * v2->array[2];
    z = v1->array[0]  * v2->array[1] - v1->array[1] * v2->array[0];
    return ddd_vector_new_xyz(x, y, z);
}

gboolean
ddd_vector_equals(DddVector* v1, DddVector* v2)
{
    g_return_val_if_fail(DDD_IS_VECTOR(v1), FALSE);
    g_return_val_if_fail(DDD_IS_VECTOR(v2), FALSE);

    if (v1 == v2)
        return TRUE;

    return v1->array[0] == v2->array[0] &&
           v1->array[1] == v2->array[1] &&
           v1->array[2] == v2->array[2];
}

gboolean
ddd_vector_not_equals(DddVector* v1, DddVector* v2)
{
    return !ddd_vector_equals(v1, v2);
}

#endif
