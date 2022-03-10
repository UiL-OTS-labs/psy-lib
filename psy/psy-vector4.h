
#ifndef PSY_VECTOR4_H
#define PSY_VECTOR4_H

#include <glib-object.h>

G_BEGIN_DECLS

#define PSY_TYPE_VECTOR4 psy_vector4_get_type()
G_DECLARE_FINAL_TYPE(PsyVector4, psy_vector4, PSY, VECTOR4, GObject)

PsyVector4* psy_vector4_new(void);
PsyVector4* psy_vector4_new_data(gsize n, gdouble* values);

void psy_vector4_destroy(PsyVector4* v);

gdouble psy_vector4_get_magnitude(PsyVector4* self);

PsyVector4* psy_vector4_normalize(PsyVector4* self);
PsyVector4* psy_vector4_negate(PsyVector4* self);

void psy_vector4_set_null(PsyVector4* self);
gboolean psy_vector4_is_null(PsyVector4* self);

PsyVector4* psy_vector4_add_s(PsyVector4* v1, gdouble s);
PsyVector4* psy_vector4_add(PsyVector4* v1, PsyVector4* v2);

PsyVector4* psy_vector4_sub_s(PsyVector4* v1, gdouble s);
PsyVector4* psy_vector4_sub(PsyVector4* v1, PsyVector4* v2);

PsyVector4* psy_vector4_mul_s(PsyVector4* v1, gdouble s);
PsyVector4* psy_vector4_mul(PsyVector4* v1, PsyVector4* v2);
gdouble psy_vector4_dot(PsyVector4* v1, PsyVector4* v2);

gboolean psy_vector4_equals(PsyVector4* v1, PsyVector4* v2);
gboolean psy_vector4_not_equals(PsyVector4* v1, PsyVector4* v2);

const gdouble* psy_vector4_ptr(PsyVector4* self);

gchar* psy_vector4_as_string(PsyVector4* self);


G_END_DECLS

#endif
