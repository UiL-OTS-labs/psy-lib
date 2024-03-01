#ifndef PSY_SHADER_PROGRAM_H
#define PSY_SHADER_PROGRAM_H

#include "gmodule.h"
#include "psy-matrix4.h"
#include "psy-shader.h"
#include <gio/gio.h>

G_BEGIN_DECLS

#define PSY_TYPE_SHADER_PROGRAM psy_shader_program_get_type()
G_DECLARE_DERIVABLE_TYPE(
    PsyShaderProgram, psy_shader_program, PSY, SHADER_PROGRAM, GObject)

typedef struct _PsyShaderProgramClass {
    GObjectClass parent_class;

    void (*set_vertex_shader)(PsyShaderProgram *self,
                              PsyShader        *shader,
                              GError          **error);

    void (*set_vertex_shader_source)(PsyShaderProgram *self,
                                     const gchar      *source,
                                     GError          **error);

    void (*set_vertex_shader_from_file)(PsyShaderProgram *self,
                                        GFile            *shader_file,
                                        GError          **error);

    void (*set_vertex_shader_from_path)(PsyShaderProgram *self,
                                        const gchar      *shader_path,
                                        GError          **error);

    void (*set_fragment_shader)(PsyShaderProgram *self,
                                PsyShader        *shader,
                                GError          **error);

    void (*set_fragment_shader_source)(PsyShaderProgram *self,
                                       const gchar      *source,
                                       GError          **error);

    void (*set_fragment_shader_from_file)(PsyShaderProgram *self,
                                          GFile            *shader_file,
                                          GError          **error);

    void (*set_fragment_shader_from_path)(PsyShaderProgram *self,
                                          const gchar      *shader,
                                          GError          **error);

    void (*link)(PsyShaderProgram *self, GError **error);

    gboolean (*is_linked)(PsyShaderProgram *self);

    void (*use_program)(PsyShaderProgram *self, GError **error);

    void (*set_uniform_matrix4)(PsyShaderProgram *self,
                                const gchar      *name,
                                PsyMatrix4       *matrix,
                                GError          **error);

    void (*set_uniform_4f)(PsyShaderProgram *self,
                           const gchar      *name,
                           gfloat           *values,
                           GError          **error);

} PsyShaderProgramClass;

G_MODULE_EXPORT void
psy_shader_program_set_vertex_shader(PsyShaderProgram *self,
                                     PsyShader        *shader,
                                     GError          **error);

G_MODULE_EXPORT PsyShader *
psy_shader_program_get_vertex_shader(PsyShaderProgram *self);

G_MODULE_EXPORT void
psy_shader_program_set_vertex_shader_source(PsyShaderProgram *self,
                                            const gchar      *source,
                                            GError          **error);

G_MODULE_EXPORT void
psy_shader_program_set_vertex_shader_from_file(PsyShaderProgram *self,
                                               GFile            *shader_file,
                                               GError          **error);

G_MODULE_EXPORT void
psy_shader_program_set_vertex_shader_from_path(PsyShaderProgram *self,
                                               const gchar      *shader_path,
                                               GError          **error);

G_MODULE_EXPORT void
psy_shader_program_set_fragment_shader(PsyShaderProgram *self,
                                       PsyShader        *shader,
                                       GError          **error);

G_MODULE_EXPORT PsyShader *
psy_shader_program_get_fragment_shader(PsyShaderProgram *self);

G_MODULE_EXPORT void
psy_shader_program_set_fragment_shader_source(PsyShaderProgram *self,
                                              const gchar      *source,
                                              GError          **error);

G_MODULE_EXPORT void
psy_shader_program_set_fragment_shader_from_file(PsyShaderProgram *self,
                                                 GFile            *shader_file,
                                                 GError          **error);

G_MODULE_EXPORT void
psy_shader_program_set_fragment_shader_from_path(PsyShaderProgram *self,
                                                 const gchar      *shader_path,
                                                 GError          **error);

G_MODULE_EXPORT void
psy_shader_program_link(PsyShaderProgram *self, GError **error);

G_MODULE_EXPORT gboolean
psy_shader_program_is_linked(PsyShaderProgram *self);

G_MODULE_EXPORT void
psy_shader_program_use(PsyShaderProgram *self, GError **error);

G_MODULE_EXPORT void
psy_shader_program_set_uniform_matrix4(PsyShaderProgram *self,
                                       const gchar      *name,
                                       PsyMatrix4       *matrix,
                                       GError          **error);

G_MODULE_EXPORT void
psy_shader_program_set_uniform_4f(PsyShaderProgram *self,
                                  const gchar      *name,
                                  gfloat           *values,
                                  GError          **error);

G_END_DECLS

#endif
