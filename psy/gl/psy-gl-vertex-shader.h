
#ifndef PSY_GL_VERTEX_SHADER_H
#define PSY_GL_VERTEX_SHADER_H

#include "psy-gl-shader.h"

G_BEGIN_DECLS

#define PSY_TYPE_GL_VERTEX_SHADER psy_gl_vertex_shader_get_type()
G_DECLARE_FINAL_TYPE(
    PsyGlVertexShader, psy_gl_vertex_shader, PSY, GL_VERTEX_SHADER, PsyGlShader)

G_MODULE_EXPORT PsyGlVertexShader *
psy_gl_vertex_shader_new(void);

G_MODULE_EXPORT void
psy_gl_vertex_shader_free(PsyGlVertexShader *self);

G_END_DECLS

#endif
