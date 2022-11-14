#ifndef PSY_VECTOR_H
#define PSY_VECTOR_H

#include <glib-object.h>

G_BEGIN_DECLS

#define PSY_TYPE_VECTOR psy_vector_get_type()
G_DECLARE_FINAL_TYPE(PsyVector, psy_vector, PSY, VECTOR, GObject)

typedef struct _PsyVector PsyVector;

struct _PsyVector {
    GObject parent_instance;
    gfloat  array[3];
};

PsyVector* psy_vector_new(void);
PsyVector* psy_vector_new_x(gfloat x);
PsyVector* psy_vector_new_xy(gfloat x, gfloat y);
PsyVector* psy_vector_new_xyz(gfloat x, gfloat y, gfloat z);

void psy_vector_destroy(PsyVector* self);

gfloat psy_vector_magnitude(PsyVector* self);
PsyVector* psy_vector_unit(PsyVector* self);
PsyVector* psy_vector_negate(PsyVector* self);

PsyVector* psy_vector_add_s(PsyVector* self, gfloat scalar);
PsyVector* psy_vector_add(PsyVector* self, PsyVector* other);

PsyVector* psy_vector_sub_s(PsyVector* self, gfloat scalar);
PsyVector* psy_vector_sub(PsyVector* self, PsyVector* other);

PsyVector* psy_vector_mul_s(PsyVector* self, gfloat scalar);
gfloat psy_vector_dot(PsyVector* self, PsyVector* other);
PsyVector* psy_vector_cross(PsyVector* self, PsyVector* other);

gboolean psy_vector_equals(PsyVector* self, PsyVector* other);
gboolean psy_vector_not_equals(PsyVector* self, PsyVector* other);


G_END_DECLS

#endif
