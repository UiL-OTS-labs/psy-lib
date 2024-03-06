
#pragma once

#include "../psy-image-canvas.h"

G_BEGIN_DECLS

#define PSY_TYPE_GL_CANVAS psy_gl_canvas_get_type()

G_MODULE_EXPORT
G_DECLARE_FINAL_TYPE(PsyGlCanvas, psy_gl_canvas, PSY, GL_CANVAS, PsyImageCanvas)

G_MODULE_EXPORT PsyGlCanvas *
psy_gl_canvas_new(gint width, gint height);

G_MODULE_EXPORT PsyGlCanvas *
psy_gl_canvas_new_full(gint     width,
                       gint     height,
                       gboolean use_es,
                       gboolean debug,
                       gint     gl_major,
                       gint     gl_minor);

G_MODULE_EXPORT void
psy_gl_canvas_free(PsyGlCanvas *self);

G_END_DECLS
