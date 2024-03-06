#ifndef PSY_VECTOR_H
#define PSY_VECTOR_H

#include <gio/gio.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define PSY_TYPE_VECTOR psy_vector_get_type()

G_MODULE_EXPORT
G_DECLARE_FINAL_TYPE(PsyVector, psy_vector, PSY, VECTOR, GObject)

typedef struct _PsyVector PsyVector;

struct _PsyVector {
    GObject parent_instance;
    gfloat  array[3];
};

G_MODULE_EXPORT PsyVector *
psy_vector_new(void);

G_MODULE_EXPORT PsyVector *
psy_vector_new_x(gfloat x);

G_MODULE_EXPORT PsyVector *
psy_vector_new_xy(gfloat x, gfloat y);

G_MODULE_EXPORT PsyVector *
psy_vector_new_xyz(gfloat x, gfloat y, gfloat z);

G_MODULE_EXPORT void
psy_vector_free(PsyVector *self);

G_MODULE_EXPORT gfloat
psy_vector_magnitude(PsyVector *self);

G_MODULE_EXPORT PsyVector *
psy_vector_unit(PsyVector *self);

G_MODULE_EXPORT PsyVector *
psy_vector_negate(PsyVector *self);

G_MODULE_EXPORT PsyVector *
psy_vector_add_s(PsyVector *self, gfloat scalar);

G_MODULE_EXPORT PsyVector *
psy_vector_add(PsyVector *self, PsyVector *other);

G_MODULE_EXPORT PsyVector *
psy_vector_sub_s(PsyVector *self, gfloat scalar);

G_MODULE_EXPORT PsyVector *
psy_vector_sub(PsyVector *self, PsyVector *other);

G_MODULE_EXPORT PsyVector *
psy_vector_mul_s(PsyVector *self, gfloat scalar);

G_MODULE_EXPORT gfloat
psy_vector_dot(PsyVector *self, PsyVector *other);

G_MODULE_EXPORT PsyVector *
psy_vector_cross(PsyVector *self, PsyVector *other);

G_MODULE_EXPORT gboolean
psy_vector_equals(PsyVector *self, PsyVector *other);

G_MODULE_EXPORT gboolean
psy_vector_not_equals(PsyVector *self, PsyVector *other);

G_END_DECLS

#endif
