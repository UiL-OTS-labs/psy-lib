
#include <glm/detail/type_mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "psy-matrix4.h"
#include "psy-vector3-private.h"
#include "psy-vector4-private.h"

/**
 * PsyMatrix4:
 *
 * This is a matrix class in order to perform simple graphical transformations.
 * The matrix may be used stand alone, but it is designed to be uploaded to
 * a shader uniform. Hence, it is stored in column major order. This matches
 * the storage format of GLSL matrices. This GObject is a wrapper around libglm,
 * which is designed to be relatively similar to GLSL shader matrices.
 */

typedef struct _PsyMatrix4 PsyMatrix4;

struct _PsyMatrix4 {
    GObject      parent_instance;
    glm::mat4x4 *matrix;
};

G_DEFINE_TYPE(PsyMatrix4, psy_matrix4, G_TYPE_OBJECT)

typedef enum {
    PROP_NULL,
    IS_IDENTITY,
    IS_NULL,
    N_PROPERTIES
} PsyMatrix4Property;

static GParamSpec *obj_properties[N_PROPERTIES] = {
    NULL,
};

static void
psy_matrix4_init(PsyMatrix4 *self)
{
    self->matrix = new glm::mat4x4();
}

static void
psy_matrix4_finalize(GObject *obj)
{
    PsyMatrix4 *self = PSY_MATRIX4(obj);
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

    switch ((PsyMatrix4Property) prop_id) {
    case IS_IDENTITY:
    {
        gboolean identity = g_value_get_boolean(value);
        if (identity)
            psy_matrix4_set_identity(matrix4);
    } break;
    case IS_NULL:
    {
        gboolean isnull = g_value_get_boolean(value);
        if (isnull)
            psy_matrix4_set_null(matrix4);
    } break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void
psy_matrix4_get_property(GObject    *object,
                         guint       prop_id,
                         GValue     *value,
                         GParamSpec *pspec)
{
    PsyMatrix4 *matrix4 = PSY_MATRIX4(object);

    switch ((PsyMatrix4Property) prop_id) {
    case IS_IDENTITY:
        g_value_set_boolean(value, psy_matrix4_is_identity(matrix4));
        break;
    case IS_NULL:
        g_value_set_boolean(value, psy_matrix4_is_null(matrix4));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void
psy_matrix4_class_init(PsyMatrix4Class *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
    (void) gobject_class;

    gobject_class->set_property = psy_matrix4_set_property;
    gobject_class->get_property = psy_matrix4_get_property;
    gobject_class->finalize     = psy_matrix4_finalize;

    obj_properties[IS_IDENTITY]
        = g_param_spec_boolean("is-identity",
                               "is_idendity",
                               "Test whether the matrix is an identity matrix "
                               "or make it an identity matrix",
                               FALSE,
                               G_PARAM_READWRITE);

    obj_properties[IS_NULL]
        = g_param_spec_boolean("is-null",
                               "is_null",
                               "Test whether or set all element to 0.",
                               FALSE,
                               G_PARAM_READWRITE);

    g_object_class_install_properties(
        gobject_class, N_PROPERTIES, obj_properties);
}

/* ************* public functions ************* */

/**
 * psy_matrix4_new:(constructor)
 *
 * Creates a new `PsyMatrix4`
 *
 * This constructs default 4 dimensional matrix with
 * all values set to 0.0
 *
 * Returns: a new `PsyMatrix4`
 */
PsyMatrix4 *
psy_matrix4_new()
{
    return PSY_MATRIX4(g_object_new(PSY_TYPE_MATRIX4, NULL));
}

/**
 * psy_matrixt_new_identity:(constructor)
 *
 * Creates a new `PsyMatrix4`
 *
 * The returned vector is an identity Matrix.
 *
 * Returns: a new `PsyMatrix4`
 */
PsyMatrix4 *
psy_matrix4_new_identity()
{
    return PSY_MATRIX4(
        g_object_new(PSY_TYPE_MATRIX4, "is-identity", TRUE, NULL));
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
PsyMatrix4 *
psy_matrix4_new_ortographic(gfloat left,
                            gfloat right,
                            gfloat bottom,
                            gfloat top,
                            gfloat z_near,
                            gfloat z_far)
{
    PsyMatrix4 *ret = psy_matrix4_new();

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
PsyMatrix4 *
psy_matrix4_new_perspective(gfloat fovy,
                            gfloat aspect,
                            gfloat z_near,
                            gfloat z_far)
{
    PsyMatrix4 *ret = psy_matrix4_new();

    *ret->matrix = glm::perspective(fovy, aspect, z_near, z_far);

    return ret;
}

/**
 * psy_matrix4_free:
 *
 * Destroys a `PsyMatrix4` instance
 *
 * self: the matrix to destroy
 */
void
psy_matrix4_free(PsyMatrix4 *self)
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
psy_matrix4_set_identity(PsyMatrix4 *self)
{
    g_return_if_fail(PSY_IS_MATRIX4(self));

    *self->matrix = glm::mat4x4(1);
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
psy_matrix4_is_identity(PsyMatrix4 *self)
{
    g_return_val_if_fail(PSY_IS_MATRIX4(self), FALSE);

    return *self->matrix == glm::mat4x4(1);
}

/**
 * psy_matrix4_set_null:
 *
 * Resets all members of the matrix to 0.
 *
 * self: Reset the matrix to be a null matrix.
 */
void
psy_matrix4_set_null(PsyMatrix4 *self)
{
    g_return_if_fail(PSY_IS_MATRIX4(self));

    *self->matrix = glm::mat4x4(0);
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
psy_matrix4_is_null(PsyMatrix4 *self)
{
    g_return_val_if_fail(PSY_IS_MATRIX4(self), FALSE);

    return *self->matrix == glm::mat4x4(0);
}

/**
 * psy_matrix4_add_s:
 * @self : the matrix to which @scalar is added
 * @scalar: the scalar to add to @self
 *
 * Returns :(transfer full): The result of self + s
 */
PsyMatrix4 *
psy_matrix4_add_s(PsyMatrix4 *self, gfloat scalar)
{
    g_return_val_if_fail(PSY_IS_MATRIX4(self), NULL);

    PsyMatrix4 *ret = PSY_MATRIX4(psy_matrix4_new());

    *ret->matrix = (*self->matrix) + scalar;

    return ret;
}

/**
 * psy_matrix4_add:
 * @self : the matrix to which @other is added
 * @other: the matrix which to add to @self
 *
 * Returns :(transfer full): The result of self + other
 */
PsyMatrix4 *
psy_matrix4_add(PsyMatrix4 *self, PsyMatrix4 *other)
{
    g_return_val_if_fail(PSY_IS_MATRIX4(other), NULL);
    g_return_val_if_fail(PSY_IS_MATRIX4(other), NULL);

    PsyMatrix4 *ret = PSY_MATRIX4(psy_matrix4_new());

    *ret->matrix = (*self->matrix) + (*other->matrix);

    return ret;
}

/**
 * psy_matrix4_sub_s:
 * @self: an `PsyMatrix4` from which @scalar is subtracted
 * @scalar: The scalar that is subracted from @self
 *
 * Performs scalar subtraction, the scalar is subtracted from
 * every element in @self.
 *
 * Returns:(transfer full): A newly initialized `PsyMatix4`
 *         that is the result of self - scalar.
 */
PsyMatrix4 *
psy_matrix4_sub_s(PsyMatrix4 *self, gfloat scalar)
{
    g_return_val_if_fail(PSY_IS_MATRIX4(self), NULL);

    PsyMatrix4 *ret = PSY_MATRIX4(psy_matrix4_new());

    *ret->matrix = (*self->matrix) - scalar;

    return ret;
}

/**
 * psy_matrix4_sub:
 * @self: an `PsyMatrix4` from which @other is subtracted
 * @other: The matrix that is subracted from @self
 *
 * Performs matrix subtraction, the elements from @other are
 * subtracted from every in @self.
 *
 * Returns:(transfer full): A newly inialized `PsyMatix4`
 *         that is the result of self - other.
 */
PsyMatrix4 *
psy_matrix4_sub(PsyMatrix4 *self, PsyMatrix4 *other)
{
    g_return_val_if_fail(PSY_IS_MATRIX4(self), NULL);
    g_return_val_if_fail(PSY_IS_MATRIX4(other), NULL);

    PsyMatrix4 *ret = PSY_MATRIX4(psy_matrix4_new());

    *ret->matrix = (*self->matrix) - (*other->matrix);
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
PsyMatrix4 *
psy_matrix4_mul_s(PsyMatrix4 *self, gfloat scalar)
{
    g_return_val_if_fail(PSY_IS_MATRIX4(self), NULL);
    PsyMatrix4 *ret = psy_matrix4_new();

    *ret->matrix = (*self->matrix) * scalar;

    return ret;
}

/**
 * psy_matrix4_mul:
 * @self: an `PsyMatrix4`
 * @other: another `PsyMatrix4`
 *
 * Performs matrix multiplication.
 *
 * Returns:(transfer full): A newly inialized `PsyMatix4`
 *         that is the result of self * other.
 */
PsyMatrix4 *
psy_matrix4_mul(PsyMatrix4 *self, PsyMatrix4 *other)
{
    g_return_val_if_fail(PSY_IS_MATRIX4(self), NULL);
    g_return_val_if_fail(PSY_IS_MATRIX4(other), NULL);

    PsyMatrix4 *ret = psy_matrix4_new();

    *ret->matrix = (*self->matrix) * (*other->matrix);
    return ret;
}

/**
 * psy_matrix4_mul_vec4:
 * @self: an instance of [class@Matrix4]
 * @rhs: an instance pf [class@Vector4] to multiply with
 *
 * Performs matrix vector multiplication
 *
 * Returns:(transfer full): A newly inialized [class@Psy.Vector4]
 *         that is the result of self * rhs.
 */
PsyVector4 *
psy_matrix4_mul_vec4(PsyMatrix4 *self, PsyVector4 *rhs)
{
    g_return_val_if_fail(PSY_IS_MATRIX4(self), NULL);
    g_return_val_if_fail(PSY_IS_MATRIX4(self), NULL);

    PsyVector4 *ret = psy_vector4_new();
    psy_vector4_get_priv_reference(ret)
        = *self->matrix * psy_vector4_get_priv_reference(rhs);

    return ret;
}

/**
 * psy_matrix4_equals:
 * @self: a `PsyMatrix4` instant to compare with @other
 * @other: another `PsyMatrix4`
 *
 * Checks whether both matrices are equal
 *
 * Returns: TRUE or FALSE
 */
gboolean
psy_matrix4_equals(PsyMatrix4 *self, PsyMatrix4 *other)
{
    g_return_val_if_fail(PSY_IS_MATRIX4(self), FALSE);
    g_return_val_if_fail(PSY_IS_MATRIX4(other), FALSE);

    if (self == other)
        return TRUE;

    return *self->matrix == *other->matrix;
}

/**
 * psy_matrix4_not_equals:
 * @self: a `PsyMatrix4` instant to compare with @other
 * @other: another `PsyMatrix4`
 *
 * Checks whether both matrices are NOT equal
 *
 * Returns: TRUE or FALSE
 */
gboolean
psy_matrix4_not_equals(PsyMatrix4 *self, PsyMatrix4 *other)
{
    return !psy_matrix4_equals(self, other);
}

/**
 * psy_matrix4_as_string:
 * @self: an instance of PsyMatrix4
 *
 * Gives a string representation of the  matrix.
 *
 * Returns:(transfer full): The matrix in string form.
 */
gchar *
psy_matrix4_as_string(PsyMatrix4 *self)
{
    gchar result[1024];
    g_return_val_if_fail(PSY_IS_MATRIX4(self), NULL);
    const auto& mat = *self->matrix;
    g_snprintf(result,
               sizeof(result),
               "| %8.4g %8.4g %8.4g %8.4g |\n"
               "| %8.4g %8.4g %8.4g %8.4g |\n"
               "| %8.4g %8.4g %8.4g %8.4g |\n"
               "| %8.4g %8.4g %8.4g %8.4g |",
               mat[0][0],
               mat[1][0],
               mat[2][0],
               mat[3][0],
               mat[0][1],
               mat[1][1],
               mat[2][1],
               mat[3][1],
               mat[0][2],
               mat[1][2],
               mat[2][2],
               mat[3][2],
               mat[0][3],
               mat[1][3],
               mat[2][3],
               mat[3][3]);

    return g_strdup(result);
}

/**
 * psy_matrix4_ptr:
 * @self: an instance of PsyMatrix4
 *
 * Returns (transfer none): a pointer to the data.
 */
const gfloat *
psy_matrix4_ptr(PsyMatrix4 *self)
{
    g_return_val_if_fail(PSY_IS_MATRIX4(self), NULL);

    return glm::value_ptr(*self->matrix);
}

/**
 * psy_matrix4_get_elements:
 * @self: an instance of PsyMatrix4
 * @elements:(out caller-allocates) (array fixed-size=16): a pointer to doubles
 *
 * Returns the matrix in elements in column major order.
 */
void
psy_matrix4_get_elements(PsyMatrix4 *self, gfloat *elements)
{
    g_return_if_fail(PSY_IS_MATRIX4(self));
    gfloat       *out = &elements[0];
    const gfloat *in  = glm::value_ptr(*(self->matrix));
    for (; out < &elements[16]; in++, out++)
        *out = (gfloat) *in;
}

/**
 * psy_matrix4_rotate:
 * @self:An instance of `PsyMatrix4`.
 * @degrees: The angle of the rotation in degrees. (that is what GLM uses).
 * @axis: the axis around which to rotate.
 *
 * Rotates @self @degrees around @axis
 */
void
psy_matrix4_rotate(PsyMatrix4 *self, gfloat degrees, PsyVector3 *axis)
{
    g_return_if_fail(PSY_IS_MATRIX4(self) && PSY_IS_VECTOR3(axis));

    const glm::vec3& vec = psy_vector3_get_priv_reference(axis);
    *self->matrix        = glm::rotate(*self->matrix, degrees, vec);
}

/**
 * psy_matrix4_scale:
 * @self:An instance of `PsyMatrix4`.
 * @vector: the vector to use for scaling.
 *
 * Scales @self by using the x, y and z values from @vector
 */
void
psy_matrix4_scale(PsyMatrix4 *self, PsyVector3 *vector)
{
    g_return_if_fail(PSY_IS_MATRIX4(self) && PSY_IS_VECTOR3(vector));

    const glm::vec3& vec = psy_vector3_get_priv_reference(vector);
    *self->matrix        = glm::scale(*self->matrix, vec);
}

/**
 * psy_matrix4_translate:
 * @self:An instance of `PsyMatrix4`.
 * @vector: the vector to use for translation.
 *
 * translates @self by using the x, y and z values from @vector
 */
void
psy_matrix4_translate(PsyMatrix4 *self, PsyVector3 *vector)
{
    g_return_if_fail(PSY_IS_MATRIX4(self) && PSY_IS_VECTOR3(vector));

    const glm::vec3& vec = psy_vector3_get_priv_reference(vector);
    *self->matrix        = glm::translate(*self->matrix, vec);
}

/**
 * psy_matrix4_inverse:
 * @self: An instance of [class@Matrix4]
 *
 * Obtain the inverse matrix of oneself if M is an matrix it returns M^-1.
 *
 * Returns:(transfer full): The inverse of @self.
 */
PsyMatrix4 *
psy_matrix4_inverse(PsyMatrix4 *self)
{
    g_return_val_if_fail(PSY_IS_MATRIX4(self), NULL);

    PsyMatrix4 *ret = psy_matrix4_new();
    *ret->matrix    = glm::inverse(*self->matrix);
    return ret;
}

/*
 * Private functions exported in psy-matrix4-private.h
 */

/**
 * psy_matrix4_get_priv_reference:(skip)
 *
 * Returns: the private implementation of the matrix a glm::mat4
 */
glm::mat4&
psy_matrix4_get_priv_reference(PsyMatrix4 *self)
{
    g_assert(PSY_IS_MATRIX4(self));

    return *self->matrix;
}

/**
 * psy_matrix4_get_priv_pointer:(skip)
 *
 * Returns: the private implementation of the matrix a glm::mat4 as pointer
 */
glm::mat4 *
psy_matrix4_get_priv_pointer(PsyMatrix4 *self)
{
    g_return_val_if_fail(PSY_IS_MATRIX4(self), NULL);

    return self->matrix;
}
