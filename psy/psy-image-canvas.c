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
#include "psy-gl-canvas.h"
#include "psy-gl-context.h"
#include "psy-timer.h"

typedef struct PsyImageCanvasPrivate {
    PsyTimePoint *time;
    PsyTimer     *iter_timer;
    gboolean      auto_iterate;
} PsyImageCanvasPrivate;

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE(PsyImageCanvas,
                                    psy_image_canvas,
                                    PSY_TYPE_CANVAS)

typedef enum {
    PROP_NULL,
    PROP_TIME,
    PROP_AUTO_ITERATE,
    N_PROPS
} PsyImageCanvasProperty;

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
    case PROP_TIME:
        psy_image_canvas_set_time(self, g_value_get_boxed(value));
        break;
    case PROP_AUTO_ITERATE:
        psy_image_canvas_set_auto_iterate(self, g_value_get_boolean(value));
        break;
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
    PsyImageCanvas        *self = PSY_IMAGE_CANVAS(object);
    PsyImageCanvasPrivate *priv = psy_image_canvas_get_instance_private(self);

    switch ((PsyImageCanvasProperty) property_id) {
    case PROP_TIME:
        g_value_take_boxed(value, psy_image_canvas_get_time(self));
        break;
    case PROP_AUTO_ITERATE:
        g_value_set_boolean(value, priv->auto_iterate);
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

    priv->time       = psy_time_point_new();
    priv->iter_timer = psy_timer_new();
}

static void
psy_image_canvas_dispose(GObject *gobject)
{
    PsyImageCanvasPrivate *priv
        = psy_image_canvas_get_instance_private(PSY_IMAGE_CANVAS(gobject));

    g_clear_object(&priv->iter_timer);

    G_OBJECT_CLASS(psy_image_canvas_parent_class)->dispose(gobject);
}

static void
psy_image_canvas_finalize(GObject *gobject)
{
    PsyImageCanvasPrivate *priv
        = psy_image_canvas_get_instance_private(PSY_IMAGE_CANVAS(gobject));

    g_clear_pointer(&priv->time, psy_time_point_free);

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
    gint64 nf = psy_canvas_get_num_frames_total(PSY_CANVAS(self));

    PsyDuration  *dur      = psy_canvas_get_frame_dur(PSY_CANVAS(self));
    PsyTimePoint *new_time = psy_time_point_add(priv->time, dur);

    psy_image_canvas_set_time(self, new_time);

    if (priv->auto_iterate) {
        psy_timer_set_fire_time(priv->iter_timer, new_time);
    }

    PSY_CANVAS_GET_CLASS(self)->draw(PSY_CANVAS(self), nf + 1, new_time);
}

/**
 * psy_image_canvas_reset:
 * @self: an instance of [class@ImageCanvas]
 *
 * Does a partial reset of the canvas, so that the time is back to zero
 * It does not reallocate images and other stuff, nor clear or touches the
 * content of the buffers.
 */
static void
image_canvas_reset(PsyCanvas *self)
{
    PSY_CANVAS_CLASS(psy_image_canvas_parent_class)->reset(self);

    PsyTimePoint *new_time = psy_time_point_new();
    psy_image_canvas_set_time(PSY_IMAGE_CANVAS(self), new_time);
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
    canvas_class->reset              = image_canvas_reset;

    klass->iterate = image_canvas_iterate;

    /**
     * PsyImageCanvas:time:
     *
     * The current time for the canvas, this timepoint is incremented with the
     * [property@Psy.Canvas:frame-dur]
     */
    obj_properties[PROP_TIME]
        = g_param_spec_boxed("time",
                             "Time",
                             "The current time for the canvas",
                             PSY_TYPE_TIME_POINT,
                             G_PARAM_READABLE);

    /**
     * PsyImageCanvas:auto-iterate:
     *
     * This property can be set, in order to automatically iterate the canvas
     * when an instance of [struct@GLib.MainLoop is running]. If set to true
     * the image canvas will automatically iterate based on a internal timer.
     * The internal time will increment with [property@Canvas:frame-dur],
     * so make sure it has a valid duration.
     */
    obj_properties[PROP_AUTO_ITERATE]
        = g_param_spec_boolean("auto-iterate",
                               "AutoIterate",
                               "Automatically iteraterate the image canvas",
                               FALSE,
                               G_PARAM_READWRITE);

    g_object_class_install_properties(object_class, N_PROPS, obj_properties);
}

/* ************ public functions ******************* */

/**
 * psy_image_canvas_new:(constructor)
 * @width: A number larger than 0
 * @height: A number larger than 0
 *
 * Creates a possibly platform specific [class@PsyImageCanvas] instance.
 * Currently it will in practice return an instance of [class@PsyGlCanvas]
 * which is a derived instance of PsyImageCanvas. That supports the same
 * methods. If in the future an instance of PsyD3dCanvas is returned on
 * windows for example is yet to be determined.
 *
 * Returns: an instance of [class@ImageCanvas]. This instance may be freed
 * with g_object_unref or psy_image_canvas_free
 */
PsyImageCanvas *
psy_image_canvas_new(gint width, gint height)
{
    g_return_val_if_fail(width > 0, NULL);
    g_return_val_if_fail(height > 0, NULL);

    return PSY_IMAGE_CANVAS(psy_gl_canvas_new(width, height));
}

/**
 * psy_image_canvas_free:(skip)
 *
 * Free canvases previously created with psy_image_canvas_new
 */
void
psy_image_canvas_free(PsyImageCanvas *self)
{
    g_return_if_fail(PSY_IS_IMAGE_CANVAS(self));
    g_object_unref(self);
}

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
 * @tp:(transfer full): An instance of [struct@PsyTimePoint]
 *
 * Sets the time for the internal time point of the ImageCanvas
 */
void
psy_image_canvas_set_time(PsyImageCanvas *self, PsyTimePoint *tp)
{
    g_return_if_fail(PSY_IS_IMAGE_CANVAS(self));
    g_return_if_fail(tp != NULL);

    PsyImageCanvasPrivate *priv = psy_image_canvas_get_instance_private(self);

    psy_time_point_free(priv->time);
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

    return psy_time_point_copy(priv->time);
}

/**
 * psy_image_canvas_set_auto_iterate:
 * @self: An instance of [class@ImageCanvas]
 * @iterate: A boolean, if set to true, the image will start to iterate, this
 *           assumes a valid frame dur has been set on the [class@Canvas].
 *
 * When this object is set, the object will start to iterate itself based on the
 * duration of the PsyCanvas. This means the [property@ImageCanvas:time] will
 * increase for each iteration.
 */
void
psy_image_canvas_set_auto_iterate(PsyImageCanvas *self, gboolean iterate)
{
    g_return_if_fail(PSY_IS_IMAGE_CANVAS(self));

    PsyImageCanvasPrivate *priv = psy_image_canvas_get_instance_private(self);

    if (priv->auto_iterate == iterate)
        return;

    if (iterate) {
        PsyDuration *frame_dur = psy_canvas_get_frame_dur(PSY_CANVAS(self));
        g_return_if_fail(frame_dur != NULL);

        PsyTimePoint *new_frame_tp = psy_time_point_add(priv->time, frame_dur);
        psy_timer_set_fire_time(priv->iter_timer, new_frame_tp);
        psy_time_point_free(new_frame_tp);
    }
    else {
        psy_timer_cancel(priv->iter_timer);
    }
    priv->auto_iterate = iterate;
}
