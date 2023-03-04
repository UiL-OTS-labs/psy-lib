
#include "psy-picture.h"
#include "enum-types.h"

typedef struct _PsyPicturePrivate {
    gchar                 *fn;
    PsyPictureSizeStrategy strategy;
} PsyPicturePrivate;

G_DEFINE_TYPE_WITH_PRIVATE(PsyPicture, psy_picture, PSY_TYPE_RECTANGLE)

typedef enum {
    PROP_NULL, // not used required by GObject
    PROP_FILENAME,
    PROP_STRATEGY,
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
    case PROP_STRATEGY:
        psy_picture_set_strategy(self, g_value_get_enum(value));
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
    case PROP_STRATEGY:
        g_value_set_enum(value, priv->strategy);
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
set_width(PsyRectangle *self, gfloat width)
{
    PSY_RECTANGLE_CLASS(psy_picture_parent_class)->set_width(self, width);

    PsyPicturePrivate *priv
        = psy_picture_get_instance_private(PSY_PICTURE(self));
    priv->strategy = PSY_PICTURE_STRATEGY_MANUAL;
}

static void
set_height(PsyRectangle *self, gfloat height)
{
    PSY_RECTANGLE_CLASS(psy_picture_parent_class)->set_height(self, height);

    PsyPicturePrivate *priv
        = psy_picture_get_instance_private(PSY_PICTURE(self));
    priv->strategy = PSY_PICTURE_STRATEGY_MANUAL;
}

static void
psy_picture_class_init(PsyPictureClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    object_class->get_property = picture_get_property;
    object_class->set_property = picture_set_property;
    object_class->finalize     = psy_picture_finalize;

    PsyRectangleClass *rect_class = PSY_RECTANGLE_CLASS(klass);
    rect_class->set_width         = set_width;
    rect_class->set_height        = set_height;

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

    /**
     * Picture:size-strategy:
     *
     * This is strategy used for the initial size of the stimulus. When
     * size-strategy is [enum@PSY_PICTURE_STRATEGY_AUTO] The size of the
     * stimulus will be set when it is drawn for the first time.
     * When someone manually changes the size, e.g. using
     * [method@VisualStimulus.set_width] or [method@VisualStimulus.set_height]
     * it will be set to PSY_PICTURE_STRATEGY_MANUAL.
     */
    picture_properties[PROP_STRATEGY]
        = g_param_spec_enum("size-strategy",
                            "SizeStrategy",
                            "Sets the size strategy of this picture",
                            psy_picture_size_strategy_get_type(),
                            PSY_PICTURE_STRATEGY_AUTOMATIC,
                            G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

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
 * psy_picture_new_filename:(constructor)
 * @window: an instance of `PsyWindow` on which this stimulus should be drawn
 * @filename: the name of the file to load as image
 *
 * Returns: a new instance of `PsyPicture` with default values.
 */
PsyPicture *
psy_picture_new_filename(PsyWindow *window, const gchar *filename)
{
    return g_object_new(
        PSY_TYPE_PICTURE, "window", window, "filename", filename, NULL);
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
psy_picture_new_xy_filename(PsyWindow   *window,
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

PsyPictureSizeStrategy
psy_picture_get_strategy(PsyPicture *self)
{
    PsyPicturePrivate *priv = psy_picture_get_instance_private(self);

    g_return_val_if_fail(PSY_IS_PICTURE(self), PSY_PICTURE_STRATEGY_AUTOMATIC);

    return priv->strategy;
}

void
psy_picture_set_strategy(PsyPicture *self, PsyPictureSizeStrategy strategy)
{
    PsyPicturePrivate *priv = psy_picture_get_instance_private(self);

    g_return_if_fail(PSY_IS_PICTURE(self));

    priv->strategy = strategy;
}
