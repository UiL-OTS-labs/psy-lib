
#ifndef DDD_GL_VERTEX_SHADER_H
#define DDD_GL_VERTEX_SHADER_H

#include "ddd-gl-shader.h"

G_BEGIN_DECLS

#define DDD_TYPE_GL_VERTEX_SHADER ddd_gl_vertex_shader_get_type()
G_DECLARE_FINAL_TYPE(
        DddGlVertexShader, ddd_gl_vertex_shader, DDD, GL_VERTEX_SHADER, DddGlShader
        )

G_MODULE_EXPORT DddGlVertexShader*
ddd_gl_vertex_shader_new();


G_END_DECLS

#endif
