
#ifndef PSY_MATRIX4_H
#define PSY_MATRIX4_H

#include <glib-object.h>
#include <gio/gio.h>

G_BEGIN_DECLS

#define PSY_TYPE_MATRIX4 psy_matrix4_get_type()
G_DECLARE_FINAL_TYPE(PsyMatrix4, psy_matrix4, PSY, MATRIX4, GObject)



PsyMatrix4* psy_matrix4_new(void);

PsyMatrix4* psy_matrix4_new_ortographic(
        gdouble left,
        gdouble right,
        gdouble bottom,
        gdouble top,
        gdouble z_near,
        gdouble z_far
        );

PsyMatrix4* psy_matrix4_new_perspective(
        gdouble fovy,
        gdouble aspect,
        gdouble near,
        gdouble far
        );

void psy_matrix4_destroy(PsyMatrix4* v);

void psy_matrix4_set_identity(PsyMatrix4* self);
gboolean psy_matrix4_is_identity(PsyMatrix4* self);

void psy_matrix4_set_null(PsyMatrix4* self);
gboolean psy_matrix4_is_null(PsyMatrix4* self);

PsyMatrix4* psy_matrix4_add_s(PsyMatrix4* self, gdouble scalar);
PsyMatrix4* psy_matrix4_add(PsyMatrix4* self, PsyMatrix4* other);

PsyMatrix4* psy_matrix4_sub_s(PsyMatrix4* self, gdouble scalar);
PsyMatrix4* psy_matrix4_sub(PsyMatrix4* self, PsyMatrix4* other);

PsyMatrix4* psy_matrix4_mul_s(PsyMatrix4* self, gdouble scalar);
PsyMatrix4* psy_matrix4_mul(PsyMatrix4* self, PsyMatrix4* other);

gboolean psy_matrix4_equals(PsyMatrix4* self, PsyMatrix4* other);
gboolean psy_matrix4_not_equals(PsyMatrix4* self, PsyMatrix4* other);

const gdouble* psy_matrix4_ptr(PsyMatrix4* self);

G_MODULE_EXPORT void
psy_matrix4_get_elements(PsyMatrix4* self, gfloat* elements);


G_END_DECLS

#endif
