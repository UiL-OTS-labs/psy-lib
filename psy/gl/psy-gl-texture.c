
#include <epoxy/gl.h>

#include "../psy-enums.h"
#include "psy-gl-error.h"
#include "psy-gl-texture.h"

typedef struct _PsyGlTexture {
    PsyTexture parent;
    guint      object_id;
    guint      is_uploaded : 1;
} PsyGlTexture;

G_DEFINE_TYPE(PsyGlTexture, psy_gl_texture, PSY_TYPE_TEXTURE)

typedef enum { PROP_NULL, PROP_OBJECT_ID, NUM_PROPERTIES } PsyGlTextureProperty;

static GParamSpec *gl_texture_properties[NUM_PROPERTIES];

static void
psy_gl_texture_set_property(GObject      *object,
                            guint         prop_id,
                            const GValue *value,
                            GParamSpec   *pspec)
{
    PsyTexture *self = PSY_TEXTURE(object);
    (void) self, (void) value;

    switch ((PsyGlTextureProperty) prop_id) {
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

static void
psy_gl_texture_get_property(GObject    *object,
                            guint       prop_id,
                            GValue     *value,
                            GParamSpec *pspec)
{
    PsyGlTexture *self = PSY_GL_TEXTURE(object);

    switch ((PsyGlTextureProperty) prop_id) {
    case PROP_OBJECT_ID:
        g_value_set_uint(value, self->object_id);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

static void
psy_gl_texture_init(PsyGlTexture *self)
{
    self->is_uploaded = FALSE;
}

static void
psy_gl_texture_dispose(GObject *object)
{
    PsyGlTexture *self = PSY_GL_TEXTURE(object);
    (void) self;

    G_OBJECT_CLASS(psy_gl_texture_parent_class)->dispose(object);
}

static void
psy_gl_texture_finalize(GObject *object)
{
    PsyGlTexture *self = PSY_GL_TEXTURE(object);

    if (self->object_id) {
        glDeleteTextures(1, &self->object_id);
        self->object_id = 0;
    }

    G_OBJECT_CLASS(psy_gl_texture_parent_class)->finalize(object);
}

static void
psy_gl_texture_upload(PsyTexture *self, GError **error)
{
    PsyGlTexture *gl_self = PSY_GL_TEXTURE(self);

    const guint8 *source       = psy_texture_get_data(PSY_TEXTURE(self));
    gint          width        = (int) psy_texture_get_width(self);
    gint          height       = (int) psy_texture_get_height(self);
    guint         num_channels = psy_texture_get_num_channels(self);

    if (!source) {
        g_set_error(error,
                    PSY_TEXTURE_ERROR,
                    PSY_TEXTURE_ERROR_FAILED,
                    "The texture has not any data to upload");
        return;
    }
    if (!width) {
        g_set_error(error,
                    PSY_TEXTURE_ERROR,
                    PSY_TEXTURE_ERROR_FAILED,
                    "The width of the texture is %u",
                    width);
    }
    if (!height) {
        g_set_error(error,
                    PSY_TEXTURE_ERROR,
                    PSY_TEXTURE_ERROR_FAILED,
                    "The height of the texture is %u",
                    height);
    }
    if (!num_channels) {
        g_set_error(error,
                    PSY_TEXTURE_ERROR,
                    PSY_TEXTURE_ERROR_FAILED,
                    "The the texture num channels is %u",
                    num_channels);
    }

    if (gl_self->object_id)
        glDeleteTextures(1, &gl_self->object_id);

    glGenTextures(1, &gl_self->object_id);
    if (psy_gl_check_error(error))
        return;

    glActiveTexture(GL_TEXTURE0);
    if (psy_gl_check_error(error))
        return;

    glBindTexture(GL_TEXTURE_2D, gl_self->object_id);
    if (psy_gl_check_error(error))
        return;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    if (psy_gl_check_error(error))
        return;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    if (psy_gl_check_error(error))
        return;

    glTexParameteri(
        GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    if (psy_gl_check_error(error))
        return;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    if (psy_gl_check_error(error))
        return;

    int pix_format = 0;
    switch (num_channels) {
    case 1:
        pix_format = GL_LUMINANCE;
        break; // May break glTexImage2d below...
    case 2:
        pix_format = GL_LUMINANCE_ALPHA;
        break; // May break glTexImage2d below...
    case 3:
        pix_format = GL_RGB;
        break;
    case 4:
        pix_format = GL_RGBA;
        break;
    default:
        g_assert_not_reached();
    }
    if (pix_format == 0) {
        g_assert(pix_format != 0);
        g_critical("The texture has a unsupported number of channels");
    }

    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 pix_format,
                 width,
                 height,
                 0,
                 pix_format,
                 GL_UNSIGNED_BYTE,
                 source);
    if (psy_gl_check_error(
            error)) // If an error happens here check case 1 and 2 above.
        return;

    glGenerateMipmap(GL_TEXTURE_2D);
    if (psy_gl_check_error(
            error)) // If an error happens here check case 1 and 2 above.
        return;

    gl_self->is_uploaded = TRUE;
}

static void
psy_gl_texture_upload_image(PsyTexture *self, PsyImage *image, GError **error)
{
    PsyGlTexture *gl_self = PSY_GL_TEXTURE(self);

    PSY_TEXTURE_CLASS(psy_gl_texture_parent_class)
        ->upload_image(self, image, error);
    if (*error)
        return;

    gint  width        = (int) psy_texture_get_width(self);
    gint  height       = (int) psy_texture_get_height(self);
    guint num_channels = psy_texture_get_num_channels(self);

    if (!width) {
        g_set_error(error,
                    PSY_TEXTURE_ERROR,
                    PSY_TEXTURE_ERROR_FAILED,
                    "The width of the texture is %u",
                    width);
    }
    if (!height) {
        g_set_error(error,
                    PSY_TEXTURE_ERROR,
                    PSY_TEXTURE_ERROR_FAILED,
                    "The height of the texture is %u",
                    height);
    }
    if (!num_channels) {
        g_set_error(error,
                    PSY_TEXTURE_ERROR,
                    PSY_TEXTURE_ERROR_FAILED,
                    "The the texture num channels is %u",
                    num_channels);
    }

    if (gl_self->object_id) {
        glDeleteTextures(1, &gl_self->object_id);
        gl_self->object_id = 0;
    }

    glGenTextures(1, &gl_self->object_id);
    if (psy_gl_check_error(error))
        return;

    glActiveTexture(GL_TEXTURE0);
    if (psy_gl_check_error(error))
        return;

    glBindTexture(GL_TEXTURE_2D, gl_self->object_id);
    if (psy_gl_check_error(error))
        return;

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    if (psy_gl_check_error(error))
        return;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    if (psy_gl_check_error(error))
        return;

    glTexParameteri(
        GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    if (psy_gl_check_error(error))
        return;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    if (psy_gl_check_error(error))
        return;

    PsyImageFormat format = psy_image_get_format(image);

    if (PSY_IMAGE_FORMAT_LUM)
        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     GL_LUMINANCE,
                     width,
                     height,
                     0,
                     GL_LUMINANCE,
                     GL_UNSIGNED_BYTE,
                     psy_image_get_ptr(image));
    else if (format == PSY_IMAGE_FORMAT_RGB)
        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     GL_RGB,
                     width,
                     height,
                     0,
                     GL_RGB,
                     GL_UNSIGNED_BYTE,
                     psy_image_get_ptr(image));
    else if (format == PSY_IMAGE_FORMAT_RGBA)
        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     GL_RGBA,
                     width,
                     height,
                     0,
                     GL_RGBA,
                     GL_UNSIGNED_BYTE,
                     psy_image_get_ptr(image));
    else {
        g_assert_not_reached();
    };

    if (psy_gl_check_error(error))
        return;

    glGenerateMipmap(GL_TEXTURE_2D);
    if (psy_gl_check_error(error))
        return;

    gl_self->is_uploaded = TRUE;
}

static gboolean
psy_gl_texture_is_uploaded(PsyTexture *self)
{
    PsyGlTexture *gl_self = PSY_GL_TEXTURE(self);
    return gl_self->is_uploaded;
}

static void
psy_gl_texture_bind(PsyTexture *self, GError **error)
{
    PsyGlTexture *gl_self = PSY_GL_TEXTURE(self);

    glActiveTexture(GL_TEXTURE0);
    if (psy_gl_check_error(error))
        return;

    glBindTexture(GL_TEXTURE_2D, gl_self->object_id);
    if (psy_gl_check_error(error))
        return;
}

static void
psy_gl_texture_class_init(PsyGlTextureClass *class)
{
    GObjectClass    *gobject_class = G_OBJECT_CLASS(class);
    PsyTextureClass *texture_class = PSY_TEXTURE_CLASS(class);

    gobject_class->set_property = psy_gl_texture_set_property;
    gobject_class->get_property = psy_gl_texture_get_property;
    gobject_class->finalize     = psy_gl_texture_finalize;
    gobject_class->dispose      = psy_gl_texture_dispose;

    texture_class->upload       = psy_gl_texture_upload;
    texture_class->is_uploaded  = psy_gl_texture_is_uploaded;
    texture_class->bind         = psy_gl_texture_bind;
    texture_class->upload_image = psy_gl_texture_upload_image;

    gl_texture_properties[PROP_OBJECT_ID]
        = g_param_spec_string("object-id",
                              "Object ID",
                              "The OpenGL id of the texture",
                              0,
                              G_PARAM_READWRITE);

    g_object_class_install_properties(
        gobject_class, NUM_PROPERTIES, gl_texture_properties);
}

/* ************ public functions ******************** */

/**
 * psy_gl_texture_new:(constructor)
 *
 * Frees PsyGlTextures previously created with psy_gl_texture_new*() familiy
 * of functions.
 *
 * Returns: A new PsyGlTexture to be freed with g_object_unref of
 * psy_gl_texture_free
 */
PsyGlTexture *
psy_gl_texture_new(void)
{
    return g_object_new(PSY_TYPE_GL_TEXTURE, NULL);
}

/**
 * psy_gl_texture_new_for_file:(constructor)
 *
 * Create a new PsyGlTexture with the [property@Texture:file] specified
 * So that the texture knows about which file we are talking.
 *
 * Frees PsyGlTextures previously created with psy_gl_texture_new*() familiy
 * of function.
 *
 * Returns: A new PsyGlTexture to be freed with g_object_unref of
 * psy_gl_texture_free.
 */
PsyGlTexture *
psy_gl_texture_new_for_file(GFile *file)
{
    return g_object_new(PSY_TYPE_GL_TEXTURE, "file", file, NULL);
}

/**
 * psy_gl_texture_new_for_path:(constructor)
 *
 * Create a new PsyGlTexture with the [property@Texture:path] specified
 * So that the texture knows about which file we are talking.
 *
 * Returns: A new PsyGlTexture to be freed with g_object_unref or
 * psy_gl_texture_free.
 */
PsyGlTexture *
psy_gl_texture_new_for_path(const gchar *path)
{
    return g_object_new(PSY_TYPE_GL_TEXTURE, "path", path, NULL);
}

/**
 * psy_gl_texture_free:(skip)
 *
 * Frees PsyGlTextures previously created with psy_gl_texture_new*() familiy
 * of functions.
 */
void
psy_gl_texture_free(PsyGlTexture *self)
{
    g_return_if_fail(PSY_IS_GL_TEXTURE(self));
    g_object_unref(self);
}

/**
 * psy_gl_texture_get_object_id:
 *
 * OpenGL stores textures with an id, returned from glGenTextures. When
 * the texture exists, you'll typically first need to bind it before using it.
 * glBindTexture
 *
 * Returns: An integer that identifies this texture for opengl
 */
guint
psy_gl_texture_get_object_id(PsyGlTexture *self)
{
    g_return_val_if_fail(PSY_IS_TEXTURE(self), 0);

    return self->object_id;
}
