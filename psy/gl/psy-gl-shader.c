

#include "psy-gl-shader.h"
#include "psy-gl-error.h"
#include "psy-shader.h"
#include <epoxy/gl_generated.h>

typedef struct _PsyGlShaderPrivate {
    GLuint object_id;
    guint  is_compiled : 1;
} PsyGlShaderPrivate;

G_DEFINE_TYPE_WITH_PRIVATE(PsyGlShader, psy_gl_shader, PSY_TYPE_SHADER)

typedef enum { PROP_NULL, PROP_OBJECT_ID, NUM_PROPERTIES } PsyGlShaderProperty;

static GParamSpec *gl_shader_properties[NUM_PROPERTIES];

static void
psy_gl_shader_get_property(GObject    *object,
                           guint       prop_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
    PsyGlShader *self = PSY_GL_SHADER(object);

    switch ((PsyGlShaderProperty) prop_id) {
    case PROP_OBJECT_ID:
        g_value_set_uint(value, psy_gl_shader_get_object_id(self));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

static void
psy_gl_shader_init(PsyGlShader *self)
{
    PsyGlShaderPrivate *priv = psy_gl_shader_get_instance_private(self);

    priv->is_compiled = FALSE;
}

static void
psy_gl_shader_dispose(GObject *object)
{
    PsyGlShader        *self = PSY_GL_SHADER(object);
    PsyGlShaderPrivate *priv = psy_gl_shader_get_instance_private(self);
    (void) priv;

    G_OBJECT_CLASS(psy_gl_shader_parent_class)->dispose(object);
}

static void
psy_gl_shader_finalize(GObject *object)
{
    PsyGlShader        *self = PSY_GL_SHADER(object);
    PsyGlShaderPrivate *priv = psy_gl_shader_get_instance_private(self);

    if (priv->object_id) {
        glDeleteShader(priv->object_id);
    }

    G_OBJECT_CLASS(psy_gl_shader_parent_class)->finalize(object);
}

static void
psy_gl_shader_constructed(GObject *object)
{
    PsyGlShader        *self  = PSY_GL_SHADER(object);
    PsyGlShaderClass   *klass = PSY_GL_SHADER_GET_CLASS(self);
    PsyGlShaderPrivate *priv  = psy_gl_shader_get_instance_private(self);

    g_assert(klass->create_shader);
    if (klass->create_shader)
        priv->object_id = klass->create_shader();

    G_OBJECT_CLASS(psy_gl_shader_parent_class)->constructed(object);
}

static void
psy_gl_shader_compile(PsyShader *self, GError **error)
{
    gint                compile_success;
    PsyGlShaderPrivate *priv
        = psy_gl_shader_get_instance_private(PSY_GL_SHADER(self));
    const char *source    = psy_shader_get_source(PSY_SHADER(self));
    GLuint      object_id = priv->object_id;

    glShaderSource(object_id, 1, &source, NULL);
    glCompileShader(object_id);
    glGetShaderiv(object_id, GL_COMPILE_STATUS, &compile_success);

    if (!compile_success) {
        gchar log_buffer[16384];
        gint  logsize;

        glGetShaderiv(object_id, GL_INFO_LOG_LENGTH, &logsize);

        glGetShaderInfoLog(object_id, sizeof(log_buffer), &logsize, log_buffer);
        g_set_error(error,
                    PSY_GL_ERROR,
                    PSY_GL_ERROR_SHADER_COMPILE,
                    "Unable to compile shader:\n%s",
                    log_buffer);
        return;
    }
    priv->is_compiled = TRUE;
}

static gboolean
psy_gl_shader_is_compiled(PsyShader *shader)
{
    PsyGlShaderPrivate *priv
        = psy_gl_shader_get_instance_private(PSY_GL_SHADER(shader));

    return priv->is_compiled;
}

static void
psy_gl_shader_class_init(PsyGlShaderClass *class)
{
    GObjectClass   *gobject_class = G_OBJECT_CLASS(class);
    PsyShaderClass *shader_class  = PSY_SHADER_CLASS(class);

    gobject_class->get_property = psy_gl_shader_get_property;
    gobject_class->finalize     = psy_gl_shader_finalize;
    gobject_class->dispose      = psy_gl_shader_dispose;
    gobject_class->constructed  = psy_gl_shader_constructed;

    shader_class->compile     = psy_gl_shader_compile;
    shader_class->is_compiled = psy_gl_shader_is_compiled;

    gl_shader_properties[PROP_OBJECT_ID]
        = g_param_spec_uint("object-id",
                            "Object ID",
                            "The OpenGL id of the object",
                            0,
                            G_MAXUINT,
                            0,
                            G_PARAM_READABLE);

    g_object_class_install_properties(
        gobject_class, NUM_PROPERTIES, gl_shader_properties);
}

/* ************ public functions ******************** */

guint
psy_gl_shader_get_object_id(PsyGlShader *self)
{
    g_return_val_if_fail(PSY_IS_SHADER(self), 0);

    PsyGlShaderPrivate *priv = psy_gl_shader_get_instance_private(self);

    return priv->object_id;
}
