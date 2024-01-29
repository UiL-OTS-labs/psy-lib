#ifndef PSY_DRAWING_CONTEXT_H
#define PSY_DRAWING_CONTEXT_H

#include <gio/gio.h>

#include <psy-matrix4.h>
#include <psy-shader-program.h>
#include <psy-texture.h>
#include <psy-vbuffer.h>

G_BEGIN_DECLS

#define PSY_DRAWING_CONTEXT_ERROR psy_drawing_context_error_quark()
G_MODULE_EXPORT GQuark
psy_drawing_context_error_quark(void);

#define PSY_TYPE_DRAWING_CONTEXT psy_drawing_context_get_type()
G_DECLARE_DERIVABLE_TYPE(
    PsyDrawingContext, psy_drawing_context, PSY, DRAWING_CONTEXT, GObject)

typedef struct _PsyDrawingContextClass {
    GObjectClass parent_class;

    PsyShaderProgram *(*create_program)(PsyDrawingContext *self);
    PsyShader *(*create_vertex_shader)(PsyDrawingContext *self);
    PsyShader *(*create_fragment_shader)(PsyDrawingContext *self);
    PsyTexture *(*create_texture)(PsyDrawingContext *self);
    PsyVBuffer *(*create_vbuffer)(PsyDrawingContext *self);

    gpointer extensions[16];

} PsyDrawingContextClass;

G_MODULE_EXPORT void
psy_drawing_context_free_resources(PsyDrawingContext *self);

G_MODULE_EXPORT PsyShaderProgram *
psy_drawing_context_create_program(PsyDrawingContext *self);

G_MODULE_EXPORT PsyShader *
psy_drawing_context_create_vertex_shader(PsyDrawingContext *self);

G_MODULE_EXPORT PsyShader *
psy_drawing_context_create_fragment_shader(PsyDrawingContext *self);

G_MODULE_EXPORT PsyTexture *
psy_drawing_context_create_texture(PsyDrawingContext *self);

G_MODULE_EXPORT PsyVBuffer *
psy_drawing_context_create_vbuffer(PsyDrawingContext *self);

G_MODULE_EXPORT void
psy_drawing_context_register_program(PsyDrawingContext *self,
                                     const gchar       *name,
                                     PsyShaderProgram  *program,
                                     GError           **error);

G_MODULE_EXPORT void
psy_drawing_context_register_texture(PsyDrawingContext *self,
                                     const gchar       *texture_name,
                                     PsyTexture        *texture,
                                     GError           **error);

G_MODULE_EXPORT void
psy_drawing_context_load_files_as_texture(PsyDrawingContext *self,
                                          gchar            **files,
                                          gsize              num_files,
                                          GError           **error);

G_MODULE_EXPORT PsyShaderProgram *
psy_drawing_context_get_program(PsyDrawingContext *self, const gchar *name);

G_MODULE_EXPORT PsyTexture *
psy_drawing_context_get_texture(PsyDrawingContext *self, const gchar *name);

G_MODULE_EXPORT extern const gchar *PSY_UNIFORM_COLOR_PROGRAM_NAME;
G_MODULE_EXPORT extern const gchar *PSY_PICTURE_PROGRAM_NAME;

G_END_DECLS

#endif
