
#ifndef DDD_GL_FRAGMENT_SHADER_H
#define DDD_GL_FRAGMENT_SHADER_H

#include "ddd-gl-shader.h"

G_BEGIN_DECLS

#define DDD_TYPE_GL_FRAGMENT_SHADER ddd_gl_fragment_shader_get_type()
G_DECLARE_FINAL_TYPE(
        DddGlFragmentShader, ddd_gl_fragment_shader, DDD, GL_FRAGMENT_SHADER, DddGlShader
        )

G_MODULE_EXPORT DddGlFragmentShader*
ddd_gl_fragment_shader_new();


G_END_DECLS

#endif
