

#include "psy-gl-vertex-shader.h"
#include <epoxy/gl_generated.h>

typedef struct _PsyGlVertexShader {
    PsyGlShader parent;
} PsyGlVertexShader;

G_DEFINE_TYPE(PsyGlVertexShader, psy_gl_vertex_shader, PSY_TYPE_GL_SHADER)

static GLuint
psy_gl_vertex_shader_create_shader()
{
    GLuint object_id;
    object_id = glCreateShader(GL_VERTEX_SHADER);
    return object_id;
}

static void
psy_gl_vertex_shader_init(PsyGlVertexShader *self)
{
    (void) self;
}

static void
psy_gl_vertex_shader_class_init(PsyGlVertexShaderClass *klass)
{
    /*
    GObjectClass* object_class = G_OBJECT_CLASS(klass);
    */
    PsyGlShaderClass *gl_shader_class = PSY_GL_SHADER_CLASS(klass);

    /*
    object_class->dispose  = psy_gl_vertex_shader_dispose;
    object_class->finalize = psy_gl_vertex_shader_finalize;
    */

    gl_shader_class->create_shader = psy_gl_vertex_shader_create_shader;
}

/* ************ public functions *************/

PsyGlVertexShader *
psy_gl_vertex_shader_new()
{
    PsyGlVertexShader *shader = g_object_new(PSY_TYPE_GL_VERTEX_SHADER, NULL);
    return shader;
}
