#ifndef PSY_DRAWING_CONTEXT_H
#define PSY_DRAWING_CONTEXT_H

#include <gio/gio.h>
#include <psy-matrix4.h>
#include <psy-program.h>
#include <psy-shader.h>
#include <psy-vbuffer.h>

G_BEGIN_DECLS

#define PSY_TYPE_DRAWING_CONTEXT psy_drawing_context_get_type()
G_DECLARE_DERIVABLE_TYPE(PsyDrawingContext, psy_drawing_context, PSY, DRAWING_CONTEXT, GObject)

typedef struct _PsyDrawingContextClass {
    GObjectClass parent_class;

    PsyProgram* (*create_program)(PsyDrawingContext* self);
    PsyShader* (*create_shader)(PsyDrawingContext* self);
    PsyVBuffer* (*create_vbuffer)(PsyDrawingContext* self);

} PsyDrawingContextClass;


G_MODULE_EXPORT PsyProgram*
psy_drawing_context_create_program(PsyDrawingContext* self);

G_MODULE_EXPORT PsyShader*
psy_drawing_context_create_shader(PsyDrawingContext* self);

G_MODULE_EXPORT PsyVBuffer*
psy_drawing_context_create_vbuffer(PsyDrawingContext* self);


G_END_DECLS

#endif
