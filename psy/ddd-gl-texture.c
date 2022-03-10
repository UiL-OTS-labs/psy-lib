

#include "ddd-gl-error.h"
#include "ddd-gl-texture.h"
#include <epoxy/gl.h>

typedef struct _DddGlTexture {
    DddTexture parent;
    guint object_id;
    guint is_uploaded : 1;
} DddGlTexture;

G_DEFINE_TYPE(DddGlTexture, ddd_gl_texture, DDD_TYPE_TEXTURE)

typedef enum {
    PROP_NULL,
    PROP_OBJECT_ID,
    NUM_PROPERTIES    
} DddGlTextureProperty;

static GParamSpec* gl_texture_properties[NUM_PROPERTIES];

static void
ddd_gl_texture_set_property(GObject        *object,
                           guint           prop_id,
                           const GValue   *value,
                           GParamSpec     *pspec)
{
    DddTexture* self = DDD_TEXTURE(object);
    (void) self, (void) value;

    switch((DddGlTextureProperty) prop_id) {
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

static void
ddd_gl_texture_get_property(GObject    *object,
                           guint       prop_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
    DddGlTexture* self = DDD_GL_TEXTURE(object);

    switch((DddGlTextureProperty) prop_id) {
        case PROP_OBJECT_ID:
            g_value_set_uint(value, self->object_id);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

static void
ddd_gl_texture_init(DddGlTexture *self)
{
    self->is_uploaded = FALSE;
}

static void
ddd_gl_texture_dispose(GObject* object)
{
    DddGlTexture* self = DDD_GL_TEXTURE(object);
    (void) self;

    G_OBJECT_CLASS(ddd_gl_texture_parent_class)->dispose(object);
}

static void
ddd_gl_texture_finalize(GObject* object)
{
    DddGlTexture* self = DDD_GL_TEXTURE(object);

    if (self->object_id) {
        glDeleteTextures(1, &self->object_id);
        self->object_id = 0;
    }

    G_OBJECT_CLASS(ddd_gl_texture_parent_class)->dispose(object);
}

static void
ddd_gl_texture_upload(DddTexture* self, GError **error)
{
    DddGlTexture * gl_self = DDD_GL_TEXTURE(self);

    const guint8* source = ddd_texture_get_data(DDD_TEXTURE(self));
    gint width  = (int) ddd_texture_get_width(self);
    gint height = (int) ddd_texture_get_height(self);
    guint num_channels = ddd_texture_get_num_channels(self);

    if(!source) {
        g_set_error(error, DDD_TEXTURE_ERROR, DDD_TEXTURE_ERROR_FAILED,
                    "The texture has not any data to upload"
                    );
        return;
    }
    if(!width) {
        g_set_error(error, DDD_TEXTURE_ERROR, DDD_TEXTURE_ERROR_FAILED,
                    "The width of the texture is %u", width);
    }
    if(!width) {
        g_set_error(error, DDD_TEXTURE_ERROR, DDD_TEXTURE_ERROR_FAILED,
                    "The height of the texture is %u", height);
    }
    if (!num_channels) {
        g_set_error(error, DDD_TEXTURE_ERROR, DDD_TEXTURE_ERROR_FAILED,
                    "The the texture num channels is %u", num_channels);
    }

    if (gl_self->object_id)
        glDeleteTextures(1, &gl_self->object_id);

    glGenTextures(1, &gl_self->object_id);
    if (ddd_gl_check_error(error))
        return;

    glActiveTexture(GL_TEXTURE0);
    if(ddd_gl_check_error(error))
        return;

    glBindTexture(GL_TEXTURE_2D, gl_self->object_id);
    if (ddd_gl_check_error(error))
        return;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    if (ddd_gl_check_error(error))
        return;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    if (ddd_gl_check_error(error))
        return;

    glTexParameteri(
            GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    if (ddd_gl_check_error(error))
        return;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    if (ddd_gl_check_error(error))
        return;

    int pix_format = 0;
    switch (num_channels) {
        case 1:
            pix_format = GL_LUMINANCE; break; // May break glTexImage2d below...
        case 2:
            pix_format = GL_LUMINANCE_ALPHA; break;  // May break glTexImage2d below...
        case 3:
            pix_format = GL_RGB; break;
        case 4:
            pix_format = GL_RGBA; break;
        default:
            g_assert_not_reached();
    }
    if (pix_format == 0) {
        g_assert(pix_format != 0);
        g_critical("The texture has a unsupported number of channels");
    }

    glTexImage2D(GL_TEXTURE_2D, 0, pix_format, width, height, 0, pix_format,
                 GL_UNSIGNED_BYTE, source);
    if (ddd_gl_check_error(error)) // If an error happens here check case 1 and 2 above.
        return;

    glGenerateMipmap(GL_TEXTURE_2D);
    if (ddd_gl_check_error(error)) // If an error happens here check case 1 and 2 above.
        return;

    gl_self->is_uploaded = TRUE;
}

static gboolean
ddd_gl_texture_is_uploaded(DddTexture* self)
{
    DddGlTexture *gl_self = DDD_GL_TEXTURE(self);
    return gl_self->is_uploaded;
}

static void
ddd_gl_texture_bind(DddTexture* self, GError **error)
{
    DddGlTexture *gl_self = DDD_GL_TEXTURE(self);

    glActiveTexture(GL_TEXTURE0);
    if (ddd_gl_check_error(error))
        return;

    glBindTexture(GL_TEXTURE_2D, gl_self->object_id);
    if (ddd_gl_check_error(error))
        return;
}

static void
ddd_gl_texture_class_init(DddGlTextureClass* class)
{
    GObjectClass       *gobject_class = G_OBJECT_CLASS(class);
    DddTextureClass    *texture_class = DDD_TEXTURE_CLASS(class);

    gobject_class->set_property = ddd_gl_texture_set_property;
    gobject_class->get_property = ddd_gl_texture_get_property;
    gobject_class->finalize     = ddd_gl_texture_finalize;
    gobject_class->dispose      = ddd_gl_texture_dispose;

    texture_class->upload       = ddd_gl_texture_upload;
    texture_class->is_uploaded  = ddd_gl_texture_is_uploaded;
    texture_class->bind         = ddd_gl_texture_bind;

    gl_texture_properties[PROP_OBJECT_ID] = g_param_spec_string(
            "object-id",
            "Object ID",
            "The OpenGL id of the texture",
            0,
            G_PARAM_READWRITE
            );

    g_object_class_install_properties(
            gobject_class, NUM_PROPERTIES, gl_texture_properties
            );
}

/* ************ public functions ******************** */

DddGlTexture*
ddd_gl_texture_new()
{
    return g_object_new(DDD_TYPE_GL_TEXTURE, NULL);
}

DddGlTexture*
ddd_gl_texture_new_for_file(GFile* file)
{
    return g_object_new(
            DDD_TYPE_GL_TEXTURE,
            "file", file,
            NULL
    );
}

DddGlTexture*
ddd_gl_texture_new_for_path(const gchar* path)
{
    return g_object_new(
            DDD_TYPE_GL_TEXTURE,
            "path", path,
            NULL
            );
}

guint
ddd_gl_texture_get_object_id(DddGlTexture* self) {
    g_return_val_if_fail(DDD_IS_TEXTURE(self), 0);
    
    return self->object_id;
}

