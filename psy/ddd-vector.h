#ifndef DDD_VECTOR_H
#define DDD_VECTOR_H

#include <glib-object.h>

G_BEGIN_DECLS

#define DDD_TYPE_VECTOR ddd_vector_get_type()
G_DECLARE_FINAL_TYPE(DddVector, ddd_vector, DDD, VECTOR, GObject)

typedef struct _DddVector DddVector;

struct _DddVector {
    GObject parent_instance;
    gfloat  array[3];
};

DddVector* ddd_vector_new(void);
DddVector* ddd_vector_new_x(gfloat x);
DddVector* ddd_vector_new_xy(gfloat x, gfloat y);
DddVector* ddd_vector_new_xyz(gfloat x, gfloat y, gfloat z);

void ddd_vector_destroy(DddVector* v);

gfloat ddd_vector_magnitude(DddVector* v);
DddVector* ddd_vector_unit(DddVector* v);
DddVector* ddd_vector_negate(DddVector* v);

DddVector* ddd_vector_add_s(DddVector* v1, gfloat s);
DddVector* ddd_vector_add(DddVector* v1, DddVector* v2);

DddVector* ddd_vector_sub_s(DddVector* v1, gfloat s);
DddVector* ddd_vector_sub(DddVector* v1, DddVector* v2);

DddVector* ddd_vector_mul_s(DddVector* v1, gfloat s);
gfloat ddd_vector_dot(DddVector* v1, DddVector* v2);
DddVector* ddd_vector_cross(DddVector* v1, DddVector* v2);

gboolean ddd_vector_equals(DddVector* v1, DddVector* v2);
gboolean ddd_vector_not_equals(DddVector* v1, DddVector* v2);


G_END_DECLS

#endif
