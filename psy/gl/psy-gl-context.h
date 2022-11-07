#ifndef PSY_GL_CONTEXT_H
#define PSY_GL_CONTEXT_H

#include <psy-drawing-context.h>

G_BEGIN_DECLS

#define PSY_TYPE_GL_CONTEXT psy_gl_context_get_type()
G_DECLARE_FINAL_TYPE(PsyGlContext, psy_gl_context, PSY, GL_CONTEXT, PsyDrawingContext)

G_MODULE_EXPORT PsyGlContext* 
psy_gl_context_new(void);



G_END_DECLS

#endif
