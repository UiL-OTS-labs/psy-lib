#ifndef PSY_PROGRAM_H
#define PSY_PROGRAM_H

#include "gmodule.h"
#include "psy-matrix4.h"
#include "psy-shader.h"
#include <gio/gio.h>

G_BEGIN_DECLS

#define PSY_TYPE_PROGRAM psy_program_get_type()
G_DECLARE_DERIVABLE_TYPE(PsyProgram, psy_program, PSY, PROGRAM, GObject)

typedef struct _PsyProgramClass {
    GObjectClass parent_class;

    void (*set_vertex_shader)(PsyProgram *self,
                              PsyShader  *shader,
                              GError    **error);

    void (*set_vertex_shader_source)(PsyProgram  *self,
                                     const gchar *source,
                                     GError     **error);

    void (*set_vertex_shader_from_file)(PsyProgram *self,
                                        GFile      *shader_file,
                                        GError    **error);

    void (*set_vertex_shader_from_path)(PsyProgram  *self,
                                        const gchar *shader_path,
                                        GError     **error);

    void (*set_fragment_shader)(PsyProgram *self,
                                PsyShader  *shader,
                                GError    **error);

    void (*set_fragment_shader_source)(PsyProgram  *self,
                                       const gchar *source,
                                       GError     **error);

    void (*set_fragment_shader_from_file)(PsyProgram *self,
                                          GFile      *shader_file,
                                          GError    **error);

    void (*set_fragment_shader_from_path)(PsyProgram  *self,
                                          const gchar *shader,
                                          GError     **error);

    void (*link)(PsyProgram *self, GError **error);

    gboolean (*is_linked)(PsyProgram *self);

    void (*use_program)(PsyProgram *self, GError **error);

    void (*set_uniform_matrix4)(PsyProgram  *self,
                                const gchar *name,
                                PsyMatrix4  *matrix,
                                GError     **error);

    void (*set_uniform_4f)(PsyProgram  *self,
                           const gchar *name,
                           gfloat      *values,
                           GError     **error);

} PsyProgramClass;

G_MODULE_EXPORT void
psy_program_set_vertex_shader(PsyProgram *self,
                              PsyShader  *shader,
                              GError    **error);
G_MODULE_EXPORT void
psy_program_set_vertex_shader_source(PsyProgram  *self,
                                     const gchar *source,
                                     GError     **error);

G_MODULE_EXPORT void
psy_program_set_vertex_shader_from_file(PsyProgram *self,
                                        GFile      *shader_file,
                                        GError    **error);

G_MODULE_EXPORT void
psy_program_set_vertex_shader_from_path(PsyProgram  *self,
                                        const gchar *shader_path,
                                        GError     **error);

G_MODULE_EXPORT void
psy_program_set_fragment_shader(PsyProgram *self,
                                PsyShader  *shader,
                                GError    **error);
G_MODULE_EXPORT void
psy_program_set_fragment_shader_source(PsyProgram  *self,
                                       const gchar *source,
                                       GError     **error);

G_MODULE_EXPORT void
psy_program_set_fragment_shader_from_file(PsyProgram *self,
                                          GFile      *shader_file,
                                          GError    **error);

G_MODULE_EXPORT void
psy_program_set_fragment_shader_from_path(PsyProgram  *self,
                                          const gchar *shader_path,
                                          GError     **error);

G_MODULE_EXPORT void
psy_program_link(PsyProgram *self, GError **error);

G_MODULE_EXPORT gboolean
psy_program_is_linked(PsyProgram *self);

G_MODULE_EXPORT void
psy_program_use(PsyProgram *self, GError **error);

G_MODULE_EXPORT void
psy_program_set_uniform_matrix4(PsyProgram  *self,
                                const gchar *name,
                                PsyMatrix4  *matrix,
                                GError     **error);

G_MODULE_EXPORT void
psy_program_set_uniform_4f(PsyProgram  *self,
                           const gchar *name,
                           gfloat      *values,
                           GError     **error);

G_END_DECLS

#endif
