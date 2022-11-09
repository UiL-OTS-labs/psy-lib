
#include "psy-cross.h"
#include "glibconfig.h"

typedef struct _PsyCrossPrivate {
    gfloat x_length;
    gfloat y_length;
    gfloat x_width;
    gfloat y_width;
} PsyCrossPrivate;

G_DEFINE_TYPE_WITH_PRIVATE(
        PsyCross, psy_cross, PSY_TYPE_VISUAL_STIMULUS
        )

typedef enum {
    PROP_NULL,          // not used required by GObject
    PROP_LENGTH,
    PROP_X_LENGTH,
    PROP_Y_LENGTH,
    PROP_WIDTH,
    PROP_X_WIDTH,
    PROP_Y_WIDTH,
    NUM_PROPERTIES
} CrossProperty;

static GParamSpec*  cross_properties[NUM_PROPERTIES] = {0};

static void
cross_set_property(GObject       *object,
                    guint          property_id,
                    const GValue  *value,
                    GParamSpec    *pspec
                    )
{
    PsyCross* self = PSY_CROSS(object);

    switch((CrossProperty) property_id) {
        case PROP_LENGTH:
            psy_cross_set_line_length_x(self, g_value_get_float(value));
            psy_cross_set_line_length_y(self, g_value_get_float(value));
            break;
        case PROP_X_LENGTH:
            psy_cross_set_line_length_x(self, g_value_get_float(value));
            break;
        case PROP_Y_LENGTH:
            psy_cross_set_line_length_y(self, g_value_get_float(value));
            break;
        case PROP_WIDTH:
            psy_cross_set_line_width_x(self, g_value_get_float(value));
            psy_cross_set_line_width_y(self, g_value_get_float(value));
            break;
        case PROP_X_WIDTH:
            psy_cross_set_line_width_x(self, g_value_get_float(value));
            break;
        case PROP_Y_WIDTH:
            psy_cross_set_line_width_y(self, g_value_get_float(value));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

static void
cross_get_property(GObject       *object,
                    guint          property_id,
                    GValue        *value,
                    GParamSpec    *pspec
                    )
{
    PsyCross* self = PSY_CROSS(object);
    PsyCrossPrivate* priv = psy_cross_get_instance_private(self);

    switch((CrossProperty) property_id) {
        case PROP_X_LENGTH:
            g_value_set_float(value, priv->x_length);
            break;
        case PROP_Y_LENGTH:
            g_value_set_float(value, priv->y_length);
            break;
        case PROP_X_WIDTH:
            g_value_set_float(value, priv->x_width);
            break;
        case PROP_Y_WIDTH:
            g_value_set_float(value, priv->y_width);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

static void
psy_cross_init(PsyCross* self)
{
    (void) self;
}

static void
psy_cross_class_init(PsyCrossClass* klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    object_class->get_property = cross_get_property;
    object_class->set_property = cross_set_property;

    /**
     * Cross:line-length:
     *
     * This is the length of the cross along the x- and y-axis
     * if you want to use different length use the `cross:line-length-x` and
     * `cross:line-length-y` properties.
     * You can't get the line as this property is a shortcut to set
     * both the length of the x and y axis.
     */
    cross_properties[PROP_LENGTH] = g_param_spec_float (
            "line-length",
            "Line Length",
            "The length of the line along x and y axis",
            0.f,
            G_MAXFLOAT,
            10,
            G_PARAM_WRITABLE | G_PARAM_CONSTRUCT
            );

    /**
     * Cross:line-length-x:
     *
     * The length of the cross along the x-axis
     */
    cross_properties[PROP_X_LENGTH] = g_param_spec_float(
            "line-length-x",
            "Line Length X",
            "The length of the line along the x-axis",
            0.0,
            G_MAXFLOAT,
            0,
            G_PARAM_READWRITE
            );
    
    /**
     * Cross:line-length-y:
     *
     * The length of the cross along the y-axis
     */
    cross_properties[PROP_Y_LENGTH] = g_param_spec_float(
            "line-length-y",
            "Line Length Y",
            "The length of the line along the y-axis",
            0.0,
            G_MAXFLOAT,
            0,
            G_PARAM_READWRITE
            );
    
    /**
     * Cross:line-width:
     *
     * This is the line width of the cross along the x- and y-axis
     * if you want to use different width use the `cross:line-width-x` and
     * `cross:line-width-y` properties
     */
    cross_properties[PROP_WIDTH] = g_param_spec_float (
            "line-width",
            "Line Width",
            "The width of the lines of the cross along x and y axis",
            0.f,
            G_MAXFLOAT,
            3,
            G_PARAM_WRITABLE | G_PARAM_CONSTRUCT
            );

    /**
     * Cross:line-width-x:
     *
     * The width of the cross along the x-axis
     */
    cross_properties[PROP_X_WIDTH] = g_param_spec_float(
            "line-width-x",
            "Line Width X",
            "The width of the line along the x-axis",
            0.0, G_MAXFLOAT, 0,
            G_PARAM_READWRITE
            );
    
    /**
     * Cross:line-width-y:
     *
     * The width of the cross along the y-axis
     */
    cross_properties[PROP_Y_WIDTH] = g_param_spec_float(
            "line-width-y",
            "Line Width Y",
            "The width of the line along the y-axis",
            0.0, G_MAXFLOAT, 0,
            G_PARAM_READWRITE
            );

    g_object_class_install_properties(
            object_class, NUM_PROPERTIES, cross_properties 
            );

}

/**
 * psy_cross_new:(constructor)
 * @window: an instance of `PsyWindow` on which this stimulus should be drawn
 *
 * Returns: a new instance of `PsyCross` with default values.
 */
PsyCross*
psy_cross_new(PsyWindow* window)
{
    return g_object_new(PSY_TYPE_CROSS,
           "window", window,
           NULL);
}

/**
 * psy_cross:(method)
 * @cross: an instance of `PsyCross`
 *
 * Returns: a new instance of `PsyCross` with the provided values.
 */
PsyCross*
psy_cross_new_full(
        PsyWindow* window, gfloat x, gfloat y, gfloat length, gfloat line_width 
        )
{
    return g_object_new(
            PSY_TYPE_CROSS,
            "window", window,
            "x", x,
            "y", y,
            "line-length", length,
            "line-width", line_width,
            NULL);
}

/**
 * psy_cross_set_line_length_x:
 * @cross: an instance of %PsyCross
 * @length: a positive number that is the desired length along the x-axis
 * 
 * Set the radius of the cross
 */
void
psy_cross_set_line_length_x(PsyCross* cross, gfloat length)
{
    g_return_if_fail(PSY_IS_CROSS(cross));
    g_return_if_fail(length >= 0.0);

    PsyCrossPrivate* priv = psy_cross_get_instance_private(cross);
    priv->x_length = length;
}

/**
 * psy_cross_get_line_length_x:
 * @cross: an instance of %PsyCross
 *
 * Returns: The length of the cross along the x axis.
 */
gfloat
psy_cross_get_line_length_x(PsyCross* cross)
{
    g_return_val_if_fail(PSY_IS_CROSS(cross), 0.0);
    PsyCrossPrivate* priv = psy_cross_get_instance_private(cross);
    return priv->x_length;
}

/**
 * psy_cross_set_line_length_y:
 * @cross: an instance of %PsyCross
 * @length: a positive number that is the desired length along the y-axis
 * 
 * Set the line length of the cross along the x axis
 */
void
psy_cross_set_line_length_y(PsyCross* cross, gfloat length)
{
    g_return_if_fail(PSY_IS_CROSS(cross));
    g_return_if_fail(length >= 0.0);

    PsyCrossPrivate* priv = psy_cross_get_instance_private(cross);
    priv->y_length = length;
}

/**
 * psy_cross_get_line_length_y:
 * @cross: an instance of %PsyCross
 *
 * Returns: The length of the cross along the y axis.
 */
gfloat
psy_cross_get_line_length_y(PsyCross* cross)
{
    g_return_val_if_fail(PSY_IS_CROSS(cross), 0.0);
    PsyCrossPrivate* priv = psy_cross_get_instance_private(cross);
    return priv->y_length;
}

/**
 * psy_cross_set_line_width_x:
 * @cross: an instance of %PsyCross
 * @width: a positive number that is the desired width along the x-axis
 * 
 * Set the radius of the cross
 */
void
psy_cross_set_line_width_x(PsyCross* cross, gfloat width)
{
    g_return_if_fail(PSY_IS_CROSS(cross));
    g_return_if_fail(width >= 0.0);

    PsyCrossPrivate* priv = psy_cross_get_instance_private(cross);
    priv->x_width = width;
}

/**
 * psy_cross_get_line_width_x:
 * @cross: an instance of %PsyCross
 *
 * Returns: The width of the cross along the x axis.
 */
gfloat
psy_cross_get_line_width_x(PsyCross* cross)
{
    g_return_val_if_fail(PSY_IS_CROSS(cross), 0.0);
    PsyCrossPrivate* priv = psy_cross_get_instance_private(cross);
    return priv->x_width;
}

/**
 * psy_cross_set_line_width_y:
 * @cross: an instance of %PsyCross
 * @width: a positive number that is the desired width along the y-axis
 * 
 * Set the radius of the cross
 */
void
psy_cross_set_line_width_y(PsyCross* cross, gfloat width)
{
    g_return_if_fail(PSY_IS_CROSS(cross));
    g_return_if_fail(width >= 0.0);

    PsyCrossPrivate* priv = psy_cross_get_instance_private(cross);
    priv->y_width = width;
}

/**
 * psy_cross_get_line_width_y:
 * @cross: an instance of %PsyCross
 *
 * Returns: The width of the cross along the y axis.
 */
gfloat
psy_cross_get_line_width_y(PsyCross* cross)
{
    g_return_val_if_fail(PSY_IS_CROSS(cross), 0.0);
    PsyCrossPrivate* priv = psy_cross_get_instance_private(cross);
    return priv->y_width;
}


