
#include "psy-matrix4.h"
#include <glm/detail/type_mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>


typedef struct _PsyMatrix4 PsyMatrix4;

struct _PsyMatrix4 {
    GObject parent_instance;
    glm::dmat4x4* matrix;
};

G_DEFINE_TYPE(PsyMatrix4, psy_matrix4, G_TYPE_OBJECT)

typedef enum {
    PROP_NULL,
    IS_IDENTITY,
    IS_NULL,
    N_PROPERTIES
}PsyMatrix4Property;

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

static void psy_matrix4_init(PsyMatrix4* self)
{
    self->matrix = new glm::dmat4x4();
}

static void psy_matrix4_finalize(GObject* obj)
{
    PsyMatrix4* self = PSY_MATRIX4(obj);
    delete self->matrix;
    G_OBJECT_CLASS(psy_matrix4_parent_class)->finalize(obj);
}

static void
psy_matrix4_set_property(GObject      *object,
                         guint         prop_id,
                         const GValue *value,
                         GParamSpec   *pspec)
{
    PsyMatrix4 *matrix4 = PSY_MATRIX4(object);

    switch ((PsyMatrix4Property)prop_id)
    {
        case IS_IDENTITY:
            {
                gboolean identity = g_value_get_boolean(value);
                if(identity)
                    psy_matrix4_set_identity(matrix4);
            }
            break;
        case IS_NULL:
            {
                gboolean isnull = g_value_get_boolean(value);
                if(isnull)
                    psy_matrix4_set_null(matrix4);
            }
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
psy_matrix4_get_property(GObject      *object,
                         guint         prop_id,
                         GValue       *value,
                         GParamSpec   *pspec)
{
    PsyMatrix4 *matrix4 = PSY_MATRIX4(object);

    switch ((PsyMatrix4Property)prop_id)
    {
        case IS_IDENTITY:
            g_value_set_boolean(value, psy_matrix4_is_identity(matrix4));
            break;
        case IS_NULL:
            g_value_set_boolean(value, psy_matrix4_is_null(matrix4));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
psy_matrix4_class_init(PsyMatrix4Class* klass)
{
    GObjectClass* gobject_class = G_OBJECT_CLASS(klass);
    (void) gobject_class;

    gobject_class->set_property = psy_matrix4_set_property;
    gobject_class->get_property = psy_matrix4_get_property;
    gobject_class->finalize = psy_matrix4_finalize;

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

PsyMatrix4*
psy_matrix4_new()
{
    return PSY_MATRIX4(g_object_new(PSY_TYPE_MATRIX4, NULL));
}

PsyMatrix4*
psy_matrix4_new_ortographic(
        gdouble left,
        gdouble right,
        gdouble bottom,
        gdouble top,
        gdouble z_near,
        gdouble z_far
        )
{
    PsyMatrix4* ret = psy_matrix4_new();

    *ret->matrix = glm::ortho(left, right, bottom, top, z_near, z_far);

    return ret;
}

PsyMatrix4*
psy_matrix4_new_perspective(
        gdouble fovy,
        gdouble aspect,
        gdouble z_near,
        gdouble z_far
        )
{
    PsyMatrix4* ret = psy_matrix4_new();

    *ret->matrix = glm::perspective(fovy, aspect, z_near, z_far);

    return ret;
}

void
psy_matrix4_destroy(PsyMatrix4* self)
{
    g_object_unref(self);
}

void
psy_matrix4_set_identity(PsyMatrix4* self)
{
    g_return_if_fail(PSY_IS_MATRIX4(self));

    *self->matrix = glm::dmat4x4 (1);
}

gboolean
psy_matrix4_is_identity(PsyMatrix4* self)
{
    g_return_val_if_fail(PSY_IS_MATRIX4(self), FALSE);

    return *self->matrix == glm::dmat4x4 (1);
}

void
psy_matrix4_set_null(PsyMatrix4* self)
{
    g_return_if_fail(PSY_IS_MATRIX4(self));

    *self->matrix = glm::dmat4x4 (0);
}

gboolean
psy_matrix4_is_null(PsyMatrix4* self)
{
    g_return_val_if_fail(PSY_IS_MATRIX4(self), FALSE);

    return *self->matrix == glm::dmat4x4 (0);
}


PsyMatrix4*
psy_matrix4_add_s(PsyMatrix4* self, gdouble s)
{
    g_return_val_if_fail(PSY_IS_MATRIX4(self), NULL);

    PsyMatrix4* ret = PSY_MATRIX4(psy_matrix4_new());

    *ret->matrix = (*self->matrix) + s;

    return ret;
}

PsyMatrix4*
psy_matrix4_add(PsyMatrix4* v1, PsyMatrix4 *v2)
{
    g_return_val_if_fail(PSY_IS_MATRIX4(v1), NULL);
    g_return_val_if_fail(PSY_IS_MATRIX4(v2), NULL);


    PsyMatrix4* ret = PSY_MATRIX4(psy_matrix4_new());

    *ret->matrix = (*v1->matrix) + (*v2->matrix);

    return ret;
}

PsyMatrix4*
psy_matrix4_sub_s(PsyMatrix4* self, gdouble scalar)
{
    g_return_val_if_fail(PSY_IS_MATRIX4(self), NULL);

    PsyMatrix4* ret = PSY_MATRIX4(psy_matrix4_new());

    *ret->matrix = (*self->matrix) - scalar;

    return ret;
}

PsyMatrix4*
psy_matrix4_sub(PsyMatrix4* v1, PsyMatrix4 *v2)
{
    g_return_val_if_fail(PSY_IS_MATRIX4(v1), NULL);
    g_return_val_if_fail(PSY_IS_MATRIX4(v2), NULL);


    PsyMatrix4* ret = PSY_MATRIX4(psy_matrix4_new());

    *ret->matrix = (*v1->matrix) - (*v2->matrix);
    return ret;
}

PsyMatrix4*
psy_matrix4_mul_s(PsyMatrix4* self, gdouble scalar)
{
    g_return_val_if_fail(PSY_IS_MATRIX4(self), NULL);
    PsyMatrix4* ret = psy_matrix4_new();

    *ret->matrix = (*self->matrix) * scalar;

    return ret;
}

PsyMatrix4*
psy_matrix4_mul(PsyMatrix4* v1, PsyMatrix4* v2)
{
    g_return_val_if_fail(PSY_IS_MATRIX4(v1), NULL);
    g_return_val_if_fail(PSY_IS_MATRIX4(v2), NULL);

    PsyMatrix4* ret = psy_matrix4_new();

    *ret->matrix = (*v1->matrix) - (*v2->matrix);
    return ret;
}

gboolean
psy_matrix4_equals(PsyMatrix4* v1, PsyMatrix4* v2)
{
    g_return_val_if_fail(PSY_IS_MATRIX4(v1), FALSE);
    g_return_val_if_fail(PSY_IS_MATRIX4(v2), FALSE);

    if (v1 == v2)
        return TRUE;

    return  *v1->matrix == *v2->matrix;
}

gboolean
psy_matrix4_not_equals(PsyMatrix4* v1, PsyMatrix4* v2)
{
    return !psy_matrix4_equals(v1, v2);
}

const gdouble*
psy_matrix4_ptr(PsyMatrix4* self)
{
    g_return_val_if_fail(PSY_IS_MATRIX4(self), NULL);

    return glm::value_ptr(*self->matrix);
}
