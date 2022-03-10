#ifndef DDD_PROGRAM_H
#define DDD_PROGRAM_H

#include "ddd-shader.h"
#include <gio/gio.h>

G_BEGIN_DECLS

#define DDD_TYPE_PROGRAM ddd_program_get_type()
G_DECLARE_DERIVABLE_TYPE(DddProgram, ddd_program, DDD, PROGRAM, GObject)


typedef struct _DddProgramClass {
    GObjectClass parent_class;

    void (*set_vertex_shader)(DddProgram *program,
                              DddShader  *shader,
                              GError    **error);

    void (*set_vertex_shader_source)(DddProgram  *program,
                                     const gchar *source,
                                     GError     **error);

    void (*set_vertex_shader_from_file)(DddProgram *program,
                                        GFile      *shader_file,
                                        GError    **error);
    
    void (*set_vertex_shader_from_path)(DddProgram   *program,
                                        const gchar  *shader_path,
                                        GError      **error);

    void (*set_fragment_shader)(DddProgram *program,
                                DddShader  *shader,
                                GError    **error);

    void (*set_fragment_shader_source)(DddProgram   *program,
                                       const gchar  *source,
                                       GError      **error);

    void (*set_fragment_shader_from_file)(DddProgram *program,
                                          GFile      *shader_file,
                                          GError    **error);
    
    void (*set_fragment_shader_from_path)(DddProgram   *program,
                                          const gchar  *shader,
                                          GError      **error);
    
    void (*link)          (DddProgram *program, GError** error);
    
    gboolean (*is_linked) (DddProgram *program);

    void (*use_program) (DddProgram* program, GError ** error);

} DddProgramClass;



G_MODULE_EXPORT void
ddd_program_set_vertex_shader(DddProgram       *program,
                              DddShader        *shader,
                              GError          **error);
G_MODULE_EXPORT void
ddd_program_set_vertex_shader_source(DddProgram       *program,
                                     const gchar      *source,
                                     GError          **error);

G_MODULE_EXPORT void
ddd_program_set_vertex_shader_from_file(DddProgram    *program,
                                        GFile         *shader_file,
                                        GError       **error);

G_MODULE_EXPORT void
ddd_program_set_vertex_shader_from_path(DddProgram    *program,
                                        const gchar   *shader_path,
                                        GError       **error);

G_MODULE_EXPORT void
ddd_program_set_fragment_shader(DddProgram       *program,
                                DddShader        *shader,
                                GError          **error);
G_MODULE_EXPORT void
ddd_program_set_fragment_shader_source(DddProgram       *program,
                                       const gchar      *source,
                                       GError          **error);

G_MODULE_EXPORT void
ddd_program_set_fragment_shader_from_file (DddProgram   *program,
                                           GFile        *shader_file,
                                           GError      **error);

G_MODULE_EXPORT void
ddd_program_set_fragment_shader_from_path(DddProgram    *program,
                                          const gchar   *shader_path,
                                          GError       **error);

G_MODULE_EXPORT void
ddd_program_link(DddProgram *self, GError **error);

G_MODULE_EXPORT gboolean
ddd_program_is_linked(DddProgram *self);

G_MODULE_EXPORT void
ddd_program_use(DddProgram* self, GError **error);

G_END_DECLS

#endif
