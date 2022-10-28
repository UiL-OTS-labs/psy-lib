#ifndef PSY_GL_CIRCLE_H
#define PSY_GL_CIRCLE_H

#include "../psy-artist.h"
#include "../psy-circle.h"
#include "../psy-window.h"

G_BEGIN_DECLS

#define PSY_TYPE_GL_CIRCLE psy_gl_circle_get_type()
G_DECLARE_FINAL_TYPE(PsyGlCircle, psy_gl_circle, PSY, GL_CIRCLE, PsyArtist)

G_MODULE_EXPORT PsyGlCircle*
psy_gl_circle_new(PsyWindow* window, PsyVisualStimulus* stimulus);

G_MODULE_EXPORT guint 
psy_gl_circle_get_object_id(PsyGlCircle* circle);


G_END_DECLS

#endif
