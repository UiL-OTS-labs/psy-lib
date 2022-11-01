
#include "psy-circle.h"
#include "glibconfig.h"

typedef struct _PsyCirclePrivate {
    gfloat radius;
    guint  num_vertices;
} PsyCirclePrivate;

G_DEFINE_TYPE_WITH_PRIVATE(
        PsyCircle, psy_circle, PSY_TYPE_VISUAL_STIMULUS
        )

typedef enum {
    PROP_NULL,          // not used required by GObject
    PROP_RADIUS,
    PROP_NUM_VERTICES,
    NUM_PROPERTIES
} CircleProperty;

static GParamSpec*  circle_properties[NUM_PROPERTIES] = {0};

static void
circle_set_property(GObject       *object,
                    guint          property_id,
                    const GValue  *value,
                    GParamSpec    *pspec
                    )
{
    PsyCircle* self = PSY_CIRCLE(object);

    switch((CircleProperty) property_id) {
        case PROP_RADIUS:
            psy_circle_set_radius(self, g_value_get_float(value));
            break;
        case PROP_NUM_VERTICES:
            psy_circle_set_num_vertices(self, g_value_get_uint(value));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

static void
circle_get_property(GObject       *object,
                    guint          property_id,
                    GValue        *value,
                    GParamSpec    *pspec
                    )
{
    PsyCircle* self = PSY_CIRCLE(object);
    PsyCirclePrivate* priv = psy_circle_get_instance_private(self);

    switch((CircleProperty) property_id) {
        case PROP_RADIUS:
            g_value_set_float(value, priv->radius);
            break;
        case PROP_NUM_VERTICES:
            g_value_set_uint(value, priv->num_vertices);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

static void
psy_circle_init(PsyCircle* self)
{
    PsyCirclePrivate *priv = psy_circle_get_instance_private(self);
    priv->num_vertices = 3;
    priv->radius       = 1;
}

static void
psy_circle_class_init(PsyCircleClass* klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    object_class->get_property = circle_get_property;
    object_class->set_property = circle_set_property;

    /**
     * Circle:radius:
     *
     * This is the radius of the circle
     */
    circle_properties[PROP_RADIUS] = g_param_spec_float (
            "radius",
            "Radius",
            "The radius of the circle",
            0.f,
            G_MAXFLOAT,
            10,
            G_PARAM_READWRITE | G_PARAM_CONSTRUCT
            );

    /**
     * Circle:num-vertices:
     *
     * The number of vertices used for the outside of the circle. At least
     * three vertices are required. There is also one in the center of the circle
     * but that one is counted separately
     */
    circle_properties[PROP_NUM_VERTICES] = g_param_spec_uint(
            "num-vertices",
            "NumVertices",
            "The number vertices used for the outside border of the circle",
            3, G_MAXINT, 25,
            G_PARAM_READWRITE | G_PARAM_CONSTRUCT
            );

    g_object_class_install_properties(
            object_class, NUM_PROPERTIES, circle_properties 
            );

}

/**
 * psy_circle_new:(constructor)
 * @window: an instance of `PsyWindow` on which this stimulus should be drawn
 *
 * Returns: a new instance of `PsyCircle` with default values.
 */
PsyCircle*
psy_circle_new(PsyWindow* window)
{
    return g_object_new(PSY_TYPE_CIRCLE,
           "window", window,
           NULL);
}

/**
 * psy_circle:(method)
 * @circle: an instance of `PsyCircle`
 *
 * Returns: a new instance of `PsyCircle` with the provided values.
 */
PsyCircle*
psy_circle_new_full(
        PsyWindow* window, gfloat x, gfloat y, gfloat radius, guint num_vertices
        )
{
    return g_object_new(
            PSY_TYPE_CIRCLE,
            "window", window,
            "x", x,
            "y", y,
            "radius", radius,
            "num_vertices", num_vertices,
            NULL);
}

/**
 * psy_circle_set_radius:
 * @circle: an instance of %PsyCircle
 * @radius: a postive number that is the radius of the circle
 *
 * Set the radius of the circle
 */
void
psy_circle_set_radius(PsyCircle* circle, gfloat radius)
{
    g_return_if_fail(PSY_IS_CIRCLE(circle));
    g_return_if_fail(radius >= 0.0);

    PsyCirclePrivate* priv = psy_circle_get_instance_private(circle);
    priv->radius = radius;
}

/**
 * psy_circle_get_radius:
 * @circle: an instance of %PsyCircle
 *
 * Returns: The radius of the circle.
 */
gfloat
psy_circle_get_radius(PsyCircle* circle)
{
    g_return_val_if_fail(PSY_IS_CIRCLE(circle), 0.0);
    PsyCirclePrivate* priv = psy_circle_get_instance_private(circle);
    return priv->radius;
}

/**
 * psy_circle_set_num_vertices:
 * @circle: an instance of %PsyCircle
 * @num_vertices: The number of vertices in order to draw the circle must be
 *                larger or equal to 3.
 *
 * Set the number of vertices that is used to approach the circle more vertices
 * creates a rounder circle but is a bit less efficient to draw.
 */
void
psy_circle_set_num_vertices(PsyCircle* circle, guint num_vertices)
{
    g_return_if_fail(PSY_IS_CIRCLE(circle));
    g_return_if_fail(num_vertices >= 3);

    PsyCirclePrivate* priv = psy_circle_get_instance_private(circle);
    priv->num_vertices = num_vertices;
}

/**
 * psy_circle_get_num_vertices:
 *
 * Set the number of vertices that is used to approach the circle, more vertices
 * creates a rounder/nicer circle but is a bit less efficient to draw.
 *
 * Returns: the number of vertices in order to approach the outline of a circle.
 */
guint
psy_circle_get_num_vertices(PsyCircle* circle)
{
    g_return_val_if_fail(PSY_IS_CIRCLE(circle), 0);
    PsyCirclePrivate* priv = psy_circle_get_instance_private(circle);
    return priv->num_vertices;
}

