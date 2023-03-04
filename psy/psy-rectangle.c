
#include "psy-rectangle.h"
#include "glibconfig.h"

typedef struct _PsyRectanglePrivate {
    gfloat width;
    gfloat height;
} PsyRectanglePrivate;

G_DEFINE_TYPE_WITH_PRIVATE(PsyRectangle,
                           psy_rectangle,
                           PSY_TYPE_VISUAL_STIMULUS)

typedef enum {
    PROP_NULL, // not used required by GObject
    PROP_WIDTH,
    PROP_HEIGHT,
    NUM_PROPERTIES
} RectangleProperty;

static GParamSpec *rectangle_properties[NUM_PROPERTIES] = {0};

static void
rectangle_set_property(GObject      *object,
                       guint         property_id,
                       const GValue *value,
                       GParamSpec   *pspec)
{
    PsyRectangle *self = PSY_RECTANGLE(object);

    switch ((RectangleProperty) property_id) {
    case PROP_WIDTH:
        psy_rectangle_set_width(self, g_value_get_float(value));
        break;
    case PROP_HEIGHT:
        psy_rectangle_set_height(self, g_value_get_float(value));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

static void
rectangle_get_property(GObject    *object,
                       guint       property_id,
                       GValue     *value,
                       GParamSpec *pspec)
{
    PsyRectangle        *self = PSY_RECTANGLE(object);
    PsyRectanglePrivate *priv = psy_rectangle_get_instance_private(self);

    switch ((RectangleProperty) property_id) {
    case PROP_WIDTH:
        g_value_set_float(value, priv->width);
        break;
    case PROP_HEIGHT:
        g_value_set_float(value, priv->height);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

static void
psy_rectangle_init(PsyRectangle *self)
{
    PsyRectanglePrivate *priv = psy_rectangle_get_instance_private(self);
    (void) priv;
}

static void
set_width(PsyRectangle *self, gfloat width)
{
    PsyRectanglePrivate *priv = psy_rectangle_get_instance_private(self);

    priv->width = width;
}

static void
set_height(PsyRectangle *self, gfloat height)
{
    PsyRectanglePrivate *priv = psy_rectangle_get_instance_private(self);

    priv->height = height;
}

static void
psy_rectangle_class_init(PsyRectangleClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    object_class->get_property = rectangle_get_property;
    object_class->set_property = rectangle_set_property;

    klass->set_width  = set_width;
    klass->set_height = set_height;

    /**
     * Rectangle:width:
     *
     * This is the width of the rectangle. The width may be negative, which
     * will reflect the image around the y-axis, however, it's recommended
     * to use psy_visual_stimulus_set_scale_x() to a negative value instead.
     */
    rectangle_properties[PROP_WIDTH]
        = g_param_spec_float("width",
                             "Width",
                             "The width of the rectangle",
                             -G_MAXFLOAT,
                             G_MAXFLOAT,
                             0,
                             G_PARAM_READWRITE);

    /**
     * Rectangle:height:
     *
     * The number of vertices used for the outside of the rectangle. At least
     * three vertices are required. There is also one in the center of the
     * rectangle but that one is counted separately
     */
    rectangle_properties[PROP_HEIGHT]
        = g_param_spec_float("height",
                             "Height",
                             "The height of the rectangle",
                             -G_MAXFLOAT,
                             G_MAXFLOAT,
                             0,
                             G_PARAM_READWRITE);

    g_object_class_install_properties(
        object_class, NUM_PROPERTIES, rectangle_properties);
}

/**
 * psy_rectangle_new:(constructor)
 * @window: an instance of `PsyWindow` on which this stimulus should be drawn
 *
 * Returns: a new instance of `PsyRectangle` with default values.
 */
PsyRectangle *
psy_rectangle_new(PsyWindow *window)
{
    return g_object_new(PSY_TYPE_RECTANGLE, "window", window, NULL);
}

/**
 * psy_rectangle_new_full:(constructor)
 * @window:the window on which we would like to draw this rectangle
 * @x:the x position of the center of the rectangle
 * @y:the y position of the center of the rectangle
 * @width: the width of the rectangle (along y-axis)
 * @height: the height of the rectangle along(x-axis)
 *
 * Returns: a new instance of `PsyRectangle` with the provided values.
 */
PsyRectangle *
psy_rectangle_new_full(
    PsyWindow *window, gfloat x, gfloat y, gfloat width, gfloat height)
{
    // clang-format off
    return g_object_new(
            PSY_TYPE_RECTANGLE,
            "window", window,
            "x", x,
            "y", y,
            "width", width,
            "height", height,
            NULL);
    // clang-format on
}

/**
 * psy_rectangle_set_width:
 * @self: an instance of [class@PsyRectangle]
 * @width: a positive number that is the width of the rectangle
 *
 * Set the width of the rectangle
 */
void
psy_rectangle_set_width(PsyRectangle *self, gfloat width)
{
    g_return_if_fail(PSY_IS_RECTANGLE(self));

    PsyRectangleClass *cls = PSY_RECTANGLE_GET_CLASS(self);
    g_assert(cls->set_width);
    cls->set_width(self, width);
}

/**
 * psy_rectangle_get_width:
 * @self: an instance of [class@PsyRectangle]
 *
 * Returns: The width of the rectangle.
 */
gfloat
psy_rectangle_get_width(PsyRectangle *self)
{
    g_return_val_if_fail(PSY_IS_RECTANGLE(self), 0.0);
    PsyRectanglePrivate *priv = psy_rectangle_get_instance_private(self);
    return priv->width;
}

/**
 * psy_rectangle_set_height:
 * @self: an instance of [class@PsyRectangle]
 * @height: a positive number that is the height of the rectangle
 *
 * Set the height of the rectangle
 */
void
psy_rectangle_set_height(PsyRectangle *self, gfloat height)
{
    g_return_if_fail(PSY_IS_RECTANGLE(self));

    PsyRectangleClass *cls = PSY_RECTANGLE_GET_CLASS(self);
    g_assert(cls->set_width);
    cls->set_height(self, height);
}

/**
 * psy_rectangle_get_height:
 * @self: an instance of [class@PsyRectangle]
 *
 * Returns: The height of the rectangle.
 */
gfloat
psy_rectangle_get_height(PsyRectangle *self)
{
    g_return_val_if_fail(PSY_IS_RECTANGLE(self), 0.0);
    PsyRectanglePrivate *priv = psy_rectangle_get_instance_private(self);
    return priv->height;
}
