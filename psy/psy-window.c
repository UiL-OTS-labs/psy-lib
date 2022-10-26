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
 *    sure that the deriving window will actually draw every stimulus.
 */
#include "psy-duration.h"
#include "psy-time-point.h"
#include "psy-visual-stimulus.h"
#include "psy-window.h"

typedef struct PsyWindowPrivate {
    gint            monitor;
    guint           n_frames;
    gint            width_mm, height_mm;
    gfloat          back_ground_color[4];

    GHashTable*     stimuli;
    GTree*          sorted_stimuli;
    PsyDuration*    frame_dur;
} PsyWindowPrivate;

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE(PsyWindow, psy_window, G_TYPE_OBJECT)

typedef enum {
    CLEAR,
    DRAW_STIMULI,
    LAST_SIGNAL,
} PsyWindowSignal;

typedef enum {
    N_MONITOR = 1,
    BACKGROUND_COLOR_VALUES,
    WIDTH_MM,
    HEIGHT_MM,
    FRAME_DUR,
    N_PROPS
} PsyWindowProperty;


static void
psy_window_set_property(GObject        *object,
                        guint           property_id,
                        const GValue   *value,
                        GParamSpec     *spec)
{
    PsyWindow* self = PSY_WINDOW(object);

    switch((PsyWindowProperty) property_id) {
        case N_MONITOR:
            psy_window_set_monitor(self, g_value_get_int(value));
            break;
        case BACKGROUND_COLOR_VALUES:
            psy_window_set_background_color_values(
                    self,
                    g_value_get_pointer(value));
            break;
        case WIDTH_MM:
            psy_window_set_width_mm(self, g_value_get_int(value));
            break;
        case HEIGHT_MM:
            psy_window_set_height_mm(self, g_value_get_int(value));
            break;
        case FRAME_DUR:
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, spec);
    }
}

static void
psy_window_get_property(GObject        *object,
                        guint           property_id,
                        GValue         *value,
                        GParamSpec     *spec)
{
    PsyWindow* self = PSY_WINDOW(object);

    switch((PsyWindowProperty) property_id) {
        case N_MONITOR:
            g_value_set_int(value, psy_window_get_monitor(self));
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
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, spec);
    }
}

static gint
stimulus_cmp(gconstpointer s1, gconstpointer s2, gpointer data)
{
    (void) data;
    PsyStimulus *stim1 = PSY_STIMULUS((gpointer)s1);
    PsyStimulus *stim2 = PSY_STIMULUS((gpointer)s2);

    PsyTimePoint *t1 = psy_stimulus_get_start_time(stim1);
    PsyTimePoint *t2 = psy_stimulus_get_start_time(stim2);

    // This way the stimuli are sorted on start time
    if (psy_time_point_less(t1, t2))
        return -1;
    else if(psy_time_point_greater(t1, t2))
        return 1;

    // Allow multiple stimuli with same start time
    if (s1 == s2)
        return 0;
    else if (s1 < s2)
        return -2;
    else
        return 2;
}

static void
psy_window_init(PsyWindow* self)
{
    PsyWindowPrivate* priv = psy_window_get_instance_private(self);
    gfloat default_bg[4] = {
        0.5, 0.5, 0.5, 1.0
    };

//
//    priv->stimuli = g_hash_table_new_full(
//            g_direct_hash,
//            g_direct_equal,
//            g_object_unref,
//            NULL
//            );

    priv->sorted_stimuli = g_tree_new_full(
            stimulus_cmp,
            NULL,
            g_object_unref,
            NULL);

    memcpy(priv->back_ground_color, default_bg, sizeof(default_bg));
}

static void
psy_window_dispose(GObject* gobject)
{
    PsyWindowPrivate* priv = psy_window_get_instance_private(
            PSY_WINDOW(gobject)
            );

    if (priv->sorted_stimuli) {
        g_tree_destroy(priv->sorted_stimuli);
        priv->sorted_stimuli = NULL;
    }

    if (priv->stimuli) {
        g_hash_table_destroy(priv->stimuli);
        priv->stimuli = NULL;
    }

    G_OBJECT_CLASS(psy_window_parent_class)->dispose(gobject);
}

static void
psy_window_finalize(GObject* gobject)
{
    PsyWindowPrivate* priv = psy_window_get_instance_private(
            PSY_WINDOW(gobject)
            );
    (void) priv;

    G_OBJECT_CLASS(psy_window_parent_class)->finalize(gobject);
}

static GParamSpec* obj_properties[N_PROPS];
//static guint window_signals[LAST_SIGNAL];

static gint
get_monitor(PsyWindow* self) {
    PsyWindowPrivate* priv = psy_window_get_instance_private(self);
    return priv->monitor;
}

static void
set_monitor(PsyWindow* self, gint nth_monitor) {
    PsyWindowPrivate* priv = psy_window_get_instance_private(self);
    priv->monitor = nth_monitor;
}

static void
set_frame_dur(PsyWindow* self, PsyDuration* dur)
{
    PsyWindowPrivate* priv = psy_window_get_instance_private(self);
    priv->frame_dur = dur;
}

static void
remove_stimulus(PsyWindow* self, PsyVisualStimulus* stimulus)
{
    PsyWindowPrivate* priv = psy_window_get_instance_private(self);
    g_tree_remove(priv->sorted_stimuli, stimulus);
}

static void
draw(PsyWindow* self, guint64 frame_num, PsyTimePoint* tp)
{
    PsyWindowClass* cls = PSY_WINDOW_GET_CLASS(self);
    g_return_if_fail(cls->clear);
    g_return_if_fail(cls->draw_stimuli);

    cls->clear(self);
    cls->draw_stimuli(self, frame_num, tp);
}

static void
draw_stimuli(PsyWindow* self, guint64 frame_num, PsyTimePoint* tp)
{
    PsyWindowPrivate* priv = psy_window_get_instance_private(self);
    PsyWindowClass* klass = PSY_WINDOW_GET_CLASS(self);
    GTreeNode* node;

    for ( node = g_tree_node_first(priv->sorted_stimuli);
          node;
          node = g_tree_node_next(node) ) {
        
        PsyStimulus* stim = g_tree_node_key(node);
        PsyVisualStimulus* vstim = PSY_VISUAL_STIMULUS(stim);
        gint64 start_frame, nth_frame, num_frames;

        /* Schedule if necessary*/
        if (!psy_visual_stimulus_is_scheduled(vstim) ) {
            PsyTimePoint *start = psy_stimulus_get_start_time(stim);
            PsyDuration *wait = psy_time_point_subtract(start, tp);
            gint64 num_frames_away = psy_duration_divide_rounded(
                    wait, priv->frame_dur
                    );
            if (num_frames_away < 0) {
                g_warning(
                        "Scheduling a stimulus that should have been presented "
                        "in the past, the stimulus will be presented as "
                        "quickly as possible.");
                num_frames_away = 0;
            }

            psy_visual_stimulus_set_start_frame (
                    vstim,
                    frame_num + num_frames_away
                    );
        }

        start_frame = psy_visual_stimulus_get_start_frame(vstim);
        nth_frame = psy_visual_stimulus_get_nth_frame(vstim);
        num_frames = psy_visual_stimulus_get_num_frames(vstim);
        if (start_frame <= (gint64) frame_num) {
            psy_visual_stimulus_update(vstim, tp, nth_frame);
            klass->draw_stimulus(self, vstim);
        }
        nth_frame = psy_visual_stimulus_get_nth_frame(vstim);
        if (nth_frame == 1) {
            psy_stimulus_set_is_started(PSY_STIMULUS(stim), tp);
        }

        if (nth_frame >= num_frames) {
            PsyTimePoint* tend = psy_time_point_add(tp, priv->frame_dur);
            psy_stimulus_set_is_finished(PSY_STIMULUS(stim), tend);
            psy_window_remove_stimulus(self, stim);
        }
    }
}

static void
set_monitor_size_mm(PsyWindow* self, gint width_mm, gint height_mm)
{
    PsyWindowPrivate* priv = psy_window_get_instance_private(self);
    priv->width_mm = width_mm;
    priv->height_mm = height_mm;
}

static void
psy_window_class_init(PsyWindowClass* klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    object_class->set_property  = psy_window_set_property;
    object_class->get_property  = psy_window_get_property;
    object_class->dispose       = psy_window_dispose;
    object_class->finalize      = psy_window_finalize;

    klass->get_monitor          = get_monitor;
    klass->set_monitor          = set_monitor;

    klass->draw                 = draw;
    klass->draw_stimuli         = draw_stimuli;
    klass->set_monitor_size_mm  = set_monitor_size_mm;

    klass->set_frame_dur        = set_frame_dur;

    klass->remove_stimulus      = remove_stimulus;

    /**
     * PsyWindow:n-monitor:
     *
     * The number of the monitor on which the PsyWindow should be
     * presented.
     */
    obj_properties[N_MONITOR] =
        g_param_spec_int("n-monitor",
                         "nmonitor",
                         "The number of the monitor to use for this window",
                         -1,
                         G_MAXINT32,
                         0,
                         G_PARAM_CONSTRUCT | G_PARAM_READWRITE
                         );

    /**
     * PsyWindow:bg-color-values
     *
     * The color of the background, you can use this property to get/set
     * the background color of the window. It is basically, an array of 4 floats
     * in RGBA format where the color values range between 0.0 and 1.0.
     */
    obj_properties [BACKGROUND_COLOR_VALUES] = 
        g_param_spec_pointer(
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
    obj_properties [WIDTH_MM] = 
        g_param_spec_int(
                "width-mm",
                "WidthMM",
                "The width of the window in mm, assuming a full screen window",
                -1,
                G_MAXINT32,
                -1,
                G_PARAM_READWRITE
                );
    
    /**
     * PsyWindow:height-mm:
     *
     * The height of the monitor of the window in mm may be -1, it's
     * uninitialized, 0 we don't know, or > 0 the height in mm, one should always
     * check with a ruler to verify, as we try to determine it from the
     * backend/os.
     * If one knows better than the os/backend, one could set it self, don't
     * worry it's unlikely that the physical size of your monitor will change.
     */
    obj_properties [HEIGHT_MM] = 
        g_param_spec_int(
                "height-mm",
                "HeightMM",
                "The height of the window in mm, assuming a full screen window",
                -1,
                G_MAXINT32,
                -1,
                G_PARAM_READWRITE 
                );

    /**
     * PsyWindow:frame-dur:
     *
     * The duration of one frame, this will be the reciprocal of the framerate
     * of the monitor on which this window is presented.
     */
    obj_properties[FRAME_DUR] = g_param_spec_object(
            "frame-dur",
            "FrameDur",
            "The duration of one frame.",
            PSY_TYPE_DURATION,
            G_PARAM_READABLE
            );

    g_object_class_install_properties(object_class, N_PROPS, obj_properties);

//    /**
//     * PsyWindow::clear
//     *
//     * This is the first action that is run when a new frame should be
//     * presented. The default handler calls the private/protected clear function
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
//     * presented. The default handler calls the private/protected clear function
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
psy_window_get_monitor(PsyWindow* self)
{
    g_return_val_if_fail(PSY_IS_WINDOW(self), -1);

    PsyWindowClass* class = PSY_WINDOW_GET_CLASS(self);
    g_return_val_if_fail(class->get_monitor, -1);

    return class->get_monitor(self);
}

/**
 * psy_window_set_monitor:
 * @self: a #PsyWindow instance
 * @n: the number of the monitor on which to display the window
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
psy_window_set_monitor(PsyWindow* self, gint nth_monitor)
{
    g_return_if_fail(PSY_IS_WINDOW(self));

    PsyWindowClass* class = PSY_WINDOW_GET_CLASS(self);
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
psy_window_set_background_color_values(PsyWindow* self, gfloat* color)
{
    g_return_if_fail(PSY_IS_WINDOW(self));
    PsyWindowPrivate* priv = psy_window_get_instance_private(self);

    memcpy(priv->back_ground_color,
           color,
           sizeof(priv->back_ground_color));
}

/**
 * psy_window_get_background_color_values:
 * @self: a #PsyWindow instance
 * @color:(out callee-allocates)(array fixed-size=4)(element-type gfloat): the desired
 *          color values in RGBA format.
 *
 * Get the background color of this window.
 */
void
psy_window_get_background_color_values(PsyWindow* self, gfloat* color)
{
    g_return_if_fail(PSY_IS_WINDOW(self));
    PsyWindowPrivate* priv = psy_window_get_instance_private(self);

    memcpy(color,
           priv->back_ground_color,
           sizeof(priv->back_ground_color));
}

/**
 * psy_window_get_width_height_mm:
 * @window: a #PsyWindow instance
 * @width_mm:(out)(nullable): The width in mm.
 * @height_mm:(out)(nullable): The height in mm.
 *
 * Obtain the width and height of the window. A negative (or 0) return value
 * indicates that we were not able to establish the width and/or height.
 */
void
psy_window_get_width_height_mm(PsyWindow* window, gint* width_mm, gint* height_mm)
{
    g_return_if_fail(PSY_IS_WINDOW(window));
    PsyWindowPrivate* private = psy_window_get_instance_private(window);
    if (width_mm)
        *width_mm = private->width_mm;
    if (height_mm)
        *height_mm = private->height_mm;
}

/**
 * psy_window_get_width_mm:
 * @window:A #PsyWindow instance
 *
 * Returns: the width in mm of the window
 */
gint
psy_window_get_width_mm(PsyWindow* window)
{
    g_return_val_if_fail(PSY_IS_WINDOW(window), -1);
    PsyWindowPrivate* private = psy_window_get_instance_private(window);
    return private->width_mm;
}

/**
 * psy_window_set_width_mm:
 * @window:A #PsyWindow instance
 * @width_mm: the width of the window/monitor
 *
 * Set the width of the window. Override the settings as found by the os/backend
 */
void
psy_window_set_width_mm(PsyWindow* window, gint width_mm)
{
    g_return_if_fail(PSY_IS_WINDOW(window));
    PsyWindowPrivate* private = psy_window_get_instance_private(window);
    private->width_mm = width_mm;
}

/**
 * psy_window_get_height_mm:
 * @window:A #PsyWindow instance
 *
 * Returns: the height in mm of the window
 */
gint
psy_window_get_height_mm(PsyWindow* window)
{
    g_return_val_if_fail(PSY_IS_WINDOW(window), -1);
    PsyWindowPrivate* private = psy_window_get_instance_private(window);
    return private->height_mm;
}

/**
 * psy_window_set_height_mm:
 * @window:A #PsyWindow instance
 * @height_mm: the height of the window/monitor
 *
 * Set the height of the window. Override the settings as found by the os/backend
 */
void
psy_window_set_height_mm(PsyWindow* window, gint height_mm)
{
    g_return_if_fail(PSY_IS_WINDOW(window));
    PsyWindowPrivate* private = psy_window_get_instance_private(window);
    private->height_mm = height_mm;
}

/**
 * psy_window_schedule_stimulus:
 * @window: a PsyWindow instance
 * @stimulus: a `PsyVisualStimulus` instance that should be drawn on this window
 *
 * Notifies the window about a stimulus that should be drawn on it, if the
 * stimulus was already present, it is ignored.
 */
void
psy_window_schedule_stimulus(PsyWindow* window, PsyVisualStimulus* stimulus)
{
    PsyWindowPrivate* priv = psy_window_get_instance_private(window);

    g_return_if_fail(PSY_IS_WINDOW(window));
    g_return_if_fail(PSY_IS_VISUAL_STIMULUS(stimulus));

    // Check if the stimulus is already scheduled
    if (g_tree_lookup(priv->sorted_stimuli, stimulus) != NULL)
        return;

    g_object_ref(stimulus);
    g_tree_insert_node(priv->sorted_stimuli, stimulus, NULL);
}


PsyDuration*
psy_window_get_frame_dur(PsyWindow* window)
{
    PsyWindowPrivate *priv = psy_window_get_instance_private(window);

    g_return_val_if_fail(PSY_IS_WINDOW(window), NULL);
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
psy_window_remove_stimulus(PsyWindow* self, PsyVisualStimulus* stimulus)
{
    g_return_if_fail(PSY_IS_WINDOW(self));

    PsyWindowClass* klass = PSY_WINDOW_GET_CLASS(self);
    klass->remove_stimulus(self, stimulus);
}

