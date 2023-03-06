
#include "psy-picture.h"
#include "enum-types.h"

/**
 * PsyPicture:
 *
 * A psy picture is a stimulus that can display an image from a file.
 * The file chosen is set to the [property@Psy.Picture:filename] property.
 * This setter sets the key to look up a texture in the [class@DrawingContext]
 * for this stimulus. The [class@PictureArtist] will try to upload the stimulus
 * when it hasn't been uploaded, but this takes time, hence you should try
 * to use a method like [method@Psy.DrawingContext.load_files_as_texture] in
 * advance. This will load, decode and turn a file into a texture. Then drawing
 * is very fast.
 */

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

typedef enum {
    SIG_AUTO_RESIZE,
    NUM_SIGNALS,
} PictureSignals;

static GParamSpec *picture_properties[NUM_PROPERTIES] = {0};
static guint       picture_signals[NUM_SIGNALS]       = {0};

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
auto_resize(PsyPicture *picture, gfloat width, gfloat height)
{
    g_print("%s:%s %f * %f\n", __FILE__, __func__, width, height);
    psy_rectangle_set_size(PSY_RECTANGLE(picture), width, height);
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

    klass->auto_resize = auto_resize;

    /**
     * PsyPicture:filename:
     *
     * The filename property represents the filename of the image. The filename
     * is a name that is used to register a [class@Texture] with the
     * [class@DrawingContext]. So we recommend to first register the file
     * using [method@Psy.DrawingContext.load_files_as_texture] or similar
     * name. This avoids that the file needs to be loaded, decoded and Turned
     * into a PsyTexture, these methods take some time and could lead to
     * dropping frames.
     */
    picture_properties[PROP_FILENAME]
        = g_param_spec_string("filename",
                              "FileName",
                              "The name of the file to read/display",
                              "",
                              G_PARAM_READWRITE);

    /**
     * PsyPicture:size-strategy:
     *
     * This is strategy used for the initial size of the stimulus. When
     * size-strategy is `PSY_PICTURE_STRATEGY_AUTO` The size of the
     * stimulus will be set when it is drawn for the first time.
     * When someone manually changes the size, e.g. using
     * [method@Psy.Rectangle.set_width] or [method@Psy.Rectangle.set_height] it
     * will be set to `PSY_PICTURE_STRATEGY_MANUAL`.
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

    /**
     * PsyPicture::auto-resize:
     * @self:The instance of [class@Picture] on which this signal is emitted
     * @width:The width of of the [class@Texture] used as information for the
     * width
     * @height:The height of of the [class@Texture] used as information for the
     * height
     *
     * This signal is emitted when the stimulus is first rendered, the client
     * may use this signal to obtain the size of the texture that was use as
     * source for the set width and height. The class hander will update the
     * size of the picture in order to match the size of the texture. Hence,
     * If you would like change the size based on the size once known, you
     * might want to connect to this signal using `g_signal_connect_after()`
     */
    picture_signals[SIG_AUTO_RESIZE]
        = g_signal_new("auto-resize",
                       PSY_TYPE_PICTURE,
                       G_SIGNAL_RUN_LAST,
                       G_STRUCT_OFFSET(PsyPictureClass, auto_resize),
                       NULL,
                       NULL,
                       NULL,
                       G_TYPE_NONE,
                       2,
                       G_TYPE_FLOAT,
                       G_TYPE_FLOAT);
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
    // clang-format off
    return g_object_new(PSY_TYPE_PICTURE,
                        "window", window,
                        "x", x,
                        "y", y,
                        "filename",filename,
                        NULL);
    // clang-format on
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

/**
 * psy_picture_get_strategy:
 * @self: an instance of [class@Picture]
 *
 * See [property@Psy.Picture:size-strategy]
 *
 * Returns: the strategy used to auto size the picture on the next draw.
 */
PsyPictureSizeStrategy
psy_picture_get_strategy(PsyPicture *self)
{
    PsyPicturePrivate *priv = psy_picture_get_instance_private(self);

    g_return_val_if_fail(PSY_IS_PICTURE(self), PSY_PICTURE_STRATEGY_AUTOMATIC);

    return priv->strategy;
}

/**
 * psy_picture_set_strategy:
 * @self: an instance of [class@Picture]
 * @strategy: the strategy used for resizing the stimulus at next draw.
 *
 * Sets the strategy used to resize the @self
 * See [property@Psy.Picture:size-strategy]
 */
void
psy_picture_set_strategy(PsyPicture *self, PsyPictureSizeStrategy strategy)
{
    PsyPicturePrivate *priv = psy_picture_get_instance_private(self);

    g_return_if_fail(PSY_IS_PICTURE(self));

    priv->strategy = strategy;
}
