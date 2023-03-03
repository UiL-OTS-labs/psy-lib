
#include "psy-picture.h"

typedef struct _PsyPicturePrivate {
    gchar *fn;
} PsyPicturePrivate;

G_DEFINE_TYPE_WITH_PRIVATE(PsyPicture, psy_picture, PSY_TYPE_RECTANGLE)

typedef enum {
    PROP_NULL, // not used required by GObject
    PROP_FILENAME,
    NUM_PROPERTIES
} PictureProperty;

static GParamSpec *picture_properties[NUM_PROPERTIES] = {0};

static void
picture_set_property(GObject      *object,
                     guint         property_id,
                     const GValue *value,
                     GParamSpec   *pspec)
{
    PsyPicture *self = PSY_PICTURE(object);

    switch ((PictureProperty) property_id) {
    case PROP_FILENAME:
        psy_picture_set_filename(self, g_value_get_string(value));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

static void
picture_get_property(GObject    *object,
                     guint       property_id,
                     GValue     *value,
                     GParamSpec *pspec)
{
    PsyPicture        *self = PSY_PICTURE(object);
    PsyPicturePrivate *priv = psy_picture_get_instance_private(self);

    switch ((PictureProperty) property_id) {
    case PROP_FILENAME:
        g_value_set_string(value, priv->fn);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

static void
psy_picture_init(PsyPicture *self)
{
    PsyPicturePrivate *priv = psy_picture_get_instance_private(self);
    (void) priv;
}

static void
psy_picture_finalize(GObject *obj)
{
    PsyPicture        *self = PSY_PICTURE(obj);
    PsyPicturePrivate *priv = psy_picture_get_instance_private(self);

    g_clear_pointer(&priv->fn, g_free);
}

static void
psy_picture_class_init(PsyPictureClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    object_class->get_property = picture_get_property;
    object_class->set_property = picture_set_property;
    object_class->finalize     = psy_picture_finalize;

    /**
     * Picture:filename:
     *
     * This is the width of the picture
     */
    picture_properties[PROP_FILENAME]
        = g_param_spec_string("filename",
                              "FileName",
                              "The name of the file to read/display",
                              "",
                              G_PARAM_READWRITE);

    g_object_class_install_properties(
        object_class, NUM_PROPERTIES, picture_properties);
}

/**
 * psy_picture_new:(constructor)
 * @window: an instance of `PsyWindow` on which this stimulus should be drawn
 *
 * Returns: a new instance of `PsyPicture` with default values.
 */
PsyPicture *
psy_picture_new(PsyWindow *window)
{
    return g_object_new(PSY_TYPE_PICTURE, "window", window, NULL);
}

/**
 * psy_picture_new_xy_name:(constructor)
 * @window: an instance of `PsyWindow` on which this stimulus should be drawn
 * @x: The x coordinate of the window
 * @y: The y coordinate of the window
 * @filename: the name of the file to load as image
 *
 * Returns: a new instance of `PsyPicture` with default values.
 */
PsyPicture *
psy_picture_new_xy_name(PsyWindow   *window,
                        gfloat       x,
                        gfloat       y,
                        const gchar *filename)
{
    return g_object_new(PSY_TYPE_PICTURE,
                        "window",
                        window,
                        "x",
                        x,
                        "y",
                        y,
                        "filename",
                        filename,
                        NULL);
}

/**
 * psy_picture_new_full:(constructor)
 * @window:the window on which we would like to draw this picture
 * @x:the x position of the center of the picture
 * @y:the y position of the center of the picture
 * @width: the width of the picture (along y-axis)
 * @height: the height of the picture along(x-axis)
 * @filename: the filename of the stimulus
 *
 * Returns: a new instance of [class@Picture] with the provided values.
 */
PsyPicture *
psy_picture_new_full(PsyWindow   *window,
                     gfloat       x,
                     gfloat       y,
                     gfloat       width,
                     gfloat       height,
                     const gchar *filename)
{
    // clang-format off
    return g_object_new(
            PSY_TYPE_PICTURE,
            "window", window,
            "x", x,
            "y", y,
            "width", width,
            "height", height,
            "filename", filename,
            NULL);
    // clang-format on
}

/**
 * psy_picture_set_filename:
 * @self: An Instance of [class@Picture]
 * @filename: a utf8 encoded filename that specifies a file to open.
 *
 * Sets the filename that belongs to this stimulus. The [class@PictureArtist]
 * will try to load this filename in order to draw the picture.
 */
void
psy_picture_set_filename(PsyPicture *self, const gchar *filename)
{
    PsyPicturePrivate *priv = psy_picture_get_instance_private(self);

    g_return_if_fail(PSY_IS_PICTURE(self));
    g_return_if_fail(filename != NULL);

    GFile *file = g_file_new_for_path(filename);

    g_free(priv->fn);
    priv->fn = g_file_get_path(file);
    g_object_unref(file);
}

/**
 * psy_picture_get_filename:
 * @self: An Instance of [class@Picture]
 *
 * Gets the filename that belongs to this picture.
 *
 * Returns:(nullable): the filename for this picture.
 */
const gchar *
psy_picture_get_filename(PsyPicture *self)
{
    PsyPicturePrivate *priv = psy_picture_get_instance_private(self);

    g_return_val_if_fail(PSY_IS_PICTURE(self), NULL);
    return priv->fn;
}
