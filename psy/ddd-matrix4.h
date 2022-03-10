
#ifndef DDD_MATRIX4_H
#define DDD_MATRIX4_H

#include <glib-object.h>

G_BEGIN_DECLS

#define DDD_TYPE_MATRIX4 ddd_matrix4_get_type()
G_DECLARE_FINAL_TYPE(DddMatrix4, ddd_matrix4, DDD, MATRIX4, GObject)



DddMatrix4* ddd_matrix4_new(void);

DddMatrix4* ddd_matrix4_new_ortographic(
        gdouble left,
        gdouble right,
        gdouble bottom,
        gdouble top,
        gdouble z_near,
        gdouble z_far
        );

DddMatrix4* ddd_matrix4_new_perspective(
        gdouble fovy,
        gdouble aspect,
        gdouble near,
        gdouble far
        );

void ddd_matrix4_destroy(DddMatrix4* v);

void ddd_matrix4_set_identity(DddMatrix4* self);
gboolean ddd_matrix4_is_identity(DddMatrix4* self);

void ddd_matrix4_set_null(DddMatrix4* self);
gboolean ddd_matrix4_is_null(DddMatrix4* self);

DddMatrix4* ddd_matrix4_add_s(DddMatrix4* v1, gdouble s);
DddMatrix4* ddd_matrix4_add(DddMatrix4* v1, DddMatrix4* v2);

DddMatrix4* ddd_matrix4_sub_s(DddMatrix4* v1, gdouble s);
DddMatrix4* ddd_matrix4_sub(DddMatrix4* v1, DddMatrix4* v2);

DddMatrix4* ddd_matrix4_mul_s(DddMatrix4* v1, gdouble s);
DddMatrix4* ddd_matrix4_mul(DddMatrix4* v1, DddMatrix4* v2);

gboolean ddd_matrix4_equals(DddMatrix4* v1, DddMatrix4* v2);
gboolean ddd_matrix4_not_equals(DddMatrix4* v1, DddMatrix4* v2);

const gdouble* ddd_matrix4_ptr(DddMatrix4* self);


G_END_DECLS

#endif
