
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

/**
 * psy_matrix3_new:
 *
 * Creates a new `PsyMatrix3`
 *
 * This constructs default 3 dimensional matrix with
 * all values set to -1.0
 *
 * Returns: a new `PsyMatrix3`
 */
PsyMatrix4*
psy_matrix4_new()
{
    return PSY_MATRIX4(g_object_new(PSY_TYPE_MATRIX4, NULL));
}

/**
 * psy_matrix4_new_ortographic:
 *
 * Create a new `PsyMatrix4` that describes a ortographic
 * projection. The matrix represents a projection
 * in which objects nearby do not appear smaller than
 * objects at a distance.
 *
 * left: The left hand side of the clipping volume
 * right: The right hand side of the clipping volume
 * bottom: the bottom of the clipping volume
 * top: the top of the clipping volume
 * z_near: The near end of the clipping volume
 * z_far: The far end of the clipping volume
 *
 * Returns: A newly created matrix representing an
 *          orthographic projection.
 */
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

/**
 * psy_matrix4_new_perspective:
 *
 * Create a new `PsyMatrix4` that describes an perspective
 * transformations. Object nearby appear larger than
 * object with a similar size that are further away.
 *
 * fovy:
 * aspect:
 * z_near:
 * z_far:
 *
 * Returns: A new PsyMatrix4 that describes a perspective
 *          projection.
 */
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

/**
 * psy_matrix4_destroy:
 *
 * Destroys a `PsyMatrix4` instance
 *
 * self: the matrix to destroy
 */
void
psy_matrix4_destroy(PsyMatrix4* self)
{
    g_object_unref(self);
}

/**
 * psy_matrix4_set_identity:
 *
 * Restore the matrix back to an identity matrix
 *
 * self: the matrix to reset to an identity matrix
 */
void
psy_matrix4_set_identity(PsyMatrix4* self)
{
    g_return_if_fail(PSY_IS_MATRIX4(self));

    *self->matrix = glm::dmat4x4 (1);
}

/**
 * psy_matrix4_is_identity:
 *
 * Tests whether the matrix is an identity matrix.
 *
 * self: the matrix to test whether it is an identity
 *       matrix
 *
 * returns: TRUE if it is and identity matrix, FALSE otherwise
 */
gboolean
psy_matrix4_is_identity(PsyMatrix4* self)
{
    g_return_val_if_fail(PSY_IS_MATRIX4(self), FALSE);

    return *self->matrix == glm::dmat4x4 (1);
}

/**
 * psy_matrix4_set_null:
 *
 * Resets all members of the matrix to 0.
 *
 * self: Reset the matrix to be a null matrix.
 */
void
psy_matrix4_set_null(PsyMatrix4* self)
{
    g_return_if_fail(PSY_IS_MATRIX4(self));

    *self->matrix = glm::dmat4x4 (0);
}

/**
 * psy_matrix4_is_null:
 *
 * Tests whether the matrix is an null matrix.
 *
 * self: the matrix to test whether it is an null
 *       matrix
 *
 * Returns: TRUE if it is and null matrix, FALSE otherwise
 */
gboolean
psy_matrix4_is_null(PsyMatrix4* self)
{
    g_return_val_if_fail(PSY_IS_MATRIX4(self), FALSE);

    return *self->matrix == glm::dmat4x4 (0);
}

/**
 * psy_matrix4_add_s:
 * @self : the matrix to which @scalar is added
 * @s: the scalar to add to @self
 *
 * Returns :(transfer full): The result of self + s
 */
PsyMatrix4*
psy_matrix4_add_s(PsyMatrix4* self, gdouble scalar)
{
    g_return_val_if_fail(PSY_IS_MATRIX4(self), NULL);

    PsyMatrix4* ret = PSY_MATRIX4(psy_matrix4_new());

    *ret->matrix = (*self->matrix) + scalar;

    return ret;
}

/**
 * psy_matrix4_add:
 * @self : the matrix to which @v2 is added
 * @v2: the matrix which to add to @self
 *
 * Returns :(transfer full): The result of self + s
 */
PsyMatrix4*
psy_matrix4_add(PsyMatrix4* v1, PsyMatrix4 *v2)
{
    g_return_val_if_fail(PSY_IS_MATRIX4(v1), NULL);
    g_return_val_if_fail(PSY_IS_MATRIX4(v2), NULL);

    PsyMatrix4* ret = PSY_MATRIX4(psy_matrix4_new());

    *ret->matrix = (*v1->matrix) + (*v2->matrix);

    return ret;
}

/**
 * psy_matrix4_sub_s:
 * @self: an `PsyMatrix4` from which @scalar is subtracted
 * @scalar: The scalar that is subracted from @self
 *
 * Performs scalar subtraction, the scal is subtractef from
 * every elemen in @self.
 *
 * Returns:(transfer full): A newly inialized `PsyMatix4`
 *         that is the result of self - scalar.
 */
PsyMatrix4*
psy_matrix4_sub_s(PsyMatrix4* self, gdouble scalar)
{
    g_return_val_if_fail(PSY_IS_MATRIX4(self), NULL);

    PsyMatrix4* ret = PSY_MATRIX4(psy_matrix4_new());

    *ret->matrix = (*self->matrix) - scalar;

    return ret;
}

/**
 * psy_matrix4_sub:
 * @v1: an `PsyMatrix4` from which @scalar is subtracted
 * @v2: The matrix that is subracted from @self
 *
 * Performs matrix subtraction, the elements from @v2 are
 * subtracted from every in @v1.
 *
 * Returns:(transfer full): A newly inialized `PsyMatix4`
 *         that is the result of v1 - v2.
 */
PsyMatrix4*
psy_matrix4_sub(PsyMatrix4* v1, PsyMatrix4 *v2)
{
    g_return_val_if_fail(PSY_IS_MATRIX4(v1), NULL);
    g_return_val_if_fail(PSY_IS_MATRIX4(v2), NULL);

    PsyMatrix4* ret = PSY_MATRIX4(psy_matrix4_new());

    *ret->matrix = (*v1->matrix) - (*v2->matrix);
    return ret;
}

/**
 * psy_matrix4_mul_s:
 * @self: an `PsyMatrix4` in which each element is scaled by s
 * @scalar: The scalar that used to scale the elements in @self
 *
 * Performs matrix scalar multiplication, the elements from
 * every @self are scaled by @scalar
 *
 * Returns:(transfer full): A newly inialized `PsyMatix4`
 *         that is the result of self * scalar.
 */
PsyMatrix4*
psy_matrix4_mul_s(PsyMatrix4* self, gdouble scalar)
{
    g_return_val_if_fail(PSY_IS_MATRIX4(self), NULL);
    PsyMatrix4* ret = psy_matrix4_new();

    *ret->matrix = (*self->matrix) * scalar;

    return ret;
}

/**
 * psy_matrix4_mul:
 * @self: an `PsyMatrix4` in which each element is scaled by s
 * @scalar: The scalar that used to scale the elements in @self
 *
 * Performs matrix multiplication.
 *
 * Returns:(transfer full): A newly inialized `PsyMatix4`
 *         that is the result of self * scalar.
 */
PsyMatrix4*
psy_matrix4_mul(PsyMatrix4* v1, PsyMatrix4* v2)
{
    g_return_val_if_fail(PSY_IS_MATRIX4(v1), NULL);
    g_return_val_if_fail(PSY_IS_MATRIX4(v2), NULL);

    PsyMatrix4* ret = psy_matrix4_new();

    *ret->matrix = (*v1->matrix) - (*v2->matrix);
    return ret;
}

/**
 * psy_matrix4_equals:
 * @v1:
 * @v2:
 *
 * Checks whether both matrices are equal
 *
 * Returns: TRUE or FALSE
 */
gboolean
psy_matrix4_equals(PsyMatrix4* v1, PsyMatrix4* v2)
{
    g_return_val_if_fail(PSY_IS_MATRIX4(v1), FALSE);
    g_return_val_if_fail(PSY_IS_MATRIX4(v2), FALSE);

    if (v1 == v2)
        return TRUE;

    return  *v1->matrix == *v2->matrix;
}

/**
 * psy_matrix4_not_equals:
 * @v1:
 * @v2:
 *
 * Checks whether both matrices are NOT equal
 *
 * Returns: TRUE or FALSE
 */
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
