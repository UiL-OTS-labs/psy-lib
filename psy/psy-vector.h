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

void psy_vector_destroy(PsyVector* v);

gfloat psy_vector_magnitude(PsyVector* v);
PsyVector* psy_vector_unit(PsyVector* v);
PsyVector* psy_vector_negate(PsyVector* v);

PsyVector* psy_vector_add_s(PsyVector* v1, gfloat s);
PsyVector* psy_vector_add(PsyVector* v1, PsyVector* v2);

PsyVector* psy_vector_sub_s(PsyVector* v1, gfloat s);
PsyVector* psy_vector_sub(PsyVector* v1, PsyVector* v2);

PsyVector* psy_vector_mul_s(PsyVector* v1, gfloat s);
gfloat psy_vector_dot(PsyVector* v1, PsyVector* v2);
PsyVector* psy_vector_cross(PsyVector* v1, PsyVector* v2);

gboolean psy_vector_equals(PsyVector* v1, PsyVector* v2);
gboolean psy_vector_not_equals(PsyVector* v1, PsyVector* v2);


G_END_DECLS

#endif
