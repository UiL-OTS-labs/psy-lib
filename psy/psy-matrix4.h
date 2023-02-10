
#ifndef PSY_MATRIX4_H
#define PSY_MATRIX4_H

#include <gio/gio.h>
#include <glib-object.h>

#include "psy-vector4.h"

G_BEGIN_DECLS

#define PSY_TYPE_MATRIX4 psy_matrix4_get_type()
G_DECLARE_FINAL_TYPE(PsyMatrix4, psy_matrix4, PSY, MATRIX4, GObject)

G_MODULE_EXPORT PsyMatrix4 *
psy_matrix4_new(void);

G_MODULE_EXPORT PsyMatrix4 *
psy_matrix4_new_ortographic(gdouble left,
                            gdouble right,
                            gdouble bottom,
                            gdouble top,
                            gdouble z_near,
                            gdouble z_far);

G_MODULE_EXPORT PsyMatrix4 *
psy_matrix4_new_perspective(gdouble fovy,
                            gdouble aspect,
                            gdouble near,
                            gdouble far);

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
psy_matrix4_add_s(PsyMatrix4 *self, gdouble scalar);
G_MODULE_EXPORT PsyMatrix4 *
psy_matrix4_add(PsyMatrix4 *self, PsyMatrix4 *other);

G_MODULE_EXPORT PsyMatrix4 *
psy_matrix4_sub_s(PsyMatrix4 *self, gdouble scalar);
G_MODULE_EXPORT PsyMatrix4 *
psy_matrix4_sub(PsyMatrix4 *self, PsyMatrix4 *other);

G_MODULE_EXPORT PsyMatrix4 *
psy_matrix4_mul_s(PsyMatrix4 *self, gdouble scalar);
G_MODULE_EXPORT PsyMatrix4 *
psy_matrix4_mul(PsyMatrix4 *self, PsyMatrix4 *other);

G_MODULE_EXPORT gboolean
psy_matrix4_equals(PsyMatrix4 *self, PsyMatrix4 *other);
G_MODULE_EXPORT gboolean
psy_matrix4_not_equals(PsyMatrix4 *self, PsyMatrix4 *other);

G_MODULE_EXPORT const gdouble *
psy_matrix4_ptr(PsyMatrix4 *self);

G_MODULE_EXPORT void
psy_matrix4_get_elements(PsyMatrix4 *self, gfloat *elements);

G_MODULE_EXPORT void
psy_matrix4_scale(PsyMatrix4 *self, PsyVector4 *vector);

G_END_DECLS

#endif
