
#ifndef PSY_GL_FRAGMENT_SHADER_H
#define PSY_GL_FRAGMENT_SHADER_H

#include "psy-gl-shader.h"

G_BEGIN_DECLS

#define PSY_TYPE_GL_FRAGMENT_SHADER psy_gl_fragment_shader_get_type()
G_DECLARE_FINAL_TYPE(PsyGlFragmentShader,
                     psy_gl_fragment_shader,
                     PSY,
                     GL_FRAGMENT_SHADER,
                     PsyGlShader)

G_MODULE_EXPORT PsyGlFragmentShader *
psy_gl_fragment_shader_new(void);

G_MODULE_EXPORT void
psy_gl_fragment_shader_free(PsyGlFragmentShader *self);

G_END_DECLS

#endif
