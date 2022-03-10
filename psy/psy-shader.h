#ifndef PSY_SHADER_H
#define PSY_SHADER_H

#include <glib-object.h>
#include <gio/gio.h>

G_BEGIN_DECLS

#define PSY_TYPE_SHADER psy_shader_get_type()
G_DECLARE_DERIVABLE_TYPE(PsyShader, psy_shader, PSY, SHADER, GObject)


typedef struct _PsyShaderClass {
    GObjectClass parent_class;

    void     (*compile)     (PsyShader *shader, GError** error);
    gboolean (*is_compiled) (PsyShader *shader);

} PsyShaderClass;


G_MODULE_EXPORT void
psy_shader_set_source(PsyShader* shader, const gchar* source);

G_MODULE_EXPORT const gchar* 
psy_shader_get_source(PsyShader* shader);

G_MODULE_EXPORT void
psy_shader_source_from_file (PsyShader    *shader,
                             GFile        *shader_file,
                             GError      **error);

G_MODULE_EXPORT void
psy_shader_source_from_path(PsyShader     *shader,
                            const gchar   *shader_path,
                            GError       **error);

G_MODULE_EXPORT void
psy_shader_compile(PsyShader *self, GError **error);

G_MODULE_EXPORT gboolean
psy_shader_is_compiled(PsyShader* self);

G_END_DECLS

#endif
