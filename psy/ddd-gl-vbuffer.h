#ifndef DDD_GL_VBUFFER_H
#define DDD_GL_VBUFFER_H

#include "ddd-vbuffer.h"

G_BEGIN_DECLS

#define DDD_TYPE_GL_VBUFFER ddd_gl_vbuffer_get_type()
G_DECLARE_FINAL_TYPE(DddGlVBuffer, ddd_gl_vbuffer, DDD, GL_VBUFFER, DddVBuffer)

G_MODULE_EXPORT DddGlVBuffer*
ddd_gl_vbuffer_new();

G_MODULE_EXPORT guint 
ddd_gl_vbuffer_get_object_id(DddGlVBuffer* vbuffer);


G_END_DECLS

#endif
