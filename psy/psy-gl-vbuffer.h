#ifndef PSY_GL_VBUFFER_H
#define PSY_GL_VBUFFER_H

#include "psy-vbuffer.h"

G_BEGIN_DECLS

#define PSY_TYPE_GL_VBUFFER psy_gl_vbuffer_get_type()
G_DECLARE_FINAL_TYPE(PsyGlVBuffer, psy_gl_vbuffer, PSY, GL_VBUFFER, PsyVBuffer)

G_MODULE_EXPORT PsyGlVBuffer*
psy_gl_vbuffer_new();

G_MODULE_EXPORT guint 
psy_gl_vbuffer_get_object_id(PsyGlVBuffer* vbuffer);


G_END_DECLS

#endif
