

#include "ddd-shader.h"


typedef struct _DddShaderPrivate {
    char* source;
} DddShaderPrivate;

G_DEFINE_TYPE_WITH_PRIVATE(DddShader, ddd_shader, G_TYPE_OBJECT)

typedef enum {
    PROP_NULL,
    PROP_SOURCE,
    PROP_IS_COMPILED,
    NUM_PROPERTIES    
} DddShaderProperty;

static GParamSpec* shader_properties[NUM_PROPERTIES];

static void
ddd_shader_set_property(GObject        *object,
                        guint           prop_id,
                        const GValue   *value,
                        GParamSpec     *pspec)
{
    DddShader* self = DDD_SHADER(object);

    switch((DddShaderProperty) prop_id) {
        case PROP_SOURCE:
            ddd_shader_set_source(self, g_value_get_string(value));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

static void
ddd_shader_get_property(GObject    *object,
                        guint       prop_id,
                        GValue     *value,
                        GParamSpec *pspec)
{
    DddShader* self = DDD_SHADER(object);
    DddShaderPrivate* priv = ddd_shader_get_instance_private(self);

    switch((DddShaderProperty) prop_id) {
        case PROP_SOURCE:
            g_value_set_string(value, priv->source);
            break;
        case PROP_IS_COMPILED:
            g_value_set_boolean(value, ddd_shader_is_compiled(self));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

static void
ddd_shader_init(DddShader *self)
{
    DddShaderPrivate* priv = ddd_shader_get_instance_private(self);
    (void) priv;
    priv->source = g_strdup("");
}

static void
ddd_shader_dispose(GObject* object)
{
    DddShader* self = DDD_SHADER(object);
    DddShaderPrivate* priv = ddd_shader_get_instance_private(self);
    (void) priv;

    G_OBJECT_CLASS(ddd_shader_parent_class)->dispose(object);
}

static void
ddd_shader_finalize(GObject* object)
{
    DddShader* self = DDD_SHADER(object);
    DddShaderPrivate* priv = ddd_shader_get_instance_private(self);
    (void) priv;

    G_OBJECT_CLASS(ddd_shader_parent_class)->dispose(object);
}


static void
ddd_shader_class_init(DddShaderClass* klass)
{
    GObjectClass   *gobject_class = G_OBJECT_CLASS(klass);

    gobject_class->set_property = ddd_shader_set_property;
    gobject_class->get_property = ddd_shader_get_property;
    gobject_class->finalize     = ddd_shader_finalize;
    gobject_class->dispose      = ddd_shader_dispose;

    shader_properties[PROP_SOURCE] = g_param_spec_string(
            "source",
            "Source",
            "The source code of the shader",
            "",
            G_PARAM_READWRITE
            );
    
    shader_properties[PROP_IS_COMPILED] = g_param_spec_boolean(
            "is-compiled",
            "compiled",
            "Whether or not the shader is compiled",
            FALSE,
            G_PARAM_READABLE
            );

    g_object_class_install_properties(gobject_class,
                                      NUM_PROPERTIES,
                                      shader_properties);
}

/* ************ public functions ******************** */

void
ddd_shader_set_source (DddShader* self, const gchar* source)
{
    g_return_if_fail(DDD_IS_SHADER(self));
    g_return_if_fail(source != NULL);
    
    DddShaderPrivate* priv = ddd_shader_get_instance_private(self);

    if (priv->source)
        g_free(priv->source);

    priv->source = g_strdup(source);
}

const char*
ddd_shader_get_source(DddShader* self) {
    g_return_val_if_fail(DDD_IS_SHADER(self), NULL);
    
    DddShaderPrivate* priv = ddd_shader_get_instance_private(self);
    
    return priv->source;
}

void
ddd_shader_source_from_file(DddShader* self, GFile* file, GError **error) {
    g_return_if_fail(DDD_IS_SHADER(self));
    g_return_if_fail(G_IS_FILE(file));
    g_return_if_fail(error == NULL || *error == NULL);

    GFileInputStream* stream = g_file_read(file, NULL, error);
    if (*error)
        return;
    
    GByteArray* bytes = g_byte_array_sized_new(1024);

    while (1) {
        guint8 tmpbuf[1024];
        gsize nread;
        gboolean result = g_input_stream_read_all(G_INPUT_STREAM(stream),
                                                  &tmpbuf,
                                                  sizeof(tmpbuf),
                                                  &nread,
                                                  NULL,
                                                  error
                                                  );
        if (!result)
            goto fail;

        g_byte_array_append(bytes, tmpbuf, nread);
        if (result && nread < sizeof(tmpbuf)) 
            break;
    }

    g_byte_array_append(bytes, (guint8*) "", 1);

    ddd_shader_set_source(self, (const char*)bytes->data);

fail:
    g_object_unref(stream);
    g_byte_array_unref(bytes);
}

void
ddd_shader_source_from_path(DddShader *self, const gchar* path, GError ** error)
{
    g_return_if_fail(DDD_IS_SHADER(self));
    g_return_if_fail(path != NULL);
    g_return_if_fail(error == NULL || *error == NULL);

    GFile* file = g_file_new_for_path(path);
    ddd_shader_source_from_file(self, file, error);
    g_object_unref(file);
}

void
ddd_shader_compile(DddShader *self, GError **error) {
    DddShaderClass* klass;
    g_return_if_fail(DDD_IS_SHADER(self));

    klass = DDD_SHADER_GET_CLASS(self);
    g_return_if_fail(klass->compile != NULL);

    klass->compile(self, error);
}

gboolean
ddd_shader_is_compiled(DddShader *self)
{
    DddShaderClass* klass;
    g_return_val_if_fail(DDD_IS_SHADER(self), FALSE);

    klass = DDD_SHADER_GET_CLASS(self);
    g_return_val_if_fail(klass->is_compiled != NULL, FALSE);

    return klass->is_compiled(self);
}

