
#include "ddd-matrix4.h"
#include <glm/detail/type_mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>


typedef struct _DddMatrix4 DddMatrix4;

struct _DddMatrix4 {
    GObject parent_instance;
    glm::dmat4x4* matrix;
};

G_DEFINE_TYPE(DddMatrix4, ddd_matrix4, G_TYPE_OBJECT)

typedef enum {
    PROP_NULL,
    IS_IDENTITY,
    IS_NULL,
    N_PROPERTIES
}DddMatrix4Property;

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

static void ddd_matrix4_init(DddMatrix4* self)
{
    self->matrix = new glm::dmat4x4();
}

static void ddd_matrix4_finalize(GObject* obj)
{
    DddMatrix4* self = DDD_MATRIX4(obj);
    delete self->matrix;
    G_OBJECT_CLASS(ddd_matrix4_parent_class)->finalize(obj);
}

static void
ddd_matrix4_set_property(GObject      *object,
                         guint         prop_id,
                         const GValue *value,
                         GParamSpec   *pspec)
{
    DddMatrix4 *matrix4 = DDD_MATRIX4(object);

    switch ((DddMatrix4Property)prop_id)
    {
        case IS_IDENTITY:
            {
                gboolean identity = g_value_get_boolean(value);
                if(identity)
                    ddd_matrix4_set_identity(matrix4);
            }
            break;
        case IS_NULL:
            {
                gboolean isnull = g_value_get_boolean(value);
                if(isnull)
                    ddd_matrix4_set_null(matrix4);
            }
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
ddd_matrix4_get_property(GObject      *object,
                         guint         prop_id,
                         GValue       *value,
                         GParamSpec   *pspec)
{
    DddMatrix4 *matrix4 = DDD_MATRIX4(object);

    switch ((DddMatrix4Property)prop_id)
    {
        case IS_IDENTITY:
            g_value_set_boolean(value, ddd_matrix4_is_identity(matrix4));
            break;
        case IS_NULL:
            g_value_set_boolean(value, ddd_matrix4_is_null(matrix4));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
ddd_matrix4_class_init(DddMatrix4Class* klass)
{
    GObjectClass* gobject_class = G_OBJECT_CLASS(klass);
    (void) gobject_class;

    gobject_class->set_property = ddd_matrix4_set_property;
    gobject_class->get_property = ddd_matrix4_get_property;
    gobject_class->finalize = ddd_matrix4_finalize;

    obj_properties[IS_IDENTITY] = g_param_spec_boolean(
            "is-identity",
            "is_idendity",
            "Test whether the matrix is an identity matrix or make it an identity matrix",
            FALSE,
            G_PARAM_READWRITE
            );

    obj_properties[IS_NULL] = g_param_spec_boolean(
            "is-null",
            "is_null",
            "Test whether or set all element to 0.",
            FALSE,
            G_PARAM_READWRITE
            );

    g_object_class_install_properties(
            gobject_class,
            N_PROPERTIES,
            obj_properties
            );
}


/* ************* public functions ************* */

DddMatrix4*
ddd_matrix4_new()
{
    return DDD_MATRIX4(g_object_new(DDD_TYPE_MATRIX4, NULL));
}

DddMatrix4*
ddd_matrix4_new_ortographic(
        gdouble left,
        gdouble right,
        gdouble bottom,
        gdouble top,
        gdouble z_near,
        gdouble z_far
        )
{
    DddMatrix4* ret = ddd_matrix4_new();

    *ret->matrix = glm::ortho(left, right, bottom, top, z_near, z_far);

    return ret;
}

DddMatrix4*
ddd_matrix4_new_perspective(
        gdouble fovy,
        gdouble aspect,
        gdouble z_near,
        gdouble z_far
        )
{
    DddMatrix4* ret = ddd_matrix4_new();

    *ret->matrix = glm::perspective(fovy, aspect, z_near, z_far);

    return ret;
}

void
ddd_matrix4_destroy(DddMatrix4* self)
{
    g_object_unref(self);
}

void
ddd_matrix4_set_identity(DddMatrix4* self)
{
    g_return_if_fail(DDD_IS_MATRIX4(self));

    *self->matrix = glm::dmat4x4 (1);
}

gboolean
ddd_matrix4_is_identity(DddMatrix4* self)
{
    g_return_val_if_fail(DDD_IS_MATRIX4(self), FALSE);

    return *self->matrix == glm::dmat4x4 (1);
}

void
ddd_matrix4_set_null(DddMatrix4* self)
{
    g_return_if_fail(DDD_IS_MATRIX4(self));

    *self->matrix = glm::dmat4x4 (0);
}

gboolean
ddd_matrix4_is_null(DddMatrix4* self)
{
    g_return_val_if_fail(DDD_IS_MATRIX4(self), FALSE);

    return *self->matrix == glm::dmat4x4 (0);
}


DddMatrix4*
ddd_matrix4_add_s(DddMatrix4* self, gdouble s)
{
    g_return_val_if_fail(DDD_IS_MATRIX4(self), NULL);

    DddMatrix4* ret = DDD_MATRIX4(ddd_matrix4_new());

    *ret->matrix = (*self->matrix) + s;

    return ret;
}

DddMatrix4*
ddd_matrix4_add(DddMatrix4* v1, DddMatrix4 *v2)
{
    g_return_val_if_fail(DDD_IS_MATRIX4(v1), NULL);
    g_return_val_if_fail(DDD_IS_MATRIX4(v2), NULL);


    DddMatrix4* ret = DDD_MATRIX4(ddd_matrix4_new());

    *ret->matrix = (*v1->matrix) + (*v2->matrix);

    return ret;
}

DddMatrix4*
ddd_matrix4_sub_s(DddMatrix4* self, gdouble scalar)
{
    g_return_val_if_fail(DDD_IS_MATRIX4(self), NULL);

    DddMatrix4* ret = DDD_MATRIX4(ddd_matrix4_new());

    *ret->matrix = (*self->matrix) - scalar;

    return ret;
}

DddMatrix4*
ddd_matrix4_sub(DddMatrix4* v1, DddMatrix4 *v2)
{
    g_return_val_if_fail(DDD_IS_MATRIX4(v1), NULL);
    g_return_val_if_fail(DDD_IS_MATRIX4(v2), NULL);


    DddMatrix4* ret = DDD_MATRIX4(ddd_matrix4_new());

    *ret->matrix = (*v1->matrix) - (*v2->matrix);
    return ret;
}

DddMatrix4*
ddd_matrix4_mul_s(DddMatrix4* self, gdouble scalar)
{
    g_return_val_if_fail(DDD_IS_MATRIX4(self), NULL);
    DddMatrix4* ret = ddd_matrix4_new();

    *ret->matrix = (*self->matrix) * scalar;

    return ret;
}

DddMatrix4*
ddd_matrix4_mul(DddMatrix4* v1, DddMatrix4* v2)
{
    g_return_val_if_fail(DDD_IS_MATRIX4(v1), NULL);
    g_return_val_if_fail(DDD_IS_MATRIX4(v2), NULL);

    DddMatrix4* ret = ddd_matrix4_new();

    *ret->matrix = (*v1->matrix) - (*v2->matrix);
    return ret;
}

gboolean
ddd_matrix4_equals(DddMatrix4* v1, DddMatrix4* v2)
{
    g_return_val_if_fail(DDD_IS_MATRIX4(v1), FALSE);
    g_return_val_if_fail(DDD_IS_MATRIX4(v2), FALSE);

    if (v1 == v2)
        return TRUE;

    return  *v1->matrix == *v2->matrix;
}

gboolean
ddd_matrix4_not_equals(DddMatrix4* v1, DddMatrix4* v2)
{
    return !ddd_matrix4_equals(v1, v2);
}

const gdouble*
ddd_matrix4_ptr(DddMatrix4* self)
{
    g_return_val_if_fail(DDD_IS_MATRIX4(self), NULL);

    return glm::value_ptr(*self->matrix);
}
