

#include "ddd-gl-fragment-shader.h"
#include <epoxy/gl_generated.h>

typedef struct _DddGlFragmentShader {
    DddGlShader parent;
} DddGlFragmentShader;

G_DEFINE_TYPE(DddGlFragmentShader, ddd_gl_fragment_shader, DDD_TYPE_GL_SHADER)

static GLuint
ddd_gl_fragment_shader_create_shader()
{
    GLuint object_id;
    object_id = glCreateShader(GL_FRAGMENT_SHADER);
    return object_id;
}

static void ddd_gl_fragment_shader_init(DddGlFragmentShader* self)
{
    (void) self;
}

static void
ddd_gl_fragment_shader_class_init(DddGlFragmentShaderClass* klass)
{
    /*
    GObjectClass* object_class = G_OBJECT_CLASS(klass);
    */
    DddGlShaderClass* gl_shader_class = DDD_GL_SHADER_CLASS(klass);

    /*
    object_class->dispose  = ddd_gl_fragment_shader_dispose;
    object_class->finalize = ddd_gl_fragment_shader_finalize;
    */

    gl_shader_class->create_shader = ddd_gl_fragment_shader_create_shader;
}


/* ******************** public functions ************************ */

DddGlFragmentShader*
ddd_gl_fragment_shader_new()
{
    DddGlFragmentShader *ret = g_object_new(DDD_TYPE_GL_FRAGMENT_SHADER, NULL);
    return ret;
}