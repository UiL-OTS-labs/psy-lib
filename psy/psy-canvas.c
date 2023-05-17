/**
 * PsyCanvas:
 *
 * PsyCanvas is an abstract class. It provides an base class for rectangular
 * arrays or picture on which we can draw with some stimuli with a specific
 * backend to be determined by deriving classes. The Rectangular array might be
 * a Picture, a Window or even a File.
 *
 * The canvas starts the drawing procedure of the canvas. Drawing occurs in
 * a number of phases.
 *
 * 1. Clear the background.
 * 2. Draw the stimuli.
 *    The [vfunc@Psy.Canvas.draw_stimuli] method should do two things:
 *      1. It will check the scheduled stimuli, to see whether there is
 *         a stimulus ready to present.
 *      2. It will allow the client to update the stimuli that should be
 *         presented and makes sure that the `PsyArtist`s will actually draw
 *         every stimulus.
 */

#include <math.h>

#include "enum-types.h"
#include "psy-artist.h"
#include "psy-canvas.h"
#include "psy-circle-artist.h"
#include "psy-circle.h"
#include "psy-color.h"
#include "psy-cross-artist.h"
#include "psy-cross.h"
#include "psy-drawing-context.h"
#include "psy-duration.h"
#include "psy-matrix4.h"
#include "psy-picture-artist.h"
#include "psy-picture.h"
#include "psy-program.h"
#include "psy-rectangle-artist.h"
#include "psy-rectangle.h"
#include "psy-stimulus.h"
#include "psy-time-point.h"
#include "psy-visual-stimulus.h"

typedef struct PsyCanvasPrivate {
    gint width, height;
    gint width_mm, height_mm;
    gint distance_mm;

    PsyFrameCount frame_count;

    PsyColor          *background_color;
    GPtrArray         *stimuli; // owns a ref on the PsyStimulus
    GHashTable        *artists; // owns a ref on the PsyStimulus and PsyArtist
    PsyDuration       *frame_dur;
    PsyDrawingContext *context;

    gint        projection_style;
    PsyMatrix4 *projection_matrix;
} PsyCanvasPrivate;

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE(PsyCanvas, psy_canvas, G_TYPE_OBJECT)

typedef enum {
    CLEAR,
    DRAW_STIMULI,
    RESIZE,
    LAST_SIGNAL,
} PsyCanvasSignal;

typedef enum {
    BACKGROUND_COLOR = 1,
    WIDTH,
    HEIGHT,
    WIDTH_MM,
    HEIGHT_MM,
    DISTANCE_MM,
    WIDTH_VISUAL_DEGREES,
    HEIGHT_VISUAL_DEGREES,
    FRAME_DUR,
    PROJECTION_STYLE,
    CONTEXT,
    NUM_STIMULI,
    N_PROPS
} PsyCanvasProperty;

static void
psy_canvas_set_property(GObject      *object,
                        guint         property_id,
                        const GValue *value,
                        GParamSpec   *spec)
{
    PsyCanvas *self = PSY_CANVAS(object);

    switch ((PsyCanvasProperty) property_id) {
    case WIDTH:
        psy_canvas_set_width(self, g_value_get_int(value));
        break;
    case HEIGHT:
        psy_canvas_set_height(self, g_value_get_int(value));
        break;
    case BACKGROUND_COLOR:
    {
        PsyColor *color = g_value_get_object(value);
        g_assert(PSY_IS_COLOR(color));
        psy_canvas_set_background_color(self, color);
    } break;
    case WIDTH_MM:
        psy_canvas_set_width_mm(self, g_value_get_int(value));
        break;
    case HEIGHT_MM:
        psy_canvas_set_height_mm(self, g_value_get_int(value));
        break;
    case DISTANCE_MM:
        psy_canvas_set_distance_mm(self, g_value_get_int(value));
        break;
    case PROJECTION_STYLE:
        psy_canvas_set_projection_style(self, g_value_get_int(value));
        break;
    case FRAME_DUR:
    case NUM_STIMULI:
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, spec);
    }
}

static void
psy_canvas_get_property(GObject    *object,
                        guint       property_id,
                        GValue     *value,
                        GParamSpec *spec)
{
    PsyCanvas *self = PSY_CANVAS(object);

    switch ((PsyCanvasProperty) property_id) {
    case BACKGROUND_COLOR:
        g_value_set_object(value, psy_canvas_get_background_color(self));
        break;
    case WIDTH:
        g_value_set_int(value, psy_canvas_get_width(self));
        break;
    case HEIGHT:
        g_value_set_int(value, psy_canvas_get_height(self));
        break;
    case WIDTH_MM:
        g_value_set_int(value, psy_canvas_get_width_mm(self));
        break;
    case HEIGHT_MM:
        g_value_set_int(value, psy_canvas_get_height_mm(self));
        break;
    case DISTANCE_MM:
        g_value_set_int(value, psy_canvas_get_distance_mm(self));
        break;
    case WIDTH_VISUAL_DEGREES:
        g_value_set_float(value, psy_canvas_get_width_vd(self));
        break;
    case HEIGHT_VISUAL_DEGREES:
        g_value_set_float(value, psy_canvas_get_height_vd(self));
        break;
    case FRAME_DUR:
        g_value_set_object(value, psy_canvas_get_frame_dur(self));
        break;
    case PROJECTION_STYLE:
        g_value_set_int(value, psy_canvas_get_projection_style(self));
        break;
    case CONTEXT:
        g_value_set_object(value, psy_canvas_get_context(self));
        break;
    case NUM_STIMULI:
        g_value_set_uint(value, psy_canvas_get_num_stimuli(self));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, spec);
    }
}

static void
psy_canvas_init(PsyCanvas *self)
{
    PsyCanvasPrivate *priv = psy_canvas_get_instance_private(self);

    gfloat r = 0.5, g = 0.5, b = 0.5;

    // Both the stimuli and artist own a reference
    priv->stimuli = g_ptr_array_new_with_free_func(g_object_unref);
    priv->artists = g_hash_table_new_full(
        g_direct_hash, g_direct_equal, g_object_unref, g_object_unref);

    priv->background_color = psy_color_new_rgb(r, g, b);

    // Assume a default frame dur based on 60Hz frame rate
    priv->frame_dur = psy_duration_new(1.0 / 60);
}

static void
psy_canvas_dispose(GObject *gobject)
{
    PsyCanvasPrivate *priv
        = psy_canvas_get_instance_private(PSY_CANVAS(gobject));

    if (priv->stimuli) {
        g_ptr_array_unref(priv->stimuli);
        priv->stimuli = NULL;
    }

    if (priv->artists) {
        g_hash_table_destroy(priv->artists);
        priv->artists = NULL;
    }

    g_clear_object(&priv->frame_dur);
    g_clear_object(&priv->context);
    g_clear_object(&priv->projection_matrix);
    g_clear_object(&priv->background_color);

    G_OBJECT_CLASS(psy_canvas_parent_class)->dispose(gobject);
}

static void
psy_canvas_finalize(GObject *gobject)
{
    PsyCanvasPrivate *priv
        = psy_canvas_get_instance_private(PSY_CANVAS(gobject));
    (void) priv;

    G_OBJECT_CLASS(psy_canvas_parent_class)->finalize(gobject);
}

static GParamSpec *obj_properties[N_PROPS];
static guint       canvas_signals[LAST_SIGNAL];

static void
resize(PsyCanvas *self, gint width, gint height)
{
    PsyCanvasPrivate *priv = psy_canvas_get_instance_private(self);
    priv->width            = width;
    priv->height           = height;

    PsyCanvasClass *klass = PSY_CANVAS_GET_CLASS(self);
    klass->set_projection_matrix(self, klass->create_projection_matrix(self));
}

static void
set_width(PsyCanvas *self, gint width)
{
    PsyCanvasPrivate *priv = psy_canvas_get_instance_private(self);
    priv->width            = width;
}

static void
set_height(PsyCanvas *self, gint height)
{
    PsyCanvasPrivate *priv = psy_canvas_get_instance_private(self);
    priv->height           = height;
}

static void
set_frame_dur(PsyCanvas *self, PsyDuration *dur)
{
    PsyCanvasPrivate *priv = psy_canvas_get_instance_private(self);
    g_clear_object(&priv->frame_dur);

    priv->frame_dur = psy_duration_dup(dur);
}

static void
schedule_stimulus(PsyCanvas *self, PsyVisualStimulus *stimulus)
{
    PsyCanvasPrivate *priv = psy_canvas_get_instance_private(self);

    // Check if the stimulus is already scheduled
    if (g_hash_table_contains(priv->artists, stimulus)) {
        g_info("PsyCanvas:%s, stimulus is already scheduled", __func__);
        return;
    }

    PsyArtist *artist = psy_visual_stimulus_create_artist(stimulus);

    // add a reference for insertion in array and hashtable
    g_object_ref(stimulus);
    g_ptr_array_add(priv->stimuli, stimulus);
    g_object_ref(stimulus);
    g_hash_table_insert(priv->artists, stimulus, artist);
}

static void
remove_stimulus(PsyCanvas *self, PsyVisualStimulus *stimulus)
{
    PsyCanvasPrivate *priv = psy_canvas_get_instance_private(self);

    g_ptr_array_remove(priv->stimuli, stimulus);
    g_hash_table_remove(priv->artists, stimulus);
}

static void
draw(PsyCanvas *self, guint64 frame_num, PsyTimePoint *tp)
{
    PsyCanvasClass   *cls  = PSY_CANVAS_GET_CLASS(self);
    PsyCanvasPrivate *priv = psy_canvas_get_instance_private(self);

    // upload the default projection matrices.
    g_return_if_fail(cls->clear);
    g_return_if_fail(cls->draw_stimuli);
    g_return_if_fail(cls->update_frame_stats);

    cls->clear(self);
    cls->upload_projection_matrices(self);
    cls->draw_stimuli(self, frame_num, tp);
    cls->update_frame_stats(self, &priv->frame_count);
}

static void
draw_stimuli(PsyCanvas *self, guint64 frame_num, PsyTimePoint *tp)
{
    PsyCanvasPrivate *priv            = psy_canvas_get_instance_private(self);
    PsyCanvasClass   *klass           = PSY_CANVAS_GET_CLASS(self);
    GPtrArray        *nodes_to_remove = g_ptr_array_new();

    // Draw stimuli from top to bottom, this means that the stimulus
    // that was added the latest, is drawn the first. This means that if
    // they are presented at the same depth (z-coordinate) that the last
    // stimulus is dominant and will be visible.
    for (gint i = priv->stimuli->len - 1; i >= 0; i--) {

        PsyStimulus       *stim  = priv->stimuli->pdata[i];
        PsyVisualStimulus *vstim = PSY_VISUAL_STIMULUS(stim);
        gint64             start_frame, nth_frame, num_frames;

        /* Schedule if necessary*/
        if (!psy_visual_stimulus_is_scheduled(vstim)) {
            PsyTimePoint *start = psy_stimulus_get_start_time(stim);
            PsyDuration  *wait  = psy_time_point_subtract(start, tp);
            gint64        num_frames_away
                = psy_duration_divide_rounded(wait, priv->frame_dur);
            g_object_unref(wait);
            if (num_frames_away < 0) {
                g_warning(
                    "Scheduling a stimulus that should have been presented "
                    "in the past, the stimulus will be presented as "
                    "quickly as possible.");
                num_frames_away = 0;
            }

            psy_visual_stimulus_set_start_frame(vstim,
                                                frame_num + num_frames_away);
        }

        start_frame = psy_visual_stimulus_get_start_frame(vstim);
        nth_frame   = psy_visual_stimulus_get_nth_frame(vstim);
        num_frames  = psy_visual_stimulus_get_num_frames(vstim);
        if (start_frame <= (gint64) frame_num) {
            psy_visual_stimulus_emit_update(vstim, tp, nth_frame);
            g_assert(klass->draw_stimulus);
            klass->draw_stimulus(self, vstim);
        }
        nth_frame = psy_visual_stimulus_get_nth_frame(vstim);
        if (nth_frame == 1) {
            psy_stimulus_set_is_started(PSY_STIMULUS(stim), tp);
        }

        // postpone removing nodes after iterating over items
        if (nth_frame >= num_frames)
            g_ptr_array_add(nodes_to_remove, stim);
    }

    PsyTimePoint *tend = psy_time_point_add(tp, priv->frame_dur);
    for (gsize i = 0; i < nodes_to_remove->len; i++) {
        PsyStimulus *stim = g_ptr_array_index(nodes_to_remove, i);
        psy_stimulus_set_is_finished(stim, tend);
        psy_canvas_remove_stimulus(self, PSY_VISUAL_STIMULUS(stim));
    }
    g_object_unref(tend);
    g_ptr_array_unref(nodes_to_remove);
}

static void
draw_stimulus(PsyCanvas *self, PsyVisualStimulus *stimulus)
{
    PsyCanvasPrivate *priv = psy_canvas_get_instance_private(self);
    PsyArtist        *artist;

    artist = g_hash_table_lookup(priv->artists, stimulus);
    psy_artist_draw(artist);
}

// static void
// set_monitor_size_mm(PsyCanvas *self, gint width_mm, gint height_mm)
// {
//     PsyCanvasPrivate *priv = psy_canvas_get_instance_private(self);
//     priv->width_mm         = width_mm;
//     priv->height_mm        = height_mm;
// }

PsyMatrix4 *
create_projection_matrix(PsyCanvas *self)
{
    // TODO check whether near and far are valid.
    gint   w, h;
    gfloat width, height, width_mm, height_mm;
    gint   w_mm, h_mm;
    gfloat left = 0.0, right = 0.0, bottom, top, near = 100.0, far = -100.0;

    // clang-format off
    g_object_get(self,
            "width", &w,
            "height", &h,
            "width_mm", &w_mm,
            "height_mm", &h_mm,
            NULL);
    // clang-format on

    width     = (gfloat) w;
    height    = (gfloat) h;
    width_mm  = (gfloat) w_mm;
    height_mm = (gfloat) h_mm;

    gint style = psy_canvas_get_projection_style(self);

    if (style & PSY_CANVAS_PROJECTION_STYLE_CENTER) {
        if (style & PSY_CANVAS_PROJECTION_STYLE_PIXELS) {
            left   = -width / 2;
            right  = width / 2;
            top    = height / 2;
            bottom = -height / 2;
        }
        else if (style & PSY_CANVAS_PROJECTION_STYLE_METER) {
            left   = -(width_mm / 1000) / 2;
            right  = (width_mm / 1000) / 2;
            top    = (height_mm / 1000) / 2;
            bottom = -(height_mm / 1000) / 2;
        }
        else if (style & PSY_CANVAS_PROJECTION_STYLE_MILLIMETER) {
            left   = -width_mm / 2;
            right  = width_mm / 2;
            top    = height_mm / 2;
            bottom = -height_mm / 2;
        }
        else {
            g_assert(style & PSY_CANVAS_PROJECTION_STYLE_VISUAL_DEGREES);
            g_warning("visual degrees are yet unsupported");
            return NULL;
        }
    }
    else {
        g_assert(style & PSY_CANVAS_PROJECTION_STYLE_C);
        if (style & PSY_CANVAS_PROJECTION_STYLE_PIXELS) {
            left   = 0.0;
            right  = width;
            top    = 0.0;
            bottom = height;
        }
        else if (style & PSY_CANVAS_PROJECTION_STYLE_METER) {
            left   = 0.0;
            right  = width_mm / 1000;
            top    = 0.0;
            bottom = height_mm / 1000;
        }
        else if (style & PSY_CANVAS_PROJECTION_STYLE_MILLIMETER) {
            left   = 0.0;
            right  = width_mm;
            top    = 0.0;
            bottom = height_mm;
        }
        else {
            g_assert(style & PSY_CANVAS_PROJECTION_STYLE_VISUAL_DEGREES);
            g_warning("visual degrees are yet unsupported");
            return NULL;
        }
    }

    return psy_matrix4_new_ortographic(left, right, bottom, top, near, far);
}

static void
set_projection_matrix(PsyCanvas *self, PsyMatrix4 *projection)
{
    PsyCanvasPrivate *priv = psy_canvas_get_instance_private(self);
    g_clear_object(&priv->projection_matrix);
    priv->projection_matrix = projection;
}

static PsyImage *
get_image(PsyCanvas *self)
{
    gint width, height;
    width  = psy_canvas_get_width(self);
    height = psy_canvas_get_height(self);

    PsyImageFormat format = PSY_IMAGE_FORMAT_RGBA;
    PsyImage      *image  = psy_image_new(width, height, format);

    return image;
}

static void
reset(PsyCanvas *self)
{
    PsyCanvasPrivate *priv = psy_canvas_get_instance_private(self);

    // Don't touch width, height as they are most likely determined by deriving
    // classes.

    memset(&priv->frame_count, 0, sizeof(priv->frame_count));

    const gfloat rgba[] = {0.5, 0.5, 0.5, 1.0};
    // clang-format off
    g_object_set(priv->background_color,
                 "r", rgba[0],
                 "g", rgba[1],
                 "b", rgba[2],
                 "a", rgba[3],
                 NULL);
    // clang-format on

    g_ptr_array_set_size(priv->stimuli, 0);
    g_hash_table_remove_all(priv->artists);
    priv->projection_style = PSY_CANVAS_PROJECTION_STYLE_CENTER
                             | PSY_CANVAS_PROJECTION_STYLE_PIXELS;
}

static void
psy_canvas_class_init(PsyCanvasClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    object_class->set_property = psy_canvas_set_property;
    object_class->get_property = psy_canvas_get_property;
    object_class->dispose      = psy_canvas_dispose;
    object_class->finalize     = psy_canvas_finalize;

    klass->resize = resize;

    klass->set_width  = set_width;
    klass->set_height = set_height;

    klass->draw          = draw;
    klass->draw_stimuli  = draw_stimuli;
    klass->draw_stimulus = draw_stimulus;
    // klass->set_monitor_size_mm = set_monitor_size_mm;

    klass->set_frame_dur = set_frame_dur;

    klass->schedule_stimulus = schedule_stimulus;
    klass->remove_stimulus   = remove_stimulus;

    klass->create_projection_matrix = create_projection_matrix;
    klass->set_projection_matrix    = set_projection_matrix;

    klass->get_image = get_image;

    klass->reset = reset;

    /**
     * PsyCanvas:bg-color-values
     *
     * The color of the background, you can use this property to get/set
     * the background color of the canvas. It is basically, an array of 4 floats
     * in RGBA format where the color values range between 0.0 and 1.0.
     */
    obj_properties[BACKGROUND_COLOR]
        = g_param_spec_object("background-color",
                              "BackgroundColor",
                              "The color used as background for the canvas.",
                              PSY_TYPE_COLOR,
                              G_PARAM_READWRITE);

    /**
     * PsyCanvas:width:
     *
     * The width of the canvas. When using windowed canvasses this property
     * should be considered read only. When using off screen surfaces such
     * as [class@PsyGlCanvas] it may be handy upon construction, as the
     * surface needs some size.
     */
    obj_properties[WIDTH]
        = g_param_spec_int("width",
                           "Width",
                           "The width of the canvas in pixels",
                           0,
                           G_MAXINT32,
                           0,
                           G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

    /**
     * PsyCanvas:height:
     *
     * The height of the canvas. When using windowed canvasses this propety
     * should be considered read only. When using off screen surfaces such
     * as [class@PsyGlCanvas] it may be handy upon construction, as the
     * surface needs some size.
     */
    obj_properties[HEIGHT]
        = g_param_spec_int("height",
                           "Height",
                           "The height of the canvas in pixel.",
                           0,
                           G_MAXINT32,
                           0,
                           G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

    /**
     * PsyCanvas:width-mm:
     *
     * The width of the monitor of the canvas in mm may be -1, it's
     * uninitialized, 0 we don't know, or > 0 the width in mm, one should always
     * check with a ruler to verify, as we try to determine it from the
     * backend.
     * If one knows better than the os/backend, one could set it self, don't
     * worry it's unlikely that the physical size of your monitor will change.
     */
    obj_properties[WIDTH_MM] = g_param_spec_int(
        "width-mm",
        "WidthMM",
        "The width of the canvas in mm, assuming a full screen canvas",
        -1,
        G_MAXINT32,
        -1,
        G_PARAM_READWRITE);

    /**
     * PsyCanvas:height-mm:
     *
     * The height of the monitor of the canvas in mm may be -1, it's
     * uninitialized, 0 we don't know, or > 0 the height in mm, one should
     * always check with a ruler to verify, as we try to determine it from the
     * backend/os.
     * If one knows better than the os/backend, one could set it self, don't
     * worry it's unlikely that the physical size of your monitor will change.
     */
    obj_properties[HEIGHT_MM] = g_param_spec_int(
        "height-mm",
        "HeightMM",
        "The height of the canvas in mm, assuming a full screen canvas",
        -1,
        G_MAXINT32,
        -1,
        G_PARAM_READWRITE);

    /**
     * PsyCanvas:distance-mm:
     *
     * The distance between the participant and the canvas (window/monitor).
     * This value may be ignored, but it must be set when you want to know
     * the size of the screen in visual degrees or want to specify the stimuli
     * in visual degrees.
     */
    obj_properties[DISTANCE_MM] = g_param_spec_int(
        "distance-mm",
        "DistanceMM",
        "The distance between the participant and the canvas",
        0,
        G_MAXINT32,
        0,
        G_PARAM_READWRITE);

    /**
     * obj_properties:width-vd:
     *
     * The width of a canvas in visual degrees. This is an computed property:
     * the width of and distance to the canvas must be known in order to compute
     * this.
     */
    obj_properties[WIDTH_VISUAL_DEGREES]
        = g_param_spec_float("width-vd",
                             "WidthVD",
                             "The width of the canvas in visual degrees",
                             -1.0,
                             G_MAXFLOAT,
                             0,
                             G_PARAM_READABLE);

    /**
     * obj_properties:height-vd:
     *
     * The height of a canvas in visual degrees. This is an computed property:
     * the height of and distance to the canvas must be known in order to
     * compute this.
     */
    obj_properties[HEIGHT_VISUAL_DEGREES]
        = g_param_spec_float("height-vd",
                             "HeightVD",
                             "The height of the canvas in visual degrees",
                             0,
                             G_MAXFLOAT,
                             0,
                             G_PARAM_READABLE);

    /**
     * PsyCanvas:frame-dur:
     *
     * The duration of one frame, this will be the reciprocal of the framerate
     * of the monitor on which this canvas is presented. Although, this
     * property is writeable, you should take in mind that psylib tends
     * to follow the system settings for the frame dur for one specific
     * monitor. So when you set the frame dur, you should take in
     * mind, that the window back end will overwrite what you put here.
     * And if you overwrite it after the back end has set the value for
     * this canvas, you'll most do not get what you bargain for. For example
     * when the system
     */
    obj_properties[FRAME_DUR]
        = g_param_spec_object("frame-dur",
                              "FrameDur",
                              "The duration of one frame.",
                              PSY_TYPE_DURATION,
                              G_PARAM_READWRITE);

    /**
     * PsyCanvas:projection-style:
     *
     * The manner in which geometrical units normalized coordinates are
     * projected to the physical output parameters. In OpenGL for example
     * units are specified normalized device parameters between -1.0 and 1.0
     * This is for other users perhaps not as convenient. For our 2D drawing
     * it might be more appropriate to specify the range in pixels.
     * This is a combination `PsyCanvasProjectionStyle` flags
     * When setting the property one should specify exactly one of:
     *
     *  - PSY_CANVAS_PROJECTION_STYLE_C
     *  - PSY_CANVAS_PROJECTION_STYLE_CENTER
     *
     * and exactly one of:
     *
     *  - PSY_CANVAS_PROJECTION_STYLE_PIXELS
     *  - PSY_CANVAS_PROJECTION_STYLE_METER
     *  - PSY_CANVAS_PROJECTION_STYLE_MILLIMETER
     *  - PSY_CANVAS_PROJECTION_STYLE_VISUAL_DEGREES
     *
     * if you would fail to this no valid projection can be set and probably
     * the last one will be kept.
     * The default is:
     *  PSY_CANVAS_PROJECTION_STYLE_CENTER | PSY_CANVAS_PROJECTION_STYLE_PIXELS
     */
    obj_properties[PROJECTION_STYLE] = g_param_spec_int(
        "projection-style",
        "Projection style",
        "Tells what the projection matrix is doing for you",
        0,
        G_MAXINT32,
        PSY_CANVAS_PROJECTION_STYLE_CENTER | PSY_CANVAS_PROJECTION_STYLE_PIXELS,
        G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

    /**
     * PsyContext:context:
     * The drawing context of this canvas. You can get the drawing context
     * of the current canvas. The context might help one to get some drawing
     * done on this canvas. Also it contains resources related to drawing
     * such as Shaders(`PsyShader`) and ShaderPrograms (`PsyProgram`)
     * this context allows to create `PsyArtist`s that can use the methods
     * from this context in order to draw the stimulus. And than makes it
     * possible to use General drawing method instead of Stimulus specific
     * methods.
     */

    obj_properties[CONTEXT]
        = g_param_spec_object("context",
                              "Context",
                              "The drawing context of this canvas.",
                              PSY_TYPE_DRAWING_CONTEXT,
                              G_PARAM_READABLE);

    /**
     * PsyContext:num-stimuli:
     *
     * Contains the number of stimuli attached to this canvas.
     */
    obj_properties[NUM_STIMULI]
        = g_param_spec_uint("num-stimuli",
                            "NumStimuli",
                            "The number of contained stimuli",
                            0,
                            G_MAXUINT32,
                            0,
                            G_PARAM_READABLE);

    g_object_class_install_properties(object_class, N_PROPS, obj_properties);

    canvas_signals[RESIZE]
        = g_signal_new("resize",
                       PSY_TYPE_CANVAS,
                       G_SIGNAL_RUN_LAST,
                       G_STRUCT_OFFSET(PsyCanvasClass, resize),
                       NULL,
                       NULL,
                       NULL,
                       G_TYPE_NONE,
                       2,
                       G_TYPE_INT,
                       G_TYPE_INT);

    //    /**
    //     * PsyCanvas::clear
    //     *
    //     * This is the first action that is run when a new frame should be
    //     * presented. The default handler calls the private/protected clear
    //     function
    //     * that clears the canvas.
    //     */
    //    canvas_signals[CLEAR] = g_signal_new(
    //            "clear",
    //            G_TYPE_FROM_CLASS(object_class),
    //            G_SIGNAL_RUN_FIRST,
    //            G_STRUCT_OFFSET(PsyCanvasClass, clear),
    //            NULL,
    //            NULL,
    //            NULL,
    //            G_TYPE_NONE,
    //            0);
    //
    //    /**
    //     * PsyCanvas::draw-stimuli
    //     *
    //     * This is the first action that is run when a new frame should be
    //     * presented. The default handler calls the private/protected clear
    //     function
    //     * that clears the canvas.
    //     */
    //    canvas_signals[DRAW_STIMULI] = g_signal_new(
    //            "draw-stimuli",
    //            G_TYPE_FROM_CLASS(object_class),
    //            G_SIGNAL_RUN_FIRST | G_SIGNAL_NO_RECURSE,
    //            G_STRUCT_OFFSET(PsyCanvasClass, clear),
    //            NULL,
    //            NULL,
    //            NULL,
    //            G_TYPE_NONE,
    //            2,
    //            G_TYPE_UINT64,
    //            PSY_TYPE_TIME_POINT
    //            );
}

/**
 * psy_canvas_set_background_color:
 * @self: a #PsyCanvas instance
 * @color:(transfer none): An instance of [class@Color].
 *
 * set the background color of this canvas.
 */
void
psy_canvas_set_background_color(PsyCanvas *self, PsyColor *color)
{
    g_return_if_fail(PSY_IS_CANVAS(self));
    PsyCanvasPrivate *priv = psy_canvas_get_instance_private(self);

    g_clear_object(&priv->background_color);
    priv->background_color = psy_color_dup(color);
}

/**
 * psy_canvas_get_background_color:
 * @self: a #PsyCanvas instance
 *
 * Get the background color of this canvas.
 *
 * Returns:(transfer none): the background color of this canvas
 */
PsyColor *
psy_canvas_get_background_color(PsyCanvas *self)
{
    g_return_val_if_fail(PSY_IS_CANVAS(self), NULL);
    PsyCanvasPrivate *priv = psy_canvas_get_instance_private(self);

    return priv->background_color;
}

/**
 * psy_canvas_get_width:
 * @self:A #PsyCanvas instance
 *
 * Returns: the width in pixels of the canvas
 */
gint
psy_canvas_get_width(PsyCanvas *self)
{
    PsyCanvasPrivate *priv = psy_canvas_get_instance_private(self);
    g_return_val_if_fail(PSY_IS_CANVAS(self), -1);

    return priv->width;
}

/**
 * psy_canvas_set_width:
 * @self: an instance of [class@Canvas].
 * @width: a width in pixels should be larger than 0.
 *
 * If possible it will set the width of the Canvas, this is mostly useful to
 * determine the size when constructing a OffScreen canvas such as
 * [class@GlCanvas], window canvases are full screen windows and they takeover
 * the system settings and might ignore this call.
 */
void
psy_canvas_set_width(PsyCanvas *self, gint width)
{
    g_return_if_fail(PSY_IS_CANVAS(self));
    g_return_if_fail(width >= 0);

    PsyCanvasClass *cls = PSY_CANVAS_GET_CLASS(self);
    g_return_if_fail(cls->set_width != NULL);

    cls->set_width(self, width);
}

/**
 * psy_canvas_get_height:
 * @self:A #PsyCanvas instance
 *
 * Returns: the height in pixels of the canvas
 */
gint
psy_canvas_get_height(PsyCanvas *self)
{
    PsyCanvasPrivate *priv = psy_canvas_get_instance_private(self);
    g_return_val_if_fail(PSY_IS_CANVAS(self), -1);

    return priv->height;
}

/**
 * psy_canvas_set_height:
 * @self: an instance of [class@Canvas].
 * @height: a height in pixels should be larger than 0.
 *
 * If possible it will set the height of the Canvas, this is mostly useful to
 * determine the size when constructing a OffScreen canvas such as
 * [class@GlCanvas], window canvases are fullscreen windows and they takeover
 * the system settings and might ignore this call.
 */
void
psy_canvas_set_height(PsyCanvas *self, gint height)
{
    g_return_if_fail(PSY_IS_CANVAS(self));
    g_return_if_fail(height >= 0);

    PsyCanvasClass *cls = PSY_CANVAS_GET_CLASS(self);
    g_return_if_fail(cls->set_height != NULL);

    cls->set_height(self, height);
}

/**
 * psy_canvas_get_width_height_mm:
 * @self: a #PsyCanvas instance
 * @width_mm:(out)(nullable): The width in mm.
 * @height_mm:(out)(nullable): The height in mm.
 *
 * Obtain the width and height of the canvas. A negative (or 0) return value
 * indicates that we were not able to establish the width and/or height.
 */
void
psy_canvas_get_width_height_mm(PsyCanvas *self, gint *width_mm, gint *height_mm)
{
    g_return_if_fail(PSY_IS_CANVAS(self));
    PsyCanvasPrivate *private = psy_canvas_get_instance_private(self);
    if (width_mm)
        *width_mm = private->width_mm;
    if (height_mm)
        *height_mm = private->height_mm;
}

/**
 * psy_canvas_get_width_mm:
 * @self:A #PsyCanvas instance
 *
 * Returns: the width in mm of the canvas
 */
gint
psy_canvas_get_width_mm(PsyCanvas *self)
{
    g_return_val_if_fail(PSY_IS_CANVAS(self), -1);
    PsyCanvasPrivate *private = psy_canvas_get_instance_private(self);
    return private->width_mm;
}

/**
 * psy_canvas_set_width_mm:
 * @self: A `PsyCanvas` instance
 * @width_mm: the width of the canvas/monitor
 *
 * Set the width of the canvas. Override the settings as found by the os/backend
 */
void
psy_canvas_set_width_mm(PsyCanvas *self, gint width_mm)
{
    g_return_if_fail(PSY_IS_CANVAS(self));
    PsyCanvasPrivate *private = psy_canvas_get_instance_private(self);
    private->width_mm         = width_mm;
}

/**
 * psy_canvas_get_height_mm:
 * @self:A #PsyCanvas instance
 *
 * Returns: the height in mm of the canvas
 */
gint
psy_canvas_get_height_mm(PsyCanvas *self)
{
    g_return_val_if_fail(PSY_IS_CANVAS(self), -1);
    PsyCanvasPrivate *private = psy_canvas_get_instance_private(self);
    return private->height_mm;
}

/**
 * psy_canvas_set_height_mm:
 * @self:A #PsyCanvas instance
 * @height_mm: the height of the canvas/monitor
 *
 * Set the height of the canvas. Override the settings as found by the
 * os/backend
 */
void
psy_canvas_set_height_mm(PsyCanvas *self, gint height_mm)
{
    g_return_if_fail(PSY_IS_CANVAS(self));
    PsyCanvasPrivate *private = psy_canvas_get_instance_private(self);
    private->height_mm        = height_mm;
}

/**
 * psy_canvas_get_distance_mm:
 * @self: an instance of [class@Canvas]
 *
 * Get the distance of the participant and the monitor. You should use a
 * chin or something similar in order to get this working.
 * Note, this value is used e.g. to compute the number of visual degrees a
 * stimulus or this canvas is tall.
 *
 * Returns: The distance between the participant and the monitor.
 */
gint
psy_canvas_get_distance_mm(PsyCanvas *self)
{
    g_return_val_if_fail(PSY_IS_CANVAS(self), -1);

    PsyCanvasPrivate *priv = psy_canvas_get_instance_private(self);

    return priv->distance_mm;
}

/**
 * psy_canvas_set_distance_mm:
 * @self: an instance of [class@Canvas]
 * @distance: The distance of the participant in mm.
 *
 * Set the distance of the participant and the monitor. You should use a
 * chin support or something similar in order to get this working.
 * This value is used e.g. to compute the number of visual degrees a
 * stimulus or this canvas is tall.
 */
void
psy_canvas_set_distance_mm(PsyCanvas *self, gint distance)
{
    g_return_if_fail(PSY_IS_CANVAS(self));
    PsyCanvasPrivate *priv = psy_canvas_get_instance_private(self);

    priv->distance_mm = distance;
}

/**
 * psy_canvas_get_width_vd:
 * @self: An instance of [class@Canvas]
 *
 * Obtain the width in visual degrees of @self.
 *
 * Returns: A positive value which is the size of the window in visual degrees.
 * -1 when an error occurs
 */
gfloat
psy_canvas_get_width_vd(PsyCanvas *self)
{
    g_return_val_if_fail(PSY_IS_CANVAS(self), -1);

    PsyCanvasPrivate *priv = psy_canvas_get_instance_private(self);

    if (priv->width_mm <= 0) {
        g_warning("Can't obtain width in visual degrees when the physical "
                  "width is unknown");
        return -1;
    }

    if (priv->distance_mm <= 0) {
        g_warning(
            "Can't obtain width in visual degrees when distance_mm is unknown");
        return -1;
    }

    return psy_radians_to_degrees(
               atan((gfloat) priv->width_mm / 2.0 / priv->distance_mm))
           * 2.0;
}

/**
 * psy_canvas_get_height_vd:
 * @self: An instance of [class@Canvas]
 *
 * Obtain the height in visual degrees of @self.
 *
 * Returns: -1 on error, a positive value otherwise, which is the size of the
 * window in visual degrees.
 */
gfloat
psy_canvas_get_height_vd(PsyCanvas *self)
{
    g_return_val_if_fail(PSY_IS_CANVAS(self), -1);

    PsyCanvasPrivate *priv = psy_canvas_get_instance_private(self);

    if (priv->height_mm <= 0) {
        g_warning("Can't obtain height in visual degrees when the physical "
                  "height is unknown");
        return -1;
    }

    if (priv->distance_mm <= 0) {
        g_warning("Can't obtain height in visual degrees when distance_mm is "
                  "unknown");
        return -1;
    }

    return psy_radians_to_degrees(
               atan((gfloat) priv->height_mm / 2.0 / priv->distance_mm))
           * 2.0;
}

/**
 * psy_canvas_schedule_stimulus:
 * @self: a PsyCanvas instance
 * @stimulus: a `PsyVisualStimulus` instance that should be drawn on this canvas
 *
 * Notifies the canvas about a stimulus that should be drawn on it, if the
 * stimulus was already present, it is ignored.
 */
void
psy_canvas_schedule_stimulus(PsyCanvas *self, PsyVisualStimulus *stimulus)
{
    g_return_if_fail(PSY_IS_CANVAS(self));
    g_return_if_fail(PSY_IS_VISUAL_STIMULUS(stimulus));

    PsyCanvasClass *klass = PSY_CANVAS_GET_CLASS(self);

    g_return_if_fail(klass->schedule_stimulus);

    klass->schedule_stimulus(self, stimulus);
}

/**
 * psy_canvas_get_frame_dur:
 * @self: A `PsyCanvas` instance
 *
 * Returns:(transfer none): a `PsyDuration` for the duration between two frames
 */
PsyDuration *
psy_canvas_get_frame_dur(PsyCanvas *self)
{
    PsyCanvasPrivate *priv = psy_canvas_get_instance_private(self);

    g_return_val_if_fail(PSY_IS_CANVAS(self), NULL);
    return priv->frame_dur;
}

/**
 * psy_canvas_set_frame_dur:
 * @self: An instance of [class@PsyCanvas]
 * @dur:(transfer none): A duration between two succesive frames.
 *
 * Sets the frame dur of the canvas. Note this may be discarded/overwritten
 * by deriving classes as PsyLib generally takes over the system settings.
 * This is possible for instances of [class@PsyImageCanvas], so that running
 * an iteration of the canvas will make the canvas continue in time.
 *
 * So you should check whether you have gotten what you have asked for.
 */
void
psy_canvas_set_frame_dur(PsyCanvas *self, PsyDuration *dur)
{
    PsyCanvasClass *cls;
    g_return_if_fail(PSY_IS_CANVAS(self));
    g_return_if_fail(PSY_IS_DURATION(dur));

    cls = PSY_CANVAS_GET_CLASS(self);

    g_return_if_fail(cls->set_frame_dur);
    cls->set_frame_dur(self, dur);
}

/**
 * psy_canvas_remove_stimulus:
 * @self: A `PsyCanvas` instance.
 * @stimulus: A `PsyVisualStimulus` instance to be removed from the canvas
 *
 * In psy-lib, it's the canvas that initiates the drawing of stimuli. So
 * removing the stimulus from the canvas, means, it won't be scheduled
 * anymore Additionally this means that the `PsyStimulus::stopped` won't be
 * scheduled anymore.
 */
void
psy_canvas_remove_stimulus(PsyCanvas *self, PsyVisualStimulus *stimulus)
{
    g_return_if_fail(PSY_IS_CANVAS(self));

    PsyCanvasClass *klass = PSY_CANVAS_GET_CLASS(self);
    g_return_if_fail(klass->remove_stimulus);

    klass->remove_stimulus(self, stimulus);
}

/**
 * psy_canvas_set_projection_style:
 * @self: an instance of `PsyCanvas`
 * @projection_style: an instance of `PsyCanvasProjectionStyle` an | orable
 * combination of `PsyCanvasProjectionStyle` Take note some flags may not be
 *      orred together and at least some must be set.
 *
 *
 * Set the style of projection of the canvas see `PsyCanvas:projection-style`
 */
void
psy_canvas_set_projection_style(PsyCanvas *self, gint projection_style)
{
    PsyCanvasPrivate *priv  = psy_canvas_get_instance_private(self);
    PsyCanvasClass   *klass = PSY_CANVAS_GET_CLASS(self);
    gint              style = projection_style;

    gint origin_style = 0;
    gint unit_style   = 0;

    g_return_if_fail(PSY_IS_CANVAS(self));

    if (style & PSY_CANVAS_PROJECTION_STYLE_C)
        origin_style++;
    if (style & PSY_CANVAS_PROJECTION_STYLE_CENTER)
        origin_style++;

    if (origin_style != 1) {
        g_critical("%s:You should set PSY_CANVAS_PROJECTION_STYLE_C or "
                   "PSY_CANVAS_PROJECTION_STYLE_CENTER",
                   __func__);
        return;
    }

    if (style & PSY_CANVAS_PROJECTION_STYLE_PIXELS)
        unit_style++;
    if (style & PSY_CANVAS_PROJECTION_STYLE_METER)
        unit_style++;
    if (style & PSY_CANVAS_PROJECTION_STYLE_MILLIMETER)
        unit_style++;
    if (style & PSY_CANVAS_PROJECTION_STYLE_VISUAL_DEGREES) {
        g_warning("Oops setting visual degrees is currently not supported.");
        unit_style++;
    }

    if (unit_style != 1) {
        g_critical("%s: You should set exactly one of:\n"
                   "   - PSY_CANVAS_PROJECTION_STYLE_PIXELS or\n"
                   "   - PSY_CANVAS_PROJECTION_STYLE_METER or\n"
                   "   - PSY_CANVAS_PROJECTION_STYLE_MILLIMETER or\n"
                   "   - PSY_CANVAS_PROJECTION_STYLE_VISUAL_DEGREES",
                   __func__);
        return;
    }

    priv->projection_style = style;

    PsyMatrix4 *projection = klass->create_projection_matrix(self);

    g_clear_object(&priv->projection_matrix);
    priv->projection_matrix = projection;
}

/**
 * psy_canvas_get_projection_style:
 * @self: an instance of `PsyCanvas`
 *
 * Returns: the style of projection of the canvas see
 * `PsyCanvas:projection-style`
 */
gint
psy_canvas_get_projection_style(PsyCanvas *self)
{
    PsyCanvasPrivate *priv = psy_canvas_get_instance_private(self);

    g_return_val_if_fail(PSY_IS_CANVAS(self), 0);

    return priv->projection_style;
}

/**
 * psy_canvas_get_projection:
 * @self: an instance of `PsyCanvas`
 *
 * Returns:(transfer none): The projection matrix used by this canvas
 */
PsyMatrix4 *
psy_canvas_get_projection(PsyCanvas *self)
{
    PsyCanvasPrivate *priv = psy_canvas_get_instance_private(self);

    g_return_val_if_fail(PSY_IS_CANVAS(self), NULL);
    return priv->projection_matrix;
}

/**
 * psy_canvas_set_context:
 * @self: an instance of `PsyCanvas`
 * @context:(transfer full): the drawing context for this canvas that draws
 * using whatever backend this canvas is using.
 *
 * Set the drawing context for this canvas
 *
 * private:
 */
void
psy_canvas_set_context(PsyCanvas *self, PsyDrawingContext *context)
{
    g_return_if_fail(PSY_IS_CANVAS(self));
    g_return_if_fail(PSY_IS_DRAWING_CONTEXT(context));

    PsyCanvasPrivate *priv = psy_canvas_get_instance_private(self);
    g_clear_object(&priv->context);
    priv->context = context;
}

/**
 * psy_canvas_get_context:
 * @self: an instance of `PsyCanvas`
 *
 * Get the drawing context for this canvas for drawing purposes.
 *
 * Returns:(transfer none): The active drawing context that belongs to this
 *                          `PsyCanvas`
 */
PsyDrawingContext *
psy_canvas_get_context(PsyCanvas *self)
{
    g_return_val_if_fail(PSY_IS_CANVAS(self), NULL);
    PsyCanvasPrivate *priv = psy_canvas_get_instance_private(self);

    return priv->context;
}

/**
 * psy_canvas_swap_stimuli:
 * @self: an instance of `PsyCanvas`
 * @i1: The index of the first stimulus must be larger or equal than
 *      0 but smaller than PsyCanvas:num-stims
 * @i2: the same for @i1
 *
 * Swap the order in which the visual stimuli are drawn.
 */
void
psy_canvas_swap_stimuli(PsyCanvas *self, guint i1, guint i2)
{
    PsyCanvasPrivate *priv = psy_canvas_get_instance_private(self);
    g_return_if_fail(PSY_CANVAS(self));
    g_return_if_fail(i1 < priv->stimuli->len);
    g_return_if_fail(i2 < priv->stimuli->len);

    PsyStimulus *stim_temp;
    stim_temp                = priv->stimuli->pdata[i1];
    priv->stimuli->pdata[i1] = priv->stimuli->pdata[i2];
    priv->stimuli->pdata[i2] = stim_temp;
}

/**
 * psy_canvas_get_num_stimuli:
 * @self: A `PsyCanvas` instance
 *
 * Returns: the number of stimuli contained by the canvas
 */
guint
psy_canvas_get_num_stimuli(PsyCanvas *self)
{
    PsyCanvasPrivate *priv = psy_canvas_get_instance_private(self);
    g_return_val_if_fail(PSY_CANVAS(self), 0);

    return priv->stimuli->len;
}

/**
 * psy_canvas_get_image:
 * @self: The canvas to take a picture of
 *
 * This method takes a snapshot of what is displayed after the current is
 * drawing is done. So it waits until all drawing is finished and retrieves
 * the pixel data of the, hence, it may be handy for getting an image of what
 * you are drawing, but it might not be handy to do for your real experiment as
 * this function may take quite some time.
 *
 * Returns:(transfer full): an instance of [class@PsyImage] with the same size
 * as the canvas
 */
PsyImage *
psy_canvas_get_image(PsyCanvas *self)
{
    g_return_val_if_fail(PSY_IS_CANVAS(self), NULL);

    PsyCanvasClass *cls = PSY_CANVAS_GET_CLASS(self);

    g_return_val_if_fail(cls->get_image, NULL);
    return cls->get_image(self);
}

/**
 * psy_canvas_get_num_frames:
 * @self: an instance of [class@Canvas]
 *
 * The num frames is the number of frames that the canvas successfully presented
 * a new frame.
 *
 * Returns: the number of frames we've presented.
 */
gint64
psy_canvas_get_num_frames(PsyCanvas *self)
{
    g_return_val_if_fail(PSY_IS_CANVAS(self), -1);
    PsyCanvasPrivate *priv = psy_canvas_get_instance_private(self);

    return priv->frame_count.num_frames;
}

/**
 * psy_canvas_get_num_frames_missed:
 * @self: an instance of [class@Canvas]
 *
 * The num frames missed is the number of frames that the canvas somehow missed
 *
 * Returns: the number of frames we've missed.
 */
gint64
psy_canvas_get_num_frames_missed(PsyCanvas *self)
{
    g_return_val_if_fail(PSY_IS_CANVAS(self), -1);
    PsyCanvasPrivate *priv = psy_canvas_get_instance_private(self);

    return priv->frame_count.missed_frames;
}

/**
 * psy_canvas_get_num_frames_total:
 * @self: an instance of [class@Canvas]
 *
 * The num frames is the sum of the number of frames we've missed and
 * succesfully presented. So this figure is hopefully the same as the
 * the result of [method@Psy.Canvas.get_num_frames]
 *
 * Returns: the number of frames we've that have elapsed.
 */
gint64
psy_canvas_get_num_frames_total(PsyCanvas *self)
{
    g_return_val_if_fail(PSY_IS_CANVAS(self), -1);
    PsyCanvasPrivate *priv = psy_canvas_get_instance_private(self);

    return priv->frame_count.tot_frames;
}

/**
 * psy_canvas_reset:
 * @self the canvas to be reset
 *
 * Removes all scheduled stimuli from the canvas. This method tries to
 * reset properties from the canvas to it's original values.
 */
void
psy_canvas_reset(PsyCanvas *self)
{
    g_return_if_fail(PSY_IS_CANVAS(self));

    PsyCanvasClass *cls = PSY_CANVAS_GET_CLASS(self);
    g_assert(cls->reset);

    cls->reset(self);
}
