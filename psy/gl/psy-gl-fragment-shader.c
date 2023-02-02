

#include "psy-gl-fragment-shader.h"
#include <epoxy/gl_generated.h>

typedef struct _PsyGlFragmentShader {
    PsyGlShader parent;
} PsyGlFragmentShader;

G_DEFINE_TYPE(PsyGlFragmentShader, psy_gl_fragment_shader, PSY_TYPE_GL_SHADER)

static GLuint
psy_gl_fragment_shader_create_shader()
{
    GLuint object_id;
    object_id = glCreateShader(GL_FRAGMENT_SHADER);
    return object_id;
}

static void
psy_gl_fragment_shader_init(PsyGlFragmentShader *self)
{
    (void) self;
}

static void
psy_gl_fragment_shader_class_init(PsyGlFragmentShaderClass *klass)
{
    /*
    GObjectClass* object_class = G_OBJECT_CLASS(klass);
    */
    PsyGlShaderClass *gl_shader_class = PSY_GL_SHADER_CLASS(klass);

    /*
    object_class->dispose  = psy_gl_fragment_shader_dispose;
    object_class->finalize = psy_gl_fragment_shader_finalize;
    */

    gl_shader_class->create_shader = psy_gl_fragment_shader_create_shader;
}

/* ******************** public functions ************************ */

PsyGlFragmentShader *
psy_gl_fragment_shader_new()
{
    PsyGlFragmentShader *ret = g_object_new(PSY_TYPE_GL_FRAGMENT_SHADER, NULL);
    return ret;
}
