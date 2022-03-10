#ifndef DDD_GL_SHADER_H
#define DDD_GL_SHADER_H

#include "ddd-shader.h"
#include <epoxy/gl.h>

G_BEGIN_DECLS

#define DDD_TYPE_GL_SHADER ddd_gl_shader_get_type()
G_DECLARE_DERIVABLE_TYPE(DddGlShader, ddd_gl_shader, DDD, GL_SHADER, DddShader)

typedef struct _DddGlShaderClass {
    DddShaderClass parent_class;
    GLuint (*create_shader) (void);
} DddGlShaderClass;

G_MODULE_EXPORT guint 
ddd_gl_shader_get_object_id(DddGlShader* shader);



G_END_DECLS

#endif
