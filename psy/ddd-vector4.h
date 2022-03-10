
#ifndef DDD_VECTOR4_H
#define DDD_VECTOR4_H

#include <glib-object.h>

G_BEGIN_DECLS

#define DDD_TYPE_VECTOR4 ddd_vector4_get_type()
G_DECLARE_FINAL_TYPE(DddVector4, ddd_vector4, DDD, VECTOR4, GObject)

DddVector4* ddd_vector4_new(void);
DddVector4* ddd_vector4_new_data(gsize n, gdouble* values);

void ddd_vector4_destroy(DddVector4* v);

gdouble ddd_vector4_get_magnitude(DddVector4* self);

DddVector4* ddd_vector4_normalize(DddVector4* self);
DddVector4* ddd_vector4_negate(DddVector4* self);

void ddd_vector4_set_null(DddVector4* self);
gboolean ddd_vector4_is_null(DddVector4* self);

DddVector4* ddd_vector4_add_s(DddVector4* v1, gdouble s);
DddVector4* ddd_vector4_add(DddVector4* v1, DddVector4* v2);

DddVector4* ddd_vector4_sub_s(DddVector4* v1, gdouble s);
DddVector4* ddd_vector4_sub(DddVector4* v1, DddVector4* v2);

DddVector4* ddd_vector4_mul_s(DddVector4* v1, gdouble s);
DddVector4* ddd_vector4_mul(DddVector4* v1, DddVector4* v2);
gdouble ddd_vector4_dot(DddVector4* v1, DddVector4* v2);

gboolean ddd_vector4_equals(DddVector4* v1, DddVector4* v2);
gboolean ddd_vector4_not_equals(DddVector4* v1, DddVector4* v2);

const gdouble* ddd_vector4_ptr(DddVector4* self);

gchar* ddd_vector4_as_string(DddVector4* self);


G_END_DECLS

#endif
