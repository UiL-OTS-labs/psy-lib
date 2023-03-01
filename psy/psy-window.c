/**
 * PsyWindow:
 *
 * PsyWindow is an abstract class. It provides an base class for platform or
 * toolkit specific backends. A window in PsyLib is typically a fullscreen
 * window that is placed on a specific monitor.
 *
 * From the point of a psychological experiment tool-kit it doesn't make
 * sense to allow window that are not fullscreen, hence every window will
 * be created at full screen resolution.
 *
 * A derived window will know when it is ready to draw the window, as such
 * it will call the PsyWindow.draw method. The draw method will call two
 * functions.
 *
 * * It will call the PsyWindowClass::clear function to clear the background
 * to the default color.
 * * It will call the draw_stimuli function.
 *
 * The draw stimuli function does two things:
 *
 * 1. It will check the scheduled stimuli, to see whether there is
 *    a stimulus ready to present.
 * 2. It will update the stimuli that should be presented and make
 *    sure that the `PsyArtist`s will actually draw every stimulus.
 */
#include "psy-window.h"
#include "enum-types.h"
#include "psy-artist.h"
#include "psy-circle-artist.h"
#include "psy-circle.h"
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

typedef struct PsyWindowPrivate {
    gint   monitor;
    guint  n_frames;
    gint   width, height;
    gint   width_mm, height_mm;
    gfloat back_ground_color[4];

    GPtrArray  *stimuli; // owns a ref on the PsyStimulus
    GHashTable *artists; // owns a ref on the PsyStimulus and PsyArtist

    PsyDuration *frame_dur;

    PsyDrawingContext *context;

    gint        projection_style;
    PsyMatrix4 *projection_matrix;
} PsyWindowPrivate;

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE(PsyWindow, psy_window, G_TYPE_OBJECT)

typedef enum {
    CLEAR,
    DRAW_STIMULI,
    RESIZE,
    LAST_SIGNAL,
} PsyWindowSignal;

typedef enum {
    N_MONITOR = 1,
    BACKGROUND_COLOR_VALUES,
    WIDTH,
    HEIGHT,
    WIDTH_MM,
    HEIGHT_MM,
    FRAME_DUR,
    PROJECTION_STYLE,
    CONTEXT,
    NUM_STIMULI,
    N_PROPS
} PsyWindowProperty;

static void
psy_window_set_property(GObject      *object,
                        guint         property_id,
                        const GValue *value,
                        GParamSpec   *spec)
{
    PsyWindow *self = PSY_WINDOW(object);

    switch ((PsyWindowProperty) property_id) {
    case N_MONITOR:
        psy_window_set_monitor(self, g_value_get_int(value));
        break;
    case BACKGROUND_COLOR_VALUES:
        psy_window_set_background_color_values(self,
                                               g_value_get_pointer(value));
        break;
    case WIDTH_MM:
        psy_window_set_width_mm(self, g_value_get_int(value));
        break;
    case HEIGHT_MM:
        psy_window_set_height_mm(self, g_value_get_int(value));
        break;
    case PROJECTION_STYLE:
        psy_window_set_projection_style(self, g_value_get_int(value));
        break;
    case FRAME_DUR:
    case NUM_STIMULI:
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, spec);
    }
}

static void
psy_window_get_property(GObject    *object,
                        guint       property_id,
                        GValue     *value,
                        GParamSpec *spec)
{
    PsyWindow *self = PSY_WINDOW(object);

    switch ((PsyWindowProperty) property_id) {
    case N_MONITOR:
        g_value_set_int(value, psy_window_get_monitor(self));
        break;
    case WIDTH:
        g_value_set_int(value, psy_window_get_width(self));
        break;
    case HEIGHT:
        g_value_set_int(value, psy_window_get_height(self));
        break;
    case WIDTH_MM:
        g_value_set_int(value, psy_window_get_width_mm(self));
        break;
    case HEIGHT_MM:
        g_value_set_int(value, psy_window_get_height_mm(self));
        break;
    case FRAME_DUR:
        g_value_set_object(value, psy_window_get_frame_dur(self));
        break;
    case PROJECTION_STYLE:
        g_value_set_int(value, psy_window_get_projection_style(self));
        break;
    case CONTEXT:
        g_value_set_object(value, psy_window_get_context(self));
        break;
    case NUM_STIMULI:
        g_value_set_uint(value, psy_window_get_num_stimuli(self));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, spec);
    }
}

static void
psy_window_init(PsyWindow *self)
{
    PsyWindowPrivate *priv          = psy_window_get_instance_private(self);
    gfloat            default_bg[4] = {0.5, 0.5, 0.5, 1.0};

    // Both the stimuli and artist own a reference
    priv->stimuli = g_ptr_array_new_with_free_func(g_object_unref);
    priv->artists = g_hash_table_new_full(
        g_direct_hash, g_direct_equal, g_object_unref, g_object_unref);

    memcpy(priv->back_ground_color, default_bg, sizeof(default_bg));
}

static void
psy_window_dispose(GObject *gobject)
{
    PsyWindowPrivate *priv
        = psy_window_get_instance_private(PSY_WINDOW(gobject));

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

    G_OBJECT_CLASS(psy_window_parent_class)->dispose(gobject);
}

static void
psy_window_finalize(GObject *gobject)
{
    PsyWindowPrivate *priv
        = psy_window_get_instance_private(PSY_WINDOW(gobject));
    (void) priv;

    G_OBJECT_CLASS(psy_window_parent_class)->finalize(gobject);
}

static GParamSpec *obj_properties[N_PROPS];
static guint       window_signals[LAST_SIGNAL];

static gint
get_monitor(PsyWindow *self)
{
    PsyWindowPrivate *priv = psy_window_get_instance_private(self);
    return priv->monitor;
}

static void
set_monitor(PsyWindow *self, gint nth_monitor)
{
    PsyWindowPrivate *priv = psy_window_get_instance_private(self);
    priv->monitor          = nth_monitor;
}

static void
resize(PsyWindow *self, gint width, gint height)
{
    PsyWindowPrivate *priv = psy_window_get_instance_private(self);
    priv->width            = width;
    priv->height           = height;

    PsyWindowClass *klass = PSY_WINDOW_GET_CLASS(self);
    klass->set_projection_matrix(self, klass->create_projection_matrix(self));
}

static void
set_width(PsyWindow *self, gint width)
{
    PsyWindowPrivate *priv = psy_window_get_instance_private(self);
    priv->width            = width;
}

static void
set_height(PsyWindow *self, gint height)
{
    PsyWindowPrivate *priv = psy_window_get_instance_private(self);
    priv->width            = height;
}

static void
set_frame_dur(PsyWindow *self, PsyDuration *dur)
{
    PsyWindowPrivate *priv = psy_window_get_instance_private(self);
    priv->frame_dur        = dur;
}

static PsyArtist *
create_artist(PsyWindow *self, PsyVisualStimulus *stimulus)
{
    GType      type   = G_OBJECT_TYPE(stimulus);
    PsyArtist *artist = NULL;

    if (type == psy_circle_get_type()) {
        g_debug("creating circle artist");
        artist = PSY_ARTIST(psy_circle_artist_new(self, stimulus));
    }
    else if (type == psy_cross_get_type()) {
        g_debug("creating cross artist");
        artist = PSY_ARTIST(psy_cross_artist_new(self, stimulus));
    }
    else if (type == psy_rectangle_get_type()) {
        g_debug("creating rectangle artist");
        artist = PSY_ARTIST(psy_rectangle_artist_new(self, stimulus));
    }
    else if (type == psy_picture_get_type()) {
        g_debug("creating picture artist");
        artist = PSY_ARTIST(psy_picture_artist_new(self, stimulus));
    }
    else {
        g_warning("PsyWindow hasn't got an Artist for %s",
                  G_OBJECT_TYPE_NAME(stimulus));
    }

    return artist;
}

static void
schedule_stimulus(PsyWindow *self, PsyVisualStimulus *stimulus)
{
    PsyWindowPrivate *priv = psy_window_get_instance_private(self);
    PsyWindowClass   *cls  = PSY_WINDOW_GET_CLASS(self);

    // Check if the stimulus is already scheduled
    if (g_hash_table_contains(priv->artists, stimulus)) {
        g_info("PsyWindow:%s, stimulus is already scheduled", __func__);
        return;
    }

    PsyArtist *artist = cls->create_artist(self, stimulus);

    // add a reference for insertion in array and hashtable
    g_object_ref(stimulus);
    g_ptr_array_add(priv->stimuli, stimulus);
    g_object_ref(stimulus);
    g_hash_table_insert(priv->artists, stimulus, artist);
}

static void
remove_stimulus(PsyWindow *self, PsyVisualStimulus *stimulus)
{
    PsyWindowPrivate *priv = psy_window_get_instance_private(self);

    g_ptr_array_remove(priv->stimuli, stimulus);
    g_hash_table_remove(priv->artists, stimulus);
}

static void
draw(PsyWindow *self, guint64 frame_num, PsyTimePoint *tp)
{
    PsyWindowClass *cls = PSY_WINDOW_GET_CLASS(self);

    // upload the default projection matrices.
    g_return_if_fail(cls->clear);
    g_return_if_fail(cls->draw_stimuli);

    cls->clear(self);
    cls->upload_projection_matrices(self);
    cls->draw_stimuli(self, frame_num, tp);
}

static void
draw_stimuli(PsyWindow *self, guint64 frame_num, PsyTimePoint *tp)
{
    PsyWindowPrivate *priv            = psy_window_get_instance_private(self);
    PsyWindowClass   *klass           = PSY_WINDOW_GET_CLASS(self);
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
        psy_window_remove_stimulus(self, PSY_VISUAL_STIMULUS(stim));
    }
    g_object_unref(tend);
    g_ptr_array_unref(nodes_to_remove);
}

static void
draw_stimulus(PsyWindow *self, PsyVisualStimulus *stimulus)
{
    PsyWindowPrivate *priv = psy_window_get_instance_private(self);
    PsyArtist        *artist;

    artist = g_hash_table_lookup(priv->artists, stimulus);
    psy_artist_draw(artist);
}

static void
set_monitor_size_mm(PsyWindow *self, gint width_mm, gint height_mm)
{
    PsyWindowPrivate *priv = psy_window_get_instance_private(self);
    priv->width_mm         = width_mm;
    priv->height_mm        = height_mm;
}

PsyMatrix4 *
create_projection_matrix(PsyWindow *self)
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

    gint style = psy_window_get_projection_style(self);

    if (style & PSY_WINDOW_PROJECTION_STYLE_CENTER) {
        if (style & PSY_WINDOW_PROJECTION_STYLE_PIXELS) {
            left   = -width / 2;
            right  = width / 2;
            top    = height / 2;
            bottom = -height / 2;
        }
        else if (style & PSY_WINDOW_PROJECTION_STYLE_METER) {
            left   = -(width_mm / 1000) / 2;
            right  = (width_mm / 1000) / 2;
            top    = (height_mm / 1000) / 2;
            bottom = -(height_mm / 1000) / 2;
        }
        else if (style & PSY_WINDOW_PROJECTION_STYLE_MILLIMETER) {
            left   = -width_mm / 2;
            right  = width_mm / 2;
            top    = height_mm / 2;
            bottom = -height_mm / 2;
        }
        else {
            g_assert(style & PSY_WINDOW_PROJECTION_STYLE_VISUAL_DEGREES);
            g_warning("visual degrees are yet unsupported");
            return NULL;
        }
    }
    else {
        g_assert(style & PSY_WINDOW_PROJECTION_STYLE_C);
        if (style & PSY_WINDOW_PROJECTION_STYLE_PIXELS) {
            left   = 0.0;
            right  = width;
            top    = 0.0;
            bottom = height;
        }
        else if (style & PSY_WINDOW_PROJECTION_STYLE_METER) {
            left   = 0.0;
            right  = width_mm / 1000;
            top    = 0.0;
            bottom = height_mm / 1000;
        }
        else if (style & PSY_WINDOW_PROJECTION_STYLE_MILLIMETER) {
            left   = 0.0;
            right  = width_mm;
            top    = 0.0;
            bottom = height_mm;
        }
        else {
            g_assert(style & PSY_WINDOW_PROJECTION_STYLE_VISUAL_DEGREES);
            g_warning("visual degrees are yet unsupported");
            return NULL;
        }
    }

    return psy_matrix4_new_ortographic(left, right, bottom, top, near, far);
}

static void
set_projection_matrix(PsyWindow *self, PsyMatrix4 *projection)
{
    PsyWindowPrivate *priv = psy_window_get_instance_private(self);
    g_clear_object(&priv->projection_matrix);
    priv->projection_matrix = projection;
}

static void
psy_window_class_init(PsyWindowClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    object_class->set_property = psy_window_set_property;
    object_class->get_property = psy_window_get_property;
    object_class->dispose      = psy_window_dispose;
    object_class->finalize     = psy_window_finalize;

    klass->get_monitor = get_monitor;
    klass->set_monitor = set_monitor;

    klass->resize = resize;

    klass->set_width  = set_width;
    klass->set_height = set_height;

    klass->draw                = draw;
    klass->draw_stimuli        = draw_stimuli;
    klass->draw_stimulus       = draw_stimulus;
    klass->set_monitor_size_mm = set_monitor_size_mm;

    klass->set_frame_dur = set_frame_dur;

    klass->create_artist     = create_artist;
    klass->schedule_stimulus = schedule_stimulus;
    klass->remove_stimulus   = remove_stimulus;

    klass->create_projection_matrix = create_projection_matrix;
    klass->set_projection_matrix    = set_projection_matrix;

    /**
     * PsyWindow:n-monitor:
     *
     * The number of the monitor on which the PsyWindow should be
     * presented.
     */
    obj_properties[N_MONITOR]
        = g_param_spec_int("n-monitor",
                           "nmonitor",
                           "The number of the monitor to use for this window",
                           -1,
                           G_MAXINT32,
                           0,
                           G_PARAM_CONSTRUCT | G_PARAM_READWRITE);
    /**
     * PsyWindow:width:
     */
    obj_properties[WIDTH]
        = g_param_spec_int("width",
                           "Width",
                           "The width of the window in pixels",
                           0,
                           G_MAXINT32,
                           0,
                           G_PARAM_READABLE);

    /**
     * PsyWindow:height:
     */
    obj_properties[HEIGHT]
        = g_param_spec_int("height",
                           "",
                           "The height of the window in pixel.",
                           0,
                           G_MAXINT32,
                           0,
                           G_PARAM_READABLE);

    /**
     * PsyWindow:bg-color-values
     *
     * The color of the background, you can use this property to get/set
     * the background color of the window. It is basically, an array of 4 floats
     * in RGBA format where the color values range between 0.0 and 1.0.
     */
    obj_properties[BACKGROUND_COLOR_VALUES] = g_param_spec_pointer(
        "bg-color-values",
        "BackgroundColorValues",
        "An array with 4 floats representing RGBA color of the background",
        G_PARAM_READWRITE);

    /**
     * PsyWindow:width-mm:
     *
     * The width of the monitor of the window in mm may be -1, it's
     * uninitialized, 0 we don't know, or > 0 the width in mm, one should always
     * check with a ruler to verify, as we try to determine it from the
     * backend.
     * If one knows better than the os/backend, one could set it self, don't
     * worry it's unlikely that the physical size of your monitor will change.
     */
    obj_properties[WIDTH_MM] = g_param_spec_int(
        "width-mm",
        "WidthMM",
        "The width of the window in mm, assuming a full screen window",
        -1,
        G_MAXINT32,
        -1,
        G_PARAM_READWRITE);

    /**
     * PsyWindow:height-mm:
     *
     * The height of the monitor of the window in mm may be -1, it's
     * uninitialized, 0 we don't know, or > 0 the height in mm, one should
     * always check with a ruler to verify, as we try to determine it from the
     * backend/os.
     * If one knows better than the os/backend, one could set it self, don't
     * worry it's unlikely that the physical size of your monitor will change.
     */
    obj_properties[HEIGHT_MM] = g_param_spec_int(
        "height-mm",
        "HeightMM",
        "The height of the window in mm, assuming a full screen window",
        -1,
        G_MAXINT32,
        -1,
        G_PARAM_READWRITE);

    /**
     * PsyWindow:frame-dur:
     *
     * The duration of one frame, this will be the reciprocal of the framerate
     * of the monitor on which this window is presented.
     */
    obj_properties[FRAME_DUR]
        = g_param_spec_object("frame-dur",
                              "FrameDur",
                              "The duration of one frame.",
                              PSY_TYPE_DURATION,
                              G_PARAM_READABLE);

    /**
     * PsyWindow:projection-style:
     *
     * The manner in which geometrical units normalized coordinates are
     * projected to the physical output parameters. In OpenGL for example
     * units are specified normalized device parameters between -1.0 and 1.0
     * This is for other users perhaps not as convenient. For our 2D drawing
     * it might be more appropriate to specify the range in pixels.
     * This is a combination `PsyWindowProjectionStyle` flags
     * When setting the property one should specify exactly one of:
     *
     *  - PSY_WINDOW_PROJECTION_STYLE_C
     *  - PSY_WINDOW_PROJECTION_STYLE_CENTER
     *
     * and exactly one of:
     *
     *  - PSY_WINDOW_PROJECTION_STYLE_PIXELS
     *  - PSY_WINDOW_PROJECTION_STYLE_METER
     *  - PSY_WINDOW_PROJECTION_STYLE_MILLIMETER
     *  - PSY_WINDOW_PROJECTION_STYLE_VISUAL_DEGREES
     *
     * if you would fail to this no valid projection can be set and probably
     * the last one will be kept.
     * The default is:
     *  PSY_WINDOW_PROJECTION_STYLE_CENTER | PSY_WINDOW_PROJECTION_STYLE_PIXELS
     */
    obj_properties[PROJECTION_STYLE] = g_param_spec_int(
        "projection-style",
        "Projection style",
        "Tells what the projection matrix is doing for you",
        0,
        G_MAXINT32,
        PSY_WINDOW_PROJECTION_STYLE_CENTER | PSY_WINDOW_PROJECTION_STYLE_PIXELS,
        G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

    /**
     * PsyContext:context:
     * The drawing context of this window. You can get the drawing context
     * of the current window. The context might help one to get some drawing
     * done on this window. Also it contains resources related to drawing
     * such as Shaders(`PsyShader`) and ShaderPrograms (`PsyProgram`)
     * this context allows to create `PsyArtist`s that can use the methods
     * from this context in order to draw the stimulus. And than makes it
     * possible to use General drawing method instead of Stimulus specific
     * methods.
     */

    obj_properties[CONTEXT]
        = g_param_spec_object("context",
                              "Context",
                              "The drawing context of this window.",
                              PSY_TYPE_DRAWING_CONTEXT,
                              G_PARAM_READABLE);

    /**
     * PsyContext:num-stimuli:
     *
     * Contains the number of stimuli attached to this window.
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

    window_signals[RESIZE]
        = g_signal_new("resize",
                       PSY_TYPE_WINDOW,
                       G_SIGNAL_RUN_LAST,
                       G_STRUCT_OFFSET(PsyWindowClass, resize),
                       NULL,
                       NULL,
                       NULL,
                       G_TYPE_NONE,
                       2,
                       G_TYPE_INT,
                       G_TYPE_INT);

    //    /**
    //     * PsyWindow::clear
    //     *
    //     * This is the first action that is run when a new frame should be
    //     * presented. The default handler calls the private/protected clear
    //     function
    //     * that clears the window.
    //     */
    //    window_signals[CLEAR] = g_signal_new(
    //            "clear",
    //            G_TYPE_FROM_CLASS(object_class),
    //            G_SIGNAL_RUN_FIRST,
    //            G_STRUCT_OFFSET(PsyWindowClass, clear),
    //            NULL,
    //            NULL,
    //            NULL,
    //            G_TYPE_NONE,
    //            0);
    //
    //    /**
    //     * PsyWindow::draw-stimuli
    //     *
    //     * This is the first action that is run when a new frame should be
    //     * presented. The default handler calls the private/protected clear
    //     function
    //     * that clears the window.
    //     */
    //    window_signals[DRAW_STIMULI] = g_signal_new(
    //            "draw-stimuli",
    //            G_TYPE_FROM_CLASS(object_class),
    //            G_SIGNAL_RUN_FIRST | G_SIGNAL_NO_RECURSE,
    //            G_STRUCT_OFFSET(PsyWindowClass, clear),
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
 * psy_window_get_monitor:
 * @self: a #PsyWindow instance
 *
 * Returns the number of the monitor this window is being presented.
 * The result should b 0 for the first monitor to n - 1 for the last
 * where n is the number of monitors available to psy-lib.
 * Once there are multiple backends, it could be the case that that
 * the number is specific for this backend, and the number represents another
 * monitor for another backend.
 *
 * Returns: 0 to n - 1 where n is the number of monitors connected or
 *          -1 in case of an error.
 */
gint
psy_window_get_monitor(PsyWindow *self)
{
    g_return_val_if_fail(PSY_IS_WINDOW(self), -1);

    PsyWindowClass *class = PSY_WINDOW_GET_CLASS(self);
    g_return_val_if_fail(class->get_monitor, -1);

    return class->get_monitor(self);
}

/**
 * psy_window_set_monitor:
 * @self: a #PsyWindow instance
 * @nth_monitor: the number of the monitor on which to display the window
 *
 * Returns the number of the monitor this window is being presented.
 * The result should b 0 for the first monitor to n - 1 for the last
 * where n is the number of monitors available to psy-lib.
 * Once there are multiple backends, it could be the case that that
 * the number is specific for this backend, and the number represents another
 * monitor for another backend.
 *
 */
void
psy_window_set_monitor(PsyWindow *self, gint nth_monitor)
{
    g_return_if_fail(PSY_IS_WINDOW(self));

    PsyWindowClass *class = PSY_WINDOW_GET_CLASS(self);
    g_return_if_fail(class->set_monitor);

    class->set_monitor(self, nth_monitor);
}

/**
 * psy_window_set_background_color_values:
 * @self: a #PsyWindow instance
 * @color:(in)(array fixed-size=4)(element-type gfloat): the desired
 *          color values in RGBA format.
 *
 * set the background color of this window.
 */
void
psy_window_set_background_color_values(PsyWindow *self, gfloat *color)
{
    g_return_if_fail(PSY_IS_WINDOW(self));
    PsyWindowPrivate *priv = psy_window_get_instance_private(self);

    memcpy(priv->back_ground_color, color, sizeof(priv->back_ground_color));
}

/**
 * psy_window_get_background_color_values:
 * @self: a #PsyWindow instance
 * @color:(out callee-allocates)(array fixed-size=4)(element-type gfloat): the
 * desired color values in RGBA format.
 *
 * Get the background color of this window.
 */
void
psy_window_get_background_color_values(PsyWindow *self, gfloat *color)
{
    g_return_if_fail(PSY_IS_WINDOW(self));
    PsyWindowPrivate *priv = psy_window_get_instance_private(self);

    memcpy(color, priv->back_ground_color, sizeof(priv->back_ground_color));
}

/**
 * psy_window_get_width:
 * @self:A #PsyWindow instance
 *
 * Returns: the width in pixels of the window
 */
gint
psy_window_get_width(PsyWindow *self)
{
    PsyWindowPrivate *priv = psy_window_get_instance_private(self);
    ;
    g_return_val_if_fail(PSY_IS_WINDOW(self), -1);

    return priv->width;
}

/**
 * psy_window_get_height:
 * @self:A #PsyWindow instance
 *
 * Returns: the height in pixels of the window
 */
gint
psy_window_get_height(PsyWindow *self)
{
    PsyWindowPrivate *priv = psy_window_get_instance_private(self);
    g_return_val_if_fail(PSY_IS_WINDOW(self), -1);

    return priv->height;
}

/**
 * psy_window_get_width_height_mm:
 * @self: a #PsyWindow instance
 * @width_mm:(out)(nullable): The width in mm.
 * @height_mm:(out)(nullable): The height in mm.
 *
 * Obtain the width and height of the window. A negative (or 0) return value
 * indicates that we were not able to establish the width and/or height.
 */
void
psy_window_get_width_height_mm(PsyWindow *self, gint *width_mm, gint *height_mm)
{
    g_return_if_fail(PSY_IS_WINDOW(self));
    PsyWindowPrivate *private = psy_window_get_instance_private(self);
    if (width_mm)
        *width_mm = private->width_mm;
    if (height_mm)
        *height_mm = private->height_mm;
}

/**
 * psy_window_get_width_mm:
 * @self:A #PsyWindow instance
 *
 * Returns: the width in mm of the window
 */
gint
psy_window_get_width_mm(PsyWindow *self)
{
    g_return_val_if_fail(PSY_IS_WINDOW(self), -1);
    PsyWindowPrivate *private = psy_window_get_instance_private(self);
    return private->width_mm;
}

/**
 * psy_window_set_width_mm:
 * @self: A `PsyWindow` instance
 * @width_mm: the width of the window/monitor
 *
 * Set the width of the window. Override the settings as found by the os/backend
 */
void
psy_window_set_width_mm(PsyWindow *self, gint width_mm)
{
    g_return_if_fail(PSY_IS_WINDOW(self));
    PsyWindowPrivate *private = psy_window_get_instance_private(self);
    private->width_mm         = width_mm;
}

/**
 * psy_window_get_height_mm:
 * @self:A #PsyWindow instance
 *
 * Returns: the height in mm of the window
 */
gint
psy_window_get_height_mm(PsyWindow *self)
{
    g_return_val_if_fail(PSY_IS_WINDOW(self), -1);
    PsyWindowPrivate *private = psy_window_get_instance_private(self);
    return private->height_mm;
}

/**
 * psy_window_set_height_mm:
 * @self:A #PsyWindow instance
 * @height_mm: the height of the window/monitor
 *
 * Set the height of the window. Override the settings as found by the
 * os/backend
 */
void
psy_window_set_height_mm(PsyWindow *self, gint height_mm)
{
    g_return_if_fail(PSY_IS_WINDOW(self));
    PsyWindowPrivate *private = psy_window_get_instance_private(self);
    private->height_mm        = height_mm;
}

/**
 * psy_window_schedule_stimulus:
 * @self: a PsyWindow instance
 * @stimulus: a `PsyVisualStimulus` instance that should be drawn on this window
 *
 * Notifies the window about a stimulus that should be drawn on it, if the
 * stimulus was already present, it is ignored.
 */
void
psy_window_schedule_stimulus(PsyWindow *self, PsyVisualStimulus *stimulus)
{
    g_return_if_fail(PSY_IS_WINDOW(self));
    g_return_if_fail(PSY_IS_VISUAL_STIMULUS(stimulus));

    PsyWindowClass *klass = PSY_WINDOW_GET_CLASS(self);

    g_return_if_fail(klass->schedule_stimulus);

    klass->schedule_stimulus(self, stimulus);
}

/**
 * psy_window_get_frame_dur:
 * @self: A `PsyWindow` instance
 *
 * Returns:(transfer none): a `PsyDuration` for the duration between two frames
 */
PsyDuration *
psy_window_get_frame_dur(PsyWindow *self)
{
    PsyWindowPrivate *priv = psy_window_get_instance_private(self);

    g_return_val_if_fail(PSY_IS_WINDOW(self), NULL);
    return priv->frame_dur;
}

/**
 * psy_window_remove_stimulus:
 * @self: A `PsyWindow` instance.
 * @stimulus: A `PsyVisualStimulus` instance to be removed from the window
 *
 * In psy-lib, it's the window that initiates the drawing of stimuli. So
 * removing the stimulus from the window, means, it won't be scheduled anymore
 * Additionally this means that the `PsyStimulus::stopped` won't be scheduled
 * anymore.
 */
void
psy_window_remove_stimulus(PsyWindow *self, PsyVisualStimulus *stimulus)
{
    g_return_if_fail(PSY_IS_WINDOW(self));

    PsyWindowClass *klass = PSY_WINDOW_GET_CLASS(self);
    g_return_if_fail(klass->remove_stimulus);

    klass->remove_stimulus(self, stimulus);
}

/**
 * psy_window_set_projection_style:
 * @self: an instance of `PsyWindow`
 * @projection_style: an instance of `PsyWindowProjectionStyle` an | orable
 * combination of `PsyWindowProjectionStyle` Take note some flags may not be
 *      orred together and at least some must be set.
 *
 *
 * Set the style of projection of the window see `PsyWindow:projection-style`
 */
void
psy_window_set_projection_style(PsyWindow *self, gint projection_style)
{
    PsyWindowPrivate *priv  = psy_window_get_instance_private(self);
    PsyWindowClass   *klass = PSY_WINDOW_GET_CLASS(self);
    gint              style = projection_style;

    gint origin_style = 0;
    gint unit_style   = 0;

    g_return_if_fail(PSY_IS_WINDOW(self));

    if (style & PSY_WINDOW_PROJECTION_STYLE_C)
        origin_style++;
    if (style & PSY_WINDOW_PROJECTION_STYLE_CENTER)
        origin_style++;

    if (origin_style != 1) {
        g_critical("%s:You should set PSY_WINDOW_PROJECTION_STYLE_C or "
                   "PSY_WINDOW_PROJECTION_STYLE_CENTER",
                   __func__);
        return;
    }

    if (style & PSY_WINDOW_PROJECTION_STYLE_PIXELS)
        unit_style++;
    if (style & PSY_WINDOW_PROJECTION_STYLE_METER)
        unit_style++;
    if (style & PSY_WINDOW_PROJECTION_STYLE_MILLIMETER)
        unit_style++;
    if (style & PSY_WINDOW_PROJECTION_STYLE_VISUAL_DEGREES) {
        g_warning("Oops setting visual degrees is currently not supported.");
        unit_style++;
    }

    if (unit_style != 1) {
        g_critical("%s: You should set exactly one of:\n"
                   "   - PSY_WINDOW_PROJECTION_STYLE_PIXELS or\n"
                   "   - PSY_WINDOW_PROJECTION_STYLE_METER or\n"
                   "   - PSY_WINDOW_PROJECTION_STYLE_MILLIMETER or\n"
                   "   - PSY_WINDOW_PROJECTION_STYLE_VISUAL_DEGREES",
                   __func__);
        return;
    }

    priv->projection_style = style;

    PsyMatrix4 *projection = klass->create_projection_matrix(self);

    g_clear_object(&priv->projection_matrix);
    priv->projection_matrix = projection;
}

/**
 * psy_window_get_projection_style:
 * @self: an instance of `PsyWindow`
 *
 * Returns: the style of projection of the window see
 * `PsyWindow:projection-style`
 */
gint
psy_window_get_projection_style(PsyWindow *self)
{
    PsyWindowPrivate *priv = psy_window_get_instance_private(self);

    g_return_val_if_fail(PSY_IS_WINDOW(self), 0);

    return priv->projection_style;
}

/**
 * psy_window_get_projection:
 * @self: an instance of `PsyWindow`
 *
 * Returns:(transfer none): The projection matrix used by this window
 */
PsyMatrix4 *
psy_window_get_projection(PsyWindow *self)
{
    PsyWindowPrivate *priv = psy_window_get_instance_private(self);

    g_return_val_if_fail(PSY_IS_WINDOW(self), NULL);
    return priv->projection_matrix;
}

/**
 * psy_window_set_context:
 * @self: an instance of `PsyWindow`
 * @context:(transfer full): the drawing context for this window that draws
 * using whatever backend this window is using.
 *
 * Set the drawing context for this window
 *
 * private:
 */
void
psy_window_set_context(PsyWindow *self, PsyDrawingContext *context)
{
    g_return_if_fail(PSY_IS_WINDOW(self));
    g_return_if_fail(PSY_IS_DRAWING_CONTEXT(context));

    PsyWindowPrivate *priv = psy_window_get_instance_private(self);
    g_clear_object(&priv->context);
    priv->context = context;
}

/**
 * psy_window_get_context:
 * @self: an instance of `PsyWindow`
 *
 * Get the drawing context for this window for drawing purposes.
 *
 * Returns:(transfer none): The active drawing context that belongs to this
 *                          `PsyWindow`
 */
PsyDrawingContext *
psy_window_get_context(PsyWindow *self)
{
    g_return_val_if_fail(PSY_IS_WINDOW(self), NULL);
    PsyWindowPrivate *priv = psy_window_get_instance_private(self);

    return priv->context;
}

/**
 * psy_window_swap_stimuli:
 * @self: an instance of `PsyWindow`
 * @i1: The index of the first stimulus must be larger or equal than
 *      0 but smaller than PsyWindow:num-stims
 * @i2: the same for @i1
 *
 * Swap the order in which the visual stimuli are drawn.
 */
void
psy_window_swap_stimuli(PsyWindow *self, guint i1, guint i2)
{
    PsyWindowPrivate *priv = psy_window_get_instance_private(self);
    g_return_if_fail(PSY_WINDOW(self));
    g_return_if_fail(i1 < priv->stimuli->len);
    g_return_if_fail(i2 < priv->stimuli->len);

    PsyStimulus *stim_temp;
    stim_temp                = priv->stimuli->pdata[i1];
    priv->stimuli->pdata[i1] = priv->stimuli->pdata[i2];
    priv->stimuli->pdata[i2] = stim_temp;
}

/**
 * psy_window_get_num_stimuli:
 * @self: A `PsyWindow` instance
 *
 * Returns: the number of stimuli contained by the window
 */
guint
psy_window_get_num_stimuli(PsyWindow *self)
{
    PsyWindowPrivate *priv = psy_window_get_instance_private(self);
    g_return_val_if_fail(PSY_WINDOW(self), 0);

    return priv->stimuli->len;
}
