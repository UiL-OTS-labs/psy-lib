
#ifndef PSY_VECTOR3_H
#define PSY_VECTOR3_H

#include <gio/gio.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define PSY_TYPE_VECTOR3 psy_vector3_get_type()
G_DECLARE_FINAL_TYPE(PsyVector3, psy_vector3, PSY, VECTOR3, GObject)

G_MODULE_EXPORT PsyVector3 *
psy_vector3_new(void);
G_MODULE_EXPORT PsyVector3 *
psy_vector3_new_data(gsize n, gfloat *values);

G_MODULE_EXPORT void
psy_vector3_destroy(PsyVector3 *self);

G_MODULE_EXPORT gfloat
psy_vector3_get_magnitude(PsyVector3 *self);

G_MODULE_EXPORT PsyVector3 *
psy_vector3_unit(PsyVector3 *self);
G_MODULE_EXPORT PsyVector3 *
psy_vector3_negate(PsyVector3 *self);

G_MODULE_EXPORT void
psy_vector3_set_null(PsyVector3 *self);
G_MODULE_EXPORT gboolean
psy_vector3_is_null(PsyVector3 *self);

G_MODULE_EXPORT PsyVector3 *
psy_vector3_add_s(PsyVector3 *self, gfloat scalar);
G_MODULE_EXPORT PsyVector3 *
psy_vector3_add(PsyVector3 *self, PsyVector3 *other);

G_MODULE_EXPORT PsyVector3 *
psy_vector3_sub_s(PsyVector3 *self, gfloat scalar);
G_MODULE_EXPORT PsyVector3 *
psy_vector3_sub(PsyVector3 *self, PsyVector3 *other);

G_MODULE_EXPORT PsyVector3 *
psy_vector3_mul_s(PsyVector3 *self, gfloat scalar);
G_MODULE_EXPORT PsyVector3 *
psy_vector3_mul(PsyVector3 *self, PsyVector3 *other);
G_MODULE_EXPORT gfloat
psy_vector3_dot(PsyVector3 *self, PsyVector3 *other);

G_MODULE_EXPORT gboolean
psy_vector3_equals(PsyVector3 *self, PsyVector3 *other);
G_MODULE_EXPORT gboolean
psy_vector3_not_equals(PsyVector3 *self, PsyVector3 *other);

G_MODULE_EXPORT const gfloat *
psy_vector3_ptr(PsyVector3 *self);

G_MODULE_EXPORT GArray *
psy_vector3_get_values(PsyVector3 *self);

G_MODULE_EXPORT void
psy_vector3_set_values(PsyVector3 *self, GArray *array);

G_MODULE_EXPORT gchar *
psy_vector3_as_string(PsyVector3 *self);

G_END_DECLS

#endif
