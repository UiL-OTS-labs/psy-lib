

#include "ddd-gl-error.h"
#include "ddd-gl-shader.h"
#include "ddd-shader.h"
#include <epoxy/gl_generated.h>

typedef struct _DddGlShaderPrivate {
    GLuint object_id;
    guint  is_compiled : 1;
} DddGlShaderPrivate;

G_DEFINE_TYPE_WITH_PRIVATE(DddGlShader, ddd_gl_shader, DDD_TYPE_SHADER)

typedef enum {
    PROP_NULL,
    PROP_OBJECT_ID,
    NUM_PROPERTIES    
} DddGlShaderProperty;

static GParamSpec* gl_shader_properties[NUM_PROPERTIES];

static void
ddd_gl_shader_set_property(GObject        *object,
                           guint           prop_id,
                           const GValue   *value,
                           GParamSpec     *pspec)
{
    DddShader* self = DDD_SHADER(object);
    (void) self, (void) value;

    switch((DddGlShaderProperty) prop_id) {
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

static void
ddd_gl_shader_get_property(GObject    *object,
                           guint       prop_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
    DddGlShader* self = DDD_GL_SHADER(object);
    DddGlShaderPrivate* priv = ddd_gl_shader_get_instance_private(self);

    switch((DddGlShaderProperty) prop_id) {
        case PROP_OBJECT_ID:
            g_value_set_uint(value, priv->object_id);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

static void
ddd_gl_shader_init(DddGlShader *self)
{
    DddGlShaderPrivate* priv = ddd_gl_shader_get_instance_private(self);

    priv->is_compiled = FALSE;
}

static void
ddd_gl_shader_dispose(GObject* object)
{
    DddGlShader* self = DDD_GL_SHADER(object);
    DddGlShaderPrivate* priv = ddd_gl_shader_get_instance_private(self);
    (void) priv;

    G_OBJECT_CLASS(ddd_gl_shader_parent_class)->dispose(object);
}

static void
ddd_gl_shader_finalize(GObject* object)
{
    DddGlShader* self = DDD_GL_SHADER(object);
    DddGlShaderPrivate* priv = ddd_gl_shader_get_instance_private(self);

    if (priv->object_id) {
        glDeleteShader(priv->object_id);
    }

    G_OBJECT_CLASS(ddd_gl_shader_parent_class)->dispose(object);
}

static void
ddd_gl_shader_constructed(GObject* object)
{
    DddGlShader* self = DDD_GL_SHADER(object);
    DddGlShaderClass * klass = DDD_GL_SHADER_GET_CLASS(self);
    DddGlShaderPrivate *priv = ddd_gl_shader_get_instance_private(self);

    g_assert(klass->create_shader);
    if(klass->create_shader)
        priv->object_id = klass->create_shader();

    G_OBJECT_CLASS(ddd_gl_shader_parent_class)->constructed(object);
}

static void
ddd_gl_shader_compile(DddShader* self, GError **error)
{
    gint compile_success;
    DddGlShaderPrivate* priv = ddd_gl_shader_get_instance_private(
            DDD_GL_SHADER(self)
            );
    const char* source = ddd_shader_get_source(DDD_SHADER(self));
    GLuint object_id = priv->object_id;

    glShaderSource(object_id, 1, &source, NULL);
    glCompileShader(object_id);
    glGetShaderiv(object_id, GL_COMPILE_STATUS, &compile_success);

    if (!compile_success) {
        GString* log = NULL;
        gint logsize;

        glGetShaderiv(object_id, GL_INFO_LOG_LENGTH, &logsize);
        log = g_string_new_len("", logsize);

        glGetShaderInfoLog(object_id, logsize, &logsize, log->str);
        g_set_error(error,
                    DDD_GL_ERROR,
                    DDD_GL_ERROR_SHADER_COMPILE,
                    "Unable to compile shader:\n%s", log->str);
        g_string_free(log, TRUE);
        return;
    }
    priv->is_compiled = TRUE;
}

static gboolean
ddd_gl_shader_is_compiled(DddShader* shader)
{
    DddGlShaderPrivate *priv = ddd_gl_shader_get_instance_private(
            DDD_GL_SHADER(shader)
            );

    return priv->is_compiled;
}


static void
ddd_gl_shader_class_init(DddGlShaderClass* class)
{
    GObjectClass       *gobject_class = G_OBJECT_CLASS(class);
    DddShaderClass     *shader_class  = DDD_SHADER_CLASS(class);

    gobject_class->set_property = ddd_gl_shader_set_property;
    gobject_class->get_property = ddd_gl_shader_get_property;
    gobject_class->finalize     = ddd_gl_shader_finalize;
    gobject_class->dispose      = ddd_gl_shader_dispose;
    gobject_class->constructed  = ddd_gl_shader_constructed;

    shader_class->compile       = ddd_gl_shader_compile;
    shader_class->is_compiled   = ddd_gl_shader_is_compiled;

    gl_shader_properties[PROP_OBJECT_ID] = g_param_spec_string(
            "object-id",
            "Object ID",
            "The OpenGL id of the object",
            0,
            G_PARAM_READWRITE
            );

    g_object_class_install_properties(
            gobject_class, NUM_PROPERTIES, gl_shader_properties
            );
}

/* ************ public functions ******************** */


guint
ddd_gl_shader_get_object_id(DddGlShader* self) {
    g_return_val_if_fail(DDD_IS_SHADER(self), 0);
    
    DddGlShaderPrivate* priv = ddd_gl_shader_get_instance_private(self);
    
    return priv->object_id;
}

