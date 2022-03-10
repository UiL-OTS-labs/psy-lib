

#include "ddd-gl-vertex-shader.h"
#include <epoxy/gl_generated.h>

typedef struct _DddGlVertexShader {
    DddGlShader parent;
} DddGlVertexShader;

G_DEFINE_TYPE(DddGlVertexShader, ddd_gl_vertex_shader, DDD_TYPE_GL_SHADER)

static GLuint
ddd_gl_vertex_shader_create_shader()
{
    GLuint object_id;
    object_id = glCreateShader(GL_VERTEX_SHADER);
    return object_id;
}

static void ddd_gl_vertex_shader_init(DddGlVertexShader* self)
{
    (void) self;
}

static void
ddd_gl_vertex_shader_class_init(DddGlVertexShaderClass* klass)
{
    /*
    GObjectClass* object_class = G_OBJECT_CLASS(klass);
    */
    DddGlShaderClass* gl_shader_class = DDD_GL_SHADER_CLASS(klass);

    /*
    object_class->dispose  = ddd_gl_vertex_shader_dispose;
    object_class->finalize = ddd_gl_vertex_shader_finalize;
    */

    gl_shader_class->create_shader = ddd_gl_vertex_shader_create_shader;
}

/* ************ public functions *************/

DddGlVertexShader*
ddd_gl_vertex_shader_new()
{
    DddGlVertexShader* shader = g_object_new(DDD_TYPE_GL_VERTEX_SHADER, NULL);
    return shader;
}