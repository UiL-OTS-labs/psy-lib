#ifndef PSY_PROGRAM_H
#define PSY_PROGRAM_H

#include "psy-matrix4.h"
#include "psy-shader.h"
#include <gio/gio.h>

G_BEGIN_DECLS

#define PSY_TYPE_PROGRAM psy_program_get_type()
G_DECLARE_DERIVABLE_TYPE(PsyProgram, psy_program, PSY, PROGRAM, GObject)

/**
 * PsyProgramType:
 * @PSY_PROGRAM_UNIFORM_COLOR: a shader program that uses one color to color fill
 *                             the shaded pixels.
 * @PSY_PROGRAM_PICTURE: a shader program that uses uses a texture in order to
 *                       shade pixels.
 *
 * Programs are used to do some drawing operations. Generally you can obtain
 * these from a canvas/window on which you'd like to draw.
 * This enumeration is use for `psy_window_get_shader_program`
 */
typedef enum {
    PSY_PROGRAM_UNIFORM_COLOR,
    PSY_PROGRAM_PICTURE,
} PsyProgramType;


typedef struct _PsyProgramClass {
    GObjectClass parent_class;

    void (*set_vertex_shader)(PsyProgram *program,
                              PsyShader  *shader,
                              GError    **error);

    void (*set_vertex_shader_source)(PsyProgram  *program,
                                     const gchar *source,
                                     GError     **error);

    void (*set_vertex_shader_from_file)(PsyProgram *program,
                                        GFile      *shader_file,
                                        GError    **error);
    
    void (*set_vertex_shader_from_path)(PsyProgram   *program,
                                        const gchar  *shader_path,
                                        GError      **error);

    void (*set_fragment_shader)(PsyProgram *program,
                                PsyShader  *shader,
                                GError    **error);

    void (*set_fragment_shader_source)(PsyProgram   *program,
                                       const gchar  *source,
                                       GError      **error);

    void (*set_fragment_shader_from_file)(PsyProgram *program,
                                          GFile      *shader_file,
                                          GError    **error);
    
    void (*set_fragment_shader_from_path)(PsyProgram   *program,
                                          const gchar  *shader,
                                          GError      **error);
    
    void (*link)          (PsyProgram *program, GError** error);
    
    gboolean (*is_linked) (PsyProgram *program);

    void (*use_program) (PsyProgram* program, GError ** error);

    void (*set_uniform_matrix4) (PsyProgram     *program,
                                 const gchar    *name,
                                 PsyMatrix4     *matrix,
                                 GError        **error
                                 );

} PsyProgramClass;



G_MODULE_EXPORT void
psy_program_set_vertex_shader(PsyProgram       *program,
                              PsyShader        *shader,
                              GError          **error);
G_MODULE_EXPORT void
psy_program_set_vertex_shader_source(PsyProgram       *program,
                                     const gchar      *source,
                                     GError          **error);

G_MODULE_EXPORT void
psy_program_set_vertex_shader_from_file(PsyProgram    *program,
                                        GFile         *shader_file,
                                        GError       **error);

G_MODULE_EXPORT void
psy_program_set_vertex_shader_from_path(PsyProgram    *program,
                                        const gchar   *shader_path,
                                        GError       **error);

G_MODULE_EXPORT void
psy_program_set_fragment_shader(PsyProgram       *program,
                                PsyShader        *shader,
                                GError          **error);
G_MODULE_EXPORT void
psy_program_set_fragment_shader_source(PsyProgram       *program,
                                       const gchar      *source,
                                       GError          **error);

G_MODULE_EXPORT void
psy_program_set_fragment_shader_from_file (PsyProgram   *program,
                                           GFile        *shader_file,
                                           GError      **error);

G_MODULE_EXPORT void
psy_program_set_fragment_shader_from_path(PsyProgram    *program,
                                          const gchar   *shader_path,
                                          GError       **error);


G_MODULE_EXPORT void
psy_program_link(PsyProgram *self, GError **error);

G_MODULE_EXPORT gboolean
psy_program_is_linked(PsyProgram *self);

G_MODULE_EXPORT void
psy_program_use(PsyProgram* self, GError **error);

G_MODULE_EXPORT void
psy_program_set_uniform_matrix4(
        PsyProgram     *program,
        const gchar    *name,
        PsyMatrix4     *matrix,
        GError        **error
        );

G_END_DECLS

#endif
