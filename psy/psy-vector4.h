
#ifndef PSY_VECTOR4_H
#define PSY_VECTOR4_H

#include <gio/gio.h>
#include <glib-object.h>

// forward declarations
struct _PsyMatrix4;
typedef struct _PsyMatrix4 PsyMatrix4;

G_BEGIN_DECLS

#define PSY_TYPE_VECTOR4 psy_vector4_get_type()

G_MODULE_EXPORT
G_DECLARE_FINAL_TYPE(PsyVector4, psy_vector4, PSY, VECTOR4, GObject)

G_MODULE_EXPORT PsyVector4 *
psy_vector4_new(void);
G_MODULE_EXPORT PsyVector4 *
psy_vector4_new_data(gsize n, gfloat *values);

G_MODULE_EXPORT void
psy_vector4_free(PsyVector4 *self);

G_MODULE_EXPORT gfloat
psy_vector4_get_magnitude(PsyVector4 *self);

G_MODULE_EXPORT PsyVector4 *
psy_vector4_unit(PsyVector4 *self);
G_MODULE_EXPORT PsyVector4 *
psy_vector4_negate(PsyVector4 *self);

G_MODULE_EXPORT void
psy_vector4_set_null(PsyVector4 *self);
G_MODULE_EXPORT gboolean
psy_vector4_is_null(PsyVector4 *self);

G_MODULE_EXPORT PsyVector4 *
psy_vector4_add_s(PsyVector4 *self, gfloat scalar);
G_MODULE_EXPORT PsyVector4 *
psy_vector4_add(PsyVector4 *self, PsyVector4 *other);

G_MODULE_EXPORT PsyVector4 *
psy_vector4_sub_s(PsyVector4 *self, gfloat scalar);
G_MODULE_EXPORT PsyVector4 *
psy_vector4_sub(PsyVector4 *self, PsyVector4 *other);

G_MODULE_EXPORT PsyVector4 *
psy_vector4_mul_s(PsyVector4 *self, gfloat scalar);
G_MODULE_EXPORT PsyVector4 *
psy_vector4_mul(PsyVector4 *self, PsyVector4 *other);
G_MODULE_EXPORT PsyVector4 *
psy_vector4_mul_matrix4(PsyVector4 *self, PsyMatrix4 *other);
G_MODULE_EXPORT gfloat
psy_vector4_dot(PsyVector4 *self, PsyVector4 *other);

G_MODULE_EXPORT gboolean
psy_vector4_equals(PsyVector4 *self, PsyVector4 *other);
G_MODULE_EXPORT gboolean
psy_vector4_not_equals(PsyVector4 *self, PsyVector4 *other);

G_MODULE_EXPORT G_MODULE_EXPORT const gfloat *
psy_vector4_ptr(PsyVector4 *self);

G_MODULE_EXPORT GArray *
psy_vector4_get_values(PsyVector4 *self);

G_MODULE_EXPORT void
psy_vector4_set_values(PsyVector4 *self, GArray *array);

G_MODULE_EXPORT gchar *
psy_vector4_as_string(PsyVector4 *self);

G_END_DECLS

#endif
