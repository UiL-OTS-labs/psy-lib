#ifndef PSY_GL_SHADER_H
#define PSY_GL_SHADER_H

#include "psy-shader.h"
#include <epoxy/gl.h>

G_BEGIN_DECLS

#define PSY_TYPE_GL_SHADER psy_gl_shader_get_type()
G_DECLARE_DERIVABLE_TYPE(PsyGlShader, psy_gl_shader, PSY, GL_SHADER, PsyShader)

typedef struct _PsyGlShaderClass {
    PsyShaderClass parent_class;
    GLuint (*create_shader)(void);
} PsyGlShaderClass;

G_MODULE_EXPORT guint
psy_gl_shader_get_object_id(PsyGlShader *shader);

G_END_DECLS

#endif
