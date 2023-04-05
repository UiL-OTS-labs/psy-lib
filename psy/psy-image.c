
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "psy-image.h"

/**
 * PsyImage:
 *
 * PsyImage represents an image in memory, with a number of channels and
 * a width, height.
 * The main purpose of PsyImage is for the unit test of psylib. In Continuous
 * Integration (CI), there is most likely no window available. We do like to
 * test some of our algorithms, to see if they work as intended.
 *
 * Perhaps in the future it might be used to draw in memory and to upload
 * the stimulus to a texture and present it like that.
 *
 * Currently, we have some support for setting and getting pixel values
 * for images with 3 and 4 channels.
 */

typedef struct _PsyImage {
    GObject parent;
    guint   width, height;
    guint   num_channnels;
    guint8 *image_data;
} PsyImage;

G_DEFINE_FINAL_TYPE(PsyImage, psy_image, G_TYPE_OBJECT)

typedef enum ImageProperty {
    PROP_0,
    PROP_WIDTH,
    PROP_HEIGHT,
    PROP_NUM_CHANNELS,
    PROP_NUM_BYTES,
    PROP_STRIDE,
    NUM_PROPS,
} ImageProperty;

static GParamSpec *properties[NUM_PROPS] = {NULL};

static void
psy_image_set_property(GObject      *object,
                       guint         property_id,
                       const GValue *value,
                       GParamSpec   *pspec)
{
    PsyImage *self = PSY_IMAGE(object);

    switch ((ImageProperty) property_id) {
    case PROP_WIDTH:
        psy_image_set_width(self, g_value_get_uint(value));
        break;
    case PROP_HEIGHT:
        psy_image_set_height(self, g_value_get_uint(value));
        break;
    case PROP_NUM_CHANNELS:
        psy_image_set_num_channels(self, g_value_get_uint(value));
        break;
    default:
        /* We don't have any other property... */
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static void
psy_image_get_property(GObject    *object,
                       guint       property_id,
                       GValue     *value,
                       GParamSpec *pspec)
{
    PsyImage *self = PSY_IMAGE(object);

    switch ((ImageProperty) property_id) {
    case PROP_WIDTH:
        g_value_set_uint(value, self->width);
        break;
    case PROP_HEIGHT:
        g_value_set_uint(value, self->height);
        break;
    case PROP_NUM_CHANNELS:
        g_value_set_uint(value, self->num_channnels);
        break;
    case PROP_NUM_BYTES:
        g_value_set_uint64(value, psy_image_get_num_bytes(self));
        break;
    case PROP_STRIDE:
        g_value_set_uint(value, psy_image_get_stride(self));
        break;
    default:
        /* We don't have any other property... */
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static void
psy_image_alloc(PsyImage *self, gsize size)
{
    self->image_data = g_realloc(self->image_data, size);
}

/**
 * image_get_pixel:
 *
 * Returns a pointer to the red part of a pixel, the pixels are packed as
 * {r,g,b,a}.
 * This function assumes range checking has been done.
 *
 * Returns: a pixel in the memory of a PsyImage.
 */
static guint8 *
image_get_pixel(PsyImage *self, guint row, guint column)
{
    guint stride = psy_image_get_stride(self);
    return self->image_data + ((row * stride) + column * self->num_channnels);
}

static void
psy_image_init(PsyImage *self)
{
    (void) self;
}

static void
psy_image_finalize(GObject *object)
{
    PsyImage *self = PSY_IMAGE(object);
    g_free(self->image_data);

    G_OBJECT_CLASS(psy_image_parent_class)->finalize(object);
}

static void
psy_image_class_init(PsyImageClass *klass)
{
    GObjectClass *obj_klass = G_OBJECT_CLASS(klass);

    obj_klass->set_property = psy_image_set_property;
    obj_klass->get_property = psy_image_get_property;
    obj_klass->finalize     = psy_image_finalize;

    /**
     * PsyImage:width:
     *
     * Get or set the current width of the image. Take care that the buffer
     * representing the image will change, invalidating the current picture
     * values inside of the image.
     */
    properties[PROP_WIDTH]
        = g_param_spec_uint("width",
                            "Width",
                            "The width of the image in pixels",
                            0,
                            G_MAXUINT,
                            0,
                            G_PARAM_READWRITE);

    /**
     * PsyImage:height:
     *
     * Get or set the current height of the image. Take care that when setting,
     * the buffer representing the image will change, invalidating the current
     * picture values inside of the image.
     */
    properties[PROP_HEIGHT]
        = g_param_spec_uint("height",
                            "Height",
                            "The height of the image in pixels",
                            0,
                            G_MAXUINT,
                            0,
                            G_PARAM_READWRITE);

    /**
     * PsyImage:num-channels:
     *
     * Get or set the number of channels for each pixel. 1 channel is for
     * a grayscale/luminance picture, 3 is for rgb and 4 for rgba.
     */
    properties[PROP_NUM_CHANNELS]
        = g_param_spec_uint("num-channels",
                            "NumChannnels",
                            "The number of channels per pixel",
                            1,
                            4,
                            4,
                            G_PARAM_READWRITE);

    /**
     * PsyImage:num-bytes:
     *
     * The number of bytes that the data of this image occupies in memory.
     */
    properties[PROP_NUM_BYTES]
        = g_param_spec_uint64("num-bytes",
                              "NumBytes",
                              "The number of bytes in this image",
                              0,
                              G_MAXSIZE,
                              0,
                              G_PARAM_READABLE);

    /**
     * PsyImage:stride:
     *
     * The number of bytes that one row of pixels of this image occupies in
     * memory.
     */
    properties[PROP_STRIDE]
        = g_param_spec_uint("stride",
                            "Stride",
                            "The number of bytes per image row of pixels",
                            0,
                            G_MAXUINT,
                            0,
                            G_PARAM_READABLE);

    g_object_class_install_properties(obj_klass, NUM_PROPS, properties);
}

/* ***************** public methods ******************* */

/**
 * psy_image_new:(constructor)
 * @width: the desired width of the image
 * @height: the desired hegith of the image
 * @num_channels: The number of channels of the image.
 *
 * Create a new instance of PsyImage, with the specified dimensions in
 * pixels. For the num_channels see [method@Psy.Image.set_num_channels]
 *
 * Returns: a new instance of PsyImage.
 */
PsyImage *
psy_image_new(guint width, guint height, guint num_channnels)
{
    // clang-format off
    return g_object_new(PSY_TYPE_IMAGE,
                        "width", width,
                        "height", height,
                        "num-channels", num_channnels,
                        NULL);
    // clang-format on
}

/**
 * psy_image_set_width:
 * @self: instance of[class@Image]
 * @width: The desired width of the picture
 *
 * Sets the width of the picture. Care should be taken in that changing the
 * width of an existing image. This may affect the contents of the existing
 * image.
 */
void
psy_image_set_width(PsyImage *self, guint width)
{
    g_return_if_fail(PSY_IS_IMAGE(self));

    gsize after, start = psy_image_get_num_bytes(self);

    self->width = width;
    after       = psy_image_get_num_bytes(self);

    if (after != start) {
        psy_image_alloc(self, after);
    }
}

/**
 * psy_image_get_width:
 * @self: instance of[class@Image]
 *
 * Obtain the width in pixels of the image.
 *
 * Returns: the width of the picture in pixels.
 */
guint
psy_image_get_width(PsyImage *self)
{
    g_return_val_if_fail(PSY_IS_IMAGE(self), 0);

    return self->width;
}

/**
 * psy_image_set_height:
 * @self: instance of[class@Image]
 * @height: The desired height of the picture
 *
 * Sets the height of the picture. Care should be taken in that changing the
 * height of an existing image. This may affect the contents of the existing
 * image.
 */
void
psy_image_set_height(PsyImage *self, guint height)
{
    g_return_if_fail(PSY_IS_IMAGE(self));

    gsize after, start = psy_image_get_num_bytes(self);

    self->height = height;
    after        = psy_image_get_num_bytes(self);

    if (after != start) {
        psy_image_alloc(self, after);
    }
}

/**
 * psy_image_get_height:
 * @self: instance of[class@Image]
 *
 * Obtain the height in pixels of the image.
 *
 * Returns: the height of the picture in pixels.
 */
guint
psy_image_get_height(PsyImage *self)
{
    g_return_val_if_fail(PSY_IS_IMAGE(self), 0);

    return self->height;
}

/**
 * psy_image_set_num_channels:
 * @self: an instance of [class@PsyImage]
 * @num_channels: should be 1,3 or 4 for grayscale/luminance, RGB or RGBA
 *
 * Set the number of channels in the picture
 */
void
psy_image_set_num_channels(PsyImage *self, guint num_channels)
{
    g_return_if_fail(PSY_IS_IMAGE(self));
    g_return_if_fail(num_channels == 1 || num_channels == 3
                     || num_channels == 4);

    gsize after, start = psy_image_get_num_bytes(self);

    self->num_channnels = num_channels;
    after               = psy_image_get_num_bytes(self);

    if (after != start) {
        psy_image_alloc(self, after);
    }
}

/**
 * psy_image_get_num_channels:
 *
 * Return the number of channels (values per pixels)
 * 1, would be grayscale, 3 would be RGB and 4 RGBA
 *
 * Returns: the number of channels
 */
guint
psy_image_get_num_channels(PsyImage *self)
{
    g_return_val_if_fail(PSY_IS_IMAGE(self), 0);

    return self->num_channnels;
}

/**
 * psy_image_get_num_bytes:
 * @self: an instance of [class@Image]
 *
 * Returns the number of bytes that the picture data occupies.
 */
gsize
psy_image_get_num_bytes(PsyImage *self)
{
    g_return_val_if_fail(PSY_IS_IMAGE(self), 0);

    return self->height * psy_image_get_stride(self);
}

/**
 * psy_image_get_stride:
 * @self: an instance of [class@Image]
 *
 * Returns the number of bytes that one line would take.
 */
guint
psy_image_get_stride(PsyImage *self)
{
    g_return_val_if_fail(PSY_IS_IMAGE(self), 0);

    return self->num_channnels * self->width;
}

/**
 * psy_image_get_bytes:
 * @self: an instance of [class@Image].
 *
 * Returns the bytes that represent the image.
 *
 * Returns: an instance of [class@Glib.Bytes]
 */
GBytes *
psy_image_get_bytes(PsyImage *self)
{
    g_return_val_if_fail(PSY_IS_IMAGE(self), NULL);

    GBytes *ret = g_bytes_new(self->image_data, psy_image_get_num_bytes(self));
    return ret;
}

/**
 * psy_image_clear:
 * @self: the instance to clear
 * @color:(transfer none): the color to use to clear the background with
 *
 * Clear the image to be completely of the color @color.
 */
void
psy_image_clear(PsyImage *self, PsyColor *color)
{
    g_return_if_fail(PSY_IS_IMAGE(self));
    g_return_if_fail(PSY_IS_COLOR(color));

    gint r, g, b, a;

    r = psy_color_get_redi(color);
    g = psy_color_get_greeni(color);
    b = psy_color_get_bluei(color);
    a = psy_color_get_alphai(color);

    guint8 *data = self->image_data;
    if (self->num_channnels != 3 && self->num_channnels != 4) {
        g_critical(
            "Clearing an image with %d channels is currently not supported",
            self->num_channnels);
        return;
    }

    for (data = self->image_data;
         data < self->image_data + psy_image_get_num_bytes(self);
         data += self->num_channnels) {
        switch (self->num_channnels) {
        case 4:
            data[0] = (gint8) CLAMP(r, 0, 255);
            data[1] = (gint8) CLAMP(g, 0, 255);
            data[2] = (gint8) CLAMP(b, 0, 255);
            data[3] = (gint8) CLAMP(a, 0, 255);
            break;
        case 3:
            data[0] = (gint8) CLAMP(r, 0, 255);
            data[1] = (gint8) CLAMP(g, 0, 255);
            data[2] = (gint8) CLAMP(b, 0, 255);
            break;
        }
    }
}

/**
 * psy_image_set_pixel:
 * @self: An instance of [class@Image]
 * @row: The row of the pixel, must be smaller than [property@Psy.Image:height].
 * @column: The column of the pixel, must be smaller than
 *          [property@Psy.Image:width].
 * @color: An instance of [class@Color]. Care should be taken that the value
 *         within the color are in the range [0, 1.0] float or [0, 255] integer
 *         as Image only knows how to handle values in the range[0,255].
 *
 * Sets one pixel in @self's image data, to the color specified by the color
 * parameter. Currently this function only knows how to handle Images with
 * 3 and 4 channels.
 */
void
psy_image_set_pixel(PsyImage *self, guint row, guint column, PsyColor *color)
{
    g_return_if_fail(PSY_IS_IMAGE(self));
    g_return_if_fail(PSY_IS_COLOR(color));

    g_return_if_fail(row < self->height);
    g_return_if_fail(column < self->width);

    g_return_if_fail(self->num_channnels == 3 || self->num_channnels == 4);

    guint8 *pixel = image_get_pixel(self, row, column);

    pixel[0] = psy_color_get_redi(color);
    pixel[1] = psy_color_get_redi(color);
    pixel[2] = psy_color_get_redi(color);
    if (self->num_channnels == 4)
        pixel[3] = psy_color_get_redi(color);
}

/**
 * psy_image_get_pixel:
 * @self: An instance of [class@Image]
 * @row: The row of the pixel, must be smaller than [property@Psy.Image:height].
 * @column: The column of the pixel, must be smaller than
 *          [property@Psy.Image:width].
 *
 * Gets one pixel in @self's image data. Currently this function only knows how
 * to handle Images with 3 and 4 channels.
 *
 * Returns:(transfer full): An instance of [class@PsyColor] that represents the
 * pixels color value.
 */
PsyColor *
psy_image_get_pixel(PsyImage *self, guint row, guint column)
{
    PsyColor *color = NULL;
    g_return_val_if_fail(PSY_IS_IMAGE(self), NULL);

    g_return_val_if_fail(row < self->height, NULL);
    g_return_val_if_fail(column < self->width, NULL);

    g_return_val_if_fail(self->num_channnels == 3 || self->num_channnels == 4,
                         NULL);

    guint8 *pixel = image_get_pixel(self, row, column);
    if (self->num_channnels == 3)
        color = psy_color_new_rgbi(pixel[0], pixel[1], pixel[2]);
    else
        color = psy_color_new_rgbai(pixel[0], pixel[1], pixel[2], pixel[3]);

    return color;
}

/**
 * psy_image_save:
 * @self: An instance of [class@PsyImage]
 * @file:(transfer none): A file describing where to save the image
 * @type: A the type of image format e.g. "jpeg", "png", "ico", "bmp"
 * @error: If something goes wrong an error will be returned here.
 *
 * Save the image to a file.
 */
G_MODULE_EXPORT gboolean
psy_image_save(PsyImage *self, GFile *file, const gchar *type, GError **error)
{
    g_return_val_if_fail(PSY_IS_IMAGE(self), FALSE);
    g_return_val_if_fail(G_IS_FILE(file), FALSE);
    g_return_val_if_fail(error == NULL || *error == NULL, FALSE);

    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_data(psy_image_get_ptr(self),
                                                 GDK_COLORSPACE_RGB,
                                                 TRUE,
                                                 8,
                                                 psy_image_get_width(self),
                                                 psy_image_get_height(self),
                                                 psy_image_get_stride(self),
                                                 NULL,
                                                 NULL);

    char    *path = g_file_get_path(file);
    gboolean ret  = gdk_pixbuf_save(pixbuf, path, type, error, NULL);

    g_free(path);
    g_object_unref(pixbuf);
    return ret;
}

/**
 * psy_image_save_path:
 * @self: An instance of [class@PsyImage]
 * @path: A file describing where to save the image
 * @error: If something goes wrong an error will be returned here.
 *
 * Save the image to a file.
 */
G_MODULE_EXPORT gboolean
psy_image_save_path(PsyImage    *self,
                    const gchar *path,
                    const char  *type,
                    GError     **error)
{
    g_return_val_if_fail(PSY_IS_IMAGE(self), FALSE);
    g_return_val_if_fail(path, FALSE);

    GFile *file = g_file_new_for_path(path);

    gboolean ret = psy_image_save(self, file, type, error);
    g_object_unref(file);
    return ret;
}

/**
 * psy_image_get_ptr:(skip)
 * @self: an instance of [class@PsyImage]
 *
 * Get access to the bytes of the image.
 *
 * stability:private
 * Returns: a pointer to the data of the image.
 */
guint8 *
psy_image_get_ptr(PsyImage *self)
{
    g_return_val_if_fail(PSY_IS_IMAGE(self), NULL);

    return self->image_data;
}
