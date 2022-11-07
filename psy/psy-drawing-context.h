#ifndef PSY_DRAWING_CONTEXT_H
#define PSY_DRAWING_CONTEXT_H

#include <gio/gio.h>
#include <psy-matrix4.h>
#include <psy-program.h>
#include <psy-shader.h>
#include <psy-vbuffer.h>

G_BEGIN_DECLS

#define PSY_DRAWING_CONTEXT_ERROR psy_drawing_context_error_quark()
G_MODULE_EXPORT GQuark
psy_drawing_context_error_quark(void);

#define PSY_TYPE_DRAWING_CONTEXT psy_drawing_context_get_type()
G_DECLARE_DERIVABLE_TYPE(PsyDrawingContext, psy_drawing_context, PSY, DRAWING_CONTEXT, GObject)

typedef struct _PsyDrawingContextClass {
    GObjectClass parent_class;

    PsyProgram* (*create_program)(PsyDrawingContext* self);
    PsyShader* (*create_vertex_shader)(PsyDrawingContext* self);
    PsyShader* (*create_fragment_shader)(PsyDrawingContext* self);
    PsyVBuffer* (*create_vbuffer)(PsyDrawingContext* self);

} PsyDrawingContextClass;


G_MODULE_EXPORT PsyProgram*
psy_drawing_context_create_program(PsyDrawingContext* self);

G_MODULE_EXPORT PsyShader*
psy_drawing_context_create_vertex_shader(PsyDrawingContext* self);

G_MODULE_EXPORT PsyShader*
psy_drawing_context_create_fragment_shader(PsyDrawingContext* self);

G_MODULE_EXPORT PsyVBuffer*
psy_drawing_context_create_vbuffer(PsyDrawingContext* self);

G_MODULE_EXPORT void
psy_drawing_context_register_programe (
        PsyDrawingContext* self,
        const gchar* name,
        PsyProgram* program,
        GError** error
        );

G_END_DECLS

#endif
