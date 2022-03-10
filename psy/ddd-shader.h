#ifndef DDD_SHADER_H
#define DDD_SHADER_H

#include <glib-object.h>
#include <gio/gio.h>

G_BEGIN_DECLS

#define DDD_TYPE_SHADER ddd_shader_get_type()
G_DECLARE_DERIVABLE_TYPE(DddShader, ddd_shader, DDD, SHADER, GObject)


typedef struct _DddShaderClass {
    GObjectClass parent_class;

    void     (*compile)     (DddShader *shader, GError** error);
    gboolean (*is_compiled) (DddShader *shader);

} DddShaderClass;


G_MODULE_EXPORT void
ddd_shader_set_source(DddShader* shader, const gchar* source);

G_MODULE_EXPORT const gchar* 
ddd_shader_get_source(DddShader* shader);

G_MODULE_EXPORT void
ddd_shader_source_from_file (DddShader    *shader,
                             GFile        *shader_file,
                             GError      **error);

G_MODULE_EXPORT void
ddd_shader_source_from_path(DddShader     *shader,
                            const gchar   *shader_path,
                            GError       **error);

G_MODULE_EXPORT void
ddd_shader_compile(DddShader *self, GError **error);

G_MODULE_EXPORT gboolean
ddd_shader_is_compiled(DddShader* self);

G_END_DECLS

#endif
