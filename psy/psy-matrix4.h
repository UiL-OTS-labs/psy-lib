
#ifndef PSY_MATRIX4_H
#define PSY_MATRIX4_H

#include <gio/gio.h>
#include <glib-object.h>

#include "psy-vector3.h"
#include "psy-vector4.h"

G_BEGIN_DECLS

#define PSY_TYPE_MATRIX4 psy_matrix4_get_type()
G_DECLARE_FINAL_TYPE(PsyMatrix4, psy_matrix4, PSY, MATRIX4, GObject)

G_MODULE_EXPORT PsyMatrix4 *
psy_matrix4_new(void);

G_MODULE_EXPORT PsyMatrix4 *
psy_matrix4_new_identity(void);

G_MODULE_EXPORT PsyMatrix4 *
psy_matrix4_new_ortographic(gfloat left,
                            gfloat right,
                            gfloat bottom,
                            gfloat top,
                            gfloat z_near,
                            gfloat z_far);

G_MODULE_EXPORT PsyMatrix4 *
psy_matrix4_new_perspective(gfloat fovy,
                            gfloat aspect,
                            gfloat near,
                            gfloat far);

G_MODULE_EXPORT void
psy_matrix4_destroy(PsyMatrix4 *v);

G_MODULE_EXPORT void
psy_matrix4_set_identity(PsyMatrix4 *self);

G_MODULE_EXPORT gboolean
psy_matrix4_is_identity(PsyMatrix4 *self);

G_MODULE_EXPORT void
psy_matrix4_set_null(PsyMatrix4 *self);
G_MODULE_EXPORT gboolean
psy_matrix4_is_null(PsyMatrix4 *self);

G_MODULE_EXPORT PsyMatrix4 *
psy_matrix4_add_s(PsyMatrix4 *self, gfloat scalar);
G_MODULE_EXPORT PsyMatrix4 *
psy_matrix4_add(PsyMatrix4 *self, PsyMatrix4 *other);

G_MODULE_EXPORT PsyMatrix4 *
psy_matrix4_sub_s(PsyMatrix4 *self, gfloat scalar);
G_MODULE_EXPORT PsyMatrix4 *
psy_matrix4_sub(PsyMatrix4 *self, PsyMatrix4 *other);

G_MODULE_EXPORT PsyMatrix4 *
psy_matrix4_mul_s(PsyMatrix4 *self, gfloat scalar);
G_MODULE_EXPORT PsyMatrix4 *
psy_matrix4_mul(PsyMatrix4 *self, PsyMatrix4 *other);

G_MODULE_EXPORT PsyVector4 *
psy_matrix4_mul_vec4(PsyMatrix4 *self, PsyVector4 *rhs);

G_MODULE_EXPORT gboolean
psy_matrix4_equals(PsyMatrix4 *self, PsyMatrix4 *other);
G_MODULE_EXPORT gboolean
psy_matrix4_not_equals(PsyMatrix4 *self, PsyMatrix4 *other);

G_MODULE_EXPORT gchar *
psy_matrix4_as_string(PsyMatrix4 *self);

G_MODULE_EXPORT const gfloat *
psy_matrix4_ptr(PsyMatrix4 *self);

G_MODULE_EXPORT void
psy_matrix4_get_elements(PsyMatrix4 *self, gfloat *elements);

G_MODULE_EXPORT void
psy_matrix4_rotate(PsyMatrix4 *self, gfloat degrees, PsyVector3 *axis);

G_MODULE_EXPORT void
psy_matrix4_scale(PsyMatrix4 *self, PsyVector3 *vector);

G_MODULE_EXPORT void
psy_matrix4_translate(PsyMatrix4 *self, PsyVector3 *vector);

G_MODULE_EXPORT PsyMatrix4 *
psy_matrix4_inverse(PsyMatrix4 *self);

G_END_DECLS

#endif
