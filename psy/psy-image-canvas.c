/**
 * PsyImageCanvas:
 *
 * PsyImageCanvas is an abstract class. This class allows one to draw without
 * using a window. One of the key purposes of this class is that it is able
 * to render stuff without opening a window. The desire to create this class
 * is strongly to be able to do some drawing within Continuous Integration (CI)
 * environments. And to run the unit test without the need to open a window.
 * This way we can do some tests regarding the rendering procedures and
 * inspect whether the algorithms work as desired.
 */
#include "psy-image-canvas.h"
#include "psy-clock.h"
#include "psy-gl-context.h"

typedef struct PsyImageCanvasPrivate {
    PsyTimePoint *time;
} PsyImageCanvasPrivate;

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE(PsyImageCanvas,
                                    psy_image_canvas,
                                    PSY_TYPE_CANVAS)

typedef enum { PROP_NULL, PROP_TIME, N_PROPS } PsyImageCanvasProperty;

static void
psy_image_canvas_set_property(GObject      *object,
                              guint         property_id,
                              const GValue *value,
                              GParamSpec   *spec)
{
    PsyImageCanvas *self = PSY_IMAGE_CANVAS(object);
    (void) self;
    (void) value;

    switch ((PsyImageCanvasProperty) property_id) {
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, spec);
    }
}

static void
psy_image_canvas_get_property(GObject    *object,
                              guint       property_id,
                              GValue     *value,
                              GParamSpec *spec)
{
    PsyImageCanvas *self = PSY_IMAGE_CANVAS(object);

    switch ((PsyImageCanvasProperty) property_id) {
    case PROP_TIME:
        g_value_take_object(value, psy_image_canvas_get_time(self));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, spec);
    }
}

static void
psy_image_canvas_init(PsyImageCanvas *self)
{
    PsyImageCanvasPrivate *priv = psy_image_canvas_get_instance_private(self);

    PsyGlContext *drawing_context = psy_gl_context_new();
    psy_canvas_set_context(PSY_CANVAS(self),
                           PSY_DRAWING_CONTEXT(drawing_context));

    PsyClock *clk = psy_clock_new();
    priv->time    = psy_clock_now(clk);

    g_object_unref(clk);
}

static void
psy_image_canvas_dispose(GObject *gobject)
{
    PsyImageCanvasPrivate *priv
        = psy_image_canvas_get_instance_private(PSY_IMAGE_CANVAS(gobject));

    g_clear_object(&priv->time);

    G_OBJECT_CLASS(psy_image_canvas_parent_class)->dispose(gobject);
}

static void
psy_image_canvas_finalize(GObject *gobject)
{
    PsyImageCanvasPrivate *priv
        = psy_image_canvas_get_instance_private(PSY_IMAGE_CANVAS(gobject));
    (void) priv;

    G_OBJECT_CLASS(psy_image_canvas_parent_class)->finalize(gobject);
}

static void
psy_image_canvas_update_frame_stats(PsyCanvas *canvas, PsyFrameCount *count)
{
    (void) canvas;
    count->num_frames++;
    count->tot_frames++;
}

static void
image_canvas_iterate(PsyImageCanvas *self)
{
    PsyImageCanvasPrivate *priv = psy_image_canvas_get_instance_private(self);

    PsyDuration  *dur      = psy_canvas_get_frame_dur(PSY_CANVAS(self));
    PsyTimePoint *new_time = psy_time_point_add(priv->time, dur);

    psy_image_canvas_set_time(self, new_time);

    PSY_CANVAS_GET_CLASS(self)->draw(PSY_CANVAS(self), 0, new_time);
}

static GParamSpec *obj_properties[N_PROPS];

// static guint       canvas_signals[LAST_SIGNAL];

static void
psy_image_canvas_class_init(PsyImageCanvasClass *klass)
{
    GObjectClass   *object_class = G_OBJECT_CLASS(klass);
    PsyCanvasClass *canvas_class = PSY_CANVAS_CLASS(klass);

    object_class->set_property = psy_image_canvas_set_property;
    object_class->get_property = psy_image_canvas_get_property;
    object_class->dispose      = psy_image_canvas_dispose;
    object_class->finalize     = psy_image_canvas_finalize;

    canvas_class->update_frame_stats = psy_image_canvas_update_frame_stats;

    klass->iterate = image_canvas_iterate;

    /**
     * PsyImageCanvas:time:
     *
     * The current time for the canvas, this timepoint is incremented with the
     * [property@Psy.Canvas:frame-dur]
     */
    obj_properties[PROP_TIME]
        = g_param_spec_object("time",
                              "Time",
                              "The current time for the canvas",
                              PSY_TYPE_TIME_POINT,
                              G_PARAM_READABLE);

    g_object_class_install_properties(object_class, N_PROPS, obj_properties);
}

/* ************ public functions ******************* */

/**
 * psy_image_canvas_iterate:
 * @self: An instance of [class@PsyImageCanvas]
 *
 * This method performs an "iteration" of a draw cycle. The internal time of
 * the canvas is incremented with [property@PsyCanvas:frame-dur].
 */
void
psy_image_canvas_iterate(PsyImageCanvas *self)
{
    g_return_if_fail(PSY_IS_IMAGE_CANVAS(self));

    PsyImageCanvasClass *cls = PSY_IMAGE_CANVAS_GET_CLASS(self);

    cls->iterate(self);
}

/**
 * psy_image_canvas_set_time:
 * @self: An instance of [class@PsyImageCanvas]
 * @tp:(transfer full): An instance of [class@PsyTimePoint]
 *
 * Sets the time for the internal time point of the ImageCanvas
 */
void
psy_image_canvas_set_time(PsyImageCanvas *self, PsyTimePoint *tp)
{
    g_return_if_fail(PSY_IS_IMAGE_CANVAS(self));
    g_return_if_fail(PSY_IS_TIME_POINT(tp));

    PsyImageCanvasPrivate *priv = psy_image_canvas_get_instance_private(self);

    g_object_unref(priv->time);
    priv->time = tp;
}

/**
 * psy_image_canvas_get_time:
 * @self: An instance of [class@PsyImageCanvas]
 *
 * This returns a deep copy of the internal time point.
 *
 * Returns:(transfer full): The internal timestamp of the image.
 */
PsyTimePoint *
psy_image_canvas_get_time(PsyImageCanvas *self)
{
    g_return_val_if_fail(PSY_IS_IMAGE_CANVAS(self), NULL);

    PsyImageCanvasPrivate *priv = psy_image_canvas_get_instance_private(self);

    return psy_time_point_dup(priv->time);
}
