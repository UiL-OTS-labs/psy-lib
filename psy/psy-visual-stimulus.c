
#include <math.h>

#include "psy-color.h"
#include "psy-visual-stimulus.h"
#include "glibconfig.h"
#include "psy-stimulus.h"
#include "psy-window.h"

typedef struct PsyVisualStimulusPrivate {
     PsyWindow *window;
     gint64     nth_frame;
     gint64     num_frames;
     gint64     start_frame;
     gfloat     x, y, z;
     PsyColor  *color;
} PsyVisualStimulusPrivate;

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE(
        PsyVisualStimulus, psy_visual_stimulus, PSY_TYPE_STIMULUS
        )

typedef enum {
    PROP_NULL,          // not used required by GObject
    PROP_WINDOW,        // The window on which this stimulus should be drawn
    PROP_NUM_FRAMES,    // The number of frames the stimulus will be presented
    PROP_NTH_FRAME,     // the frame for which we are rendering
    PROP_START_FRAME,   // the frame at which this object should be first presented
    PROP_X,             // the x coordinate of the stimulus
    PROP_Y,             // the y coordinate of the stimulus
    PROP_Z,             // the z coordinate of the stimulus
    PROP_COLOR,         // the fill color of the stimulus.
    NUM_PROPERTIES
} VisualStimulusProperty;

typedef enum {
    SIG_UPDATE,     // Called when the stimulus is updated for the next frame
    NUM_SIGNALS
} VisualStimulusSignals;

static GParamSpec*  visual_stimulus_properties[NUM_PROPERTIES] = {0};
static guint        visual_stimulus_signals[NUM_SIGNALS] = {0};

static void
psy_visual_stimulus_set_property(GObject       *object,
                                 guint          property_id,
                                 const GValue  *value,
                                 GParamSpec    *pspec
                                 )
{
    PsyVisualStimulus* self = PSY_VISUAL_STIMULUS(object);

    switch((VisualStimulusProperty) property_id) {
        case PROP_WINDOW:
            psy_visual_stimulus_set_window(self, g_value_get_object(value));
            break;
        case PROP_X:
            psy_visual_stimulus_set_x(self, g_value_get_float(value));
            break;
        case PROP_Y:
            psy_visual_stimulus_set_y(self, g_value_get_float(value));
            break;
        case PROP_Z:
            psy_visual_stimulus_set_z(self, g_value_get_float(value));
            break;
        case PROP_COLOR:
            psy_visual_stimulus_set_color(self, g_value_get_object(value));
            break;
        case PROP_NUM_FRAMES: // gettable only
        case PROP_NTH_FRAME:  // gettable only
        case PROP_START_FRAME:
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

static void
psy_visual_stimulus_get_property(GObject       *object,
                                 guint          property_id,
                                 GValue        *value,
                                 GParamSpec    *pspec
                                 )
{
    PsyVisualStimulus        *self = PSY_VISUAL_STIMULUS(object);
    PsyVisualStimulusPrivate *priv = psy_visual_stimulus_get_instance_private(self);

    switch((VisualStimulusProperty) property_id) {
        case PROP_NUM_FRAMES:
            g_value_set_int64(value, priv->num_frames);
            break;
        case PROP_NTH_FRAME:
            g_value_set_int64(value, priv->nth_frame);
            break;
        case PROP_WINDOW:
            g_value_set_object(value, priv->window);
            break;
        case PROP_START_FRAME:
            g_value_set_int64(value, priv->start_frame);
            break;
        case PROP_X:
            g_value_set_float(value, priv->x);
            break;
        case PROP_Y:
            g_value_set_float(value, priv->y);
            break;
        case PROP_Z:
            g_value_set_float(value, priv->z);
            break;
        case PROP_COLOR:
            g_value_set_object(value, psy_visual_stimulus_get_color(self));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

static void
psy_visual_stimulus_dispose(GObject* object)
{
    PsyVisualStimulusPrivate* priv = psy_visual_stimulus_get_instance_private(
            PSY_VISUAL_STIMULUS(object)
            );

    g_clear_object(&priv->color);
    g_clear_object(&priv->window);

    G_OBJECT_CLASS(psy_visual_stimulus_parent_class)->dispose(object);
}

static void
psy_visual_stimulus_init(PsyVisualStimulus* self)
{
    PsyVisualStimulusPrivate *priv = psy_visual_stimulus_get_instance_private(self);
    priv->window        =  NULL;
    priv->nth_frame     =  0;
    priv->num_frames    = -1;
    priv->start_frame   =  0;
}

static void
visual_stimulus_update(
        PsyVisualStimulus* self, PsyTimePoint* frame_time, gint64 nth_frame
        )
{
    PsyVisualStimulusPrivate* priv = psy_visual_stimulus_get_instance_private(self);
    (void) frame_time;
    (void) nth_frame;
    priv->nth_frame++;
}

static void
visual_stimulus_play(PsyStimulus* stimulus, PsyTimePoint* start_time)
{
    PsyVisualStimulus* vstim = PSY_VISUAL_STIMULUS(stimulus);

    PsyWindow* window = psy_visual_stimulus_get_window(vstim);

    psy_window_schedule_stimulus(window, vstim);

    PSY_STIMULUS_CLASS(psy_visual_stimulus_parent_class)->play(
            stimulus, start_time
            );
}

static void
visual_stimulus_set_duration(PsyStimulus* self, PsyDuration* stim_dur)
{
    PsyVisualStimulusPrivate* priv = psy_visual_stimulus_get_instance_private(
            PSY_VISUAL_STIMULUS(self)
            );

    PsyDuration* frame_dur = psy_window_get_frame_dur(priv->window);
    
    if (psy_duration_less(stim_dur, frame_dur)) {
        g_warning("Specified duration is less than one frame");
    }

    gint64 num_frames = psy_duration_divide_rounded(stim_dur, frame_dur);
    PsyDuration* corrected_dur = psy_duration_multiply_scalar(
            frame_dur,
            num_frames
            );
    priv->num_frames = num_frames;
    PSY_STIMULUS_CLASS(psy_visual_stimulus_parent_class)->set_duration(
            self, corrected_dur
            );    
}


static void
psy_visual_stimulus_class_init(PsyVisualStimulusClass* klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    object_class->get_property = psy_visual_stimulus_get_property;
    object_class->set_property = psy_visual_stimulus_set_property;
    object_class->dispose = psy_visual_stimulus_dispose;

    PsyStimulusClass* stimulus_class = PSY_STIMULUS_CLASS(klass);
    stimulus_class->play        = visual_stimulus_play;
    stimulus_class->set_duration= visual_stimulus_set_duration;

    klass->update = visual_stimulus_update;

    /**
     * PsyVisualStimulus:window:
     *
     * The window on which this stimulus will be played/displays when it
     * is started as stimulus. It should not be NULL, so it should be specified
     * when constructing the window.
     */
    visual_stimulus_properties[PROP_WINDOW] = g_param_spec_object (
            "window",
            "Window",
            "The window on which this stimulus will be drawn",
            PSY_TYPE_WINDOW,
            G_PARAM_WRITABLE | G_PARAM_CONSTRUCT
            );

    /**
     * PsyVisualStimulus:num-frames:
     * 
     * When a visual stimulus is played for an amount of time, the window
     * on which it will be drawn will be determine when it will be presented for
     * the first time.
     * This value will be most useful in the update signal handler or in
     * `psy_visual_stimulus_update`.
     */
    visual_stimulus_properties[PROP_NUM_FRAMES] = g_param_spec_int64(
            "num-frames",
            "NumFrames",
            "The number of frames that this stimulus will be displayed.",
            -1, G_MAXINT64, -1,
            G_PARAM_READABLE
            );

    /**
     * PsyVisualStimulus:nth-frame:
     * 
     * Everytime the window draws a stimulus, you'll get the time to update it.
     * This value will be most useful in the update signal handler or in
     * `psy_visual_stimulus_update`.
     */
    visual_stimulus_properties[PROP_NTH_FRAME] = g_param_spec_int64(
            "nth-frame",
            "NthFrame",
            "The nth frame of this stimulus.",
            -1, G_MAXINT64, -1,
            G_PARAM_READABLE
            );

    /**
     * PsyVisualStimulus:start-frame:
     *
     * When visual stimuli are scheduled, we have to specify a given frame
     * on which the stimulus will be presented for the first time.
     *
     */
    visual_stimulus_properties[PROP_START_FRAME] = g_param_spec_int64(
            "start-frame",
            "StartFrame",
            "The number of the frame on which this stimulus should be presented",
            0, G_MAXINT64, 0,
            G_PARAM_READABLE
            );

    /**
     * PsyVisualStimulus:x
     *
     * The x coordinate of the stimulus
     */
    visual_stimulus_properties[PROP_X] = g_param_spec_float(
            "x",
            "x-coordinate",
            "the x coordinate of the stimulus",
            -G_MAXFLOAT,
            G_MAXFLOAT,
            0,
            G_PARAM_READWRITE
            );
    
    /**
     * PsyVisualStimulus:y
     *
     * The y coordinate of the stimulus
     */
    visual_stimulus_properties[PROP_Y] = g_param_spec_float(
            "y",
            "y-coordinate",
            "the y coordinate of the stimulus",
            -G_MAXFLOAT,
            G_MAXFLOAT,
            0,
            G_PARAM_READWRITE
            );
    
    /**
     * PsyVisualStimulus:z
     *
     * The z coordinate of the stimulus
     */
    visual_stimulus_properties[PROP_Z] = g_param_spec_float(
            "z",
            "z-coordinate",
            "the z coordinate of the stimulus",
            -G_MAXFLOAT,
            G_MAXFLOAT,
            0,
            G_PARAM_READWRITE
            );

    /**
     * PsyVisualStimulus:color
     *
     * The color `PsyColor` used to fill this object with
     */
    visual_stimulus_properties[PROP_COLOR] = g_param_spec_object(
            "color",
            "Color",
            "The fill color for this stimulus",
            PSY_TYPE_COLOR,
            G_PARAM_READWRITE
            );
    
    g_object_class_install_properties(
            object_class, NUM_PROPERTIES, visual_stimulus_properties
            );

    /**
     * Stimulus::update:
     * @self: the instance of #PsyVisualStimulus on which this signal is emitted
     * @frame_time: The time when this frame should become visible
     * @nth_frame: The number of the frame, in unison with
     *             #PsyVisualStimulus:num_frames this may be used for animations.
     *
     * This signal is emitted so a client may update certain parameters of a visual
     * stimulus. These parameters make it ready for drawing on a new frame at the time
     * specified by @frame_time.
     */
    visual_stimulus_signals [SIG_UPDATE] = g_signal_new(
            "update",
            PSY_TYPE_VISUAL_STIMULUS,
            G_SIGNAL_RUN_FIRST,
            G_STRUCT_OFFSET(PsyVisualStimulusClass, update),
            NULL,
            NULL,
            NULL,
            G_TYPE_NONE,
            2,
            PSY_TYPE_TIME_POINT,
            G_TYPE_INT64
            );
}

/**
 * psy_visual_stimulus_get_window:
 * @stimulus: a `PsyVisualStimulus`
 *
 * Get the window on which this stimulus should be drawn.
 *
 * Returns: (nullable) (transfer none): The `PsyWindow` on which this stimulus
 *                                      should be drawn.
 */
PsyWindow*
psy_visual_stimulus_get_window(PsyVisualStimulus* stimulus)
{
    PsyVisualStimulusPrivate* priv = psy_visual_stimulus_get_instance_private(stimulus);

    g_return_val_if_fail(PSY_IS_VISUAL_STIMULUS(stimulus), NULL);

    return priv->window;
}

/**
 * psy_visual_stimulus_set_window:
 * @stimulus: a `PsyVisualStimulus`
 * @window:(transfer full): a `PsyWindow` to draw this stimulus on.
 *
 * Set the window on which this stimulus should be drawn.
 */
void
psy_visual_stimulus_set_window(PsyVisualStimulus* stimulus,
                               PsyWindow* window)
{
    PsyVisualStimulusPrivate* priv = psy_visual_stimulus_get_instance_private(stimulus);
    
    g_return_if_fail(PSY_IS_VISUAL_STIMULUS(stimulus));
    g_return_if_fail(PSY_IS_WINDOW(window));

    g_clear_object(&priv->window);
    priv->window = g_object_ref(window);
}

/*
 * psy_visual_stimulus_emit_update:
 * @self the #PsyVisualStimulus instance in need of an update for a comming frame.
 * @frame_time:(transfer none): The time at which the next frame when the new
 *      frame is to be presented.
 * @nth_frame: the number of the frame, at each presentation the number of the frame
 *              should be incremented with 1.
 *
 * stability:private
 *
 * This function should not be used by 3rd party libraries. It designed in order
 * to the PsyWindows to indicate when they will be drawing the frame, in order
 * for the client to update the frame. Calling this function yourself, will probably
 * only be confusing. If you want to to update this stimulus, you should connect to
 * the `PsyVisualStimulus::update` signal handler and update the stimulus from there.
 * One could also override the update method in order to update the stimulus.
 * 
 */
void
psy_visual_stimulus_emit_update (
        PsyVisualStimulus  *self,
        PsyTimePoint       *frame_time,
        gint64              nth_frame
        )
{
    g_return_if_fail(PSY_IS_STIMULUS(self));
    g_return_if_fail(PSY_IS_TIME_POINT(frame_time));

    g_signal_emit(
            self,
            visual_stimulus_signals[SIG_UPDATE],
            0,
            frame_time,
            nth_frame
            );
}

/**
 * psy_visual_stimulus_get_num_frames:
 * @self: an instance of `PsyVisualStimulus`.
 *
 * This function returns how many times the stimulus is going to be presented
 * This number reflects the duration of the stimulus. e.g A stimulus with a
 * duration of 50 ms will be presented precisely 3 frames at 60Hz monitors.
 *
 * Returns: An integer reflecting how many frames this stimulus has been presented.
 */
gint64
psy_visual_stimulus_get_num_frames(PsyVisualStimulus* self)
{
    PsyVisualStimulusPrivate* priv = psy_visual_stimulus_get_instance_private(self);
    g_return_val_if_fail(PSY_IS_VISUAL_STIMULUS(self), -1);

    return priv->num_frames;
}

/**
 * psy_visual_stimulus_get_nth_frame:
 * @self: an instance of `PsyVisualStimulus`.
 *
 * This function returns how many times the stimulus has be presented. Notice
 * that this starts at 0 when preparing the first frame.
 *
 * Returns: An integer reflecting how many frames this stimulus has been presented.
 */
gint64
psy_visual_stimulus_get_nth_frame(PsyVisualStimulus* self)
{
    PsyVisualStimulusPrivate* priv = psy_visual_stimulus_get_instance_private(self);
    g_return_val_if_fail(PSY_IS_VISUAL_STIMULUS(self), -1);

    return priv->nth_frame;
}

gboolean
psy_visual_stimulus_is_scheduled(PsyVisualStimulus* self)
{
    PsyVisualStimulusPrivate* priv = psy_visual_stimulus_get_instance_private(self);
    g_return_val_if_fail(PSY_IS_VISUAL_STIMULUS(self), FALSE);

    return priv->start_frame > 0;
}

/**
 * psy_visual_stimulus_set_start_frame:
 * @self: an instance of `PsyVisualStimulus`
 * @frame_num: the number of the frame on which this stimulus should start
 *
 * Sets the frame number of the frame of a monitor on which this stimulus should
 * start.
 *
 * stability:private
 */
void
psy_visual_stimulus_set_start_frame(PsyVisualStimulus* self, gint64 frame_num)
{
    PsyVisualStimulusPrivate* priv = psy_visual_stimulus_get_instance_private(self);
    g_return_if_fail(PSY_IS_VISUAL_STIMULUS(self));
    
    priv->start_frame = frame_num;
}

/**
 * psy_visual_stimulus_get_start_frame:
 * @self: an instance of `PsyVisualStimulus`
 *
 * Gets the frame number of the frame of a monitor on which this stimulus should
 * start.
 *
 * Returns: The number of the frame of the `PsyWindow` that @self should be
 *          first presented.
 */
gint64
psy_visual_stimulus_get_start_frame(PsyVisualStimulus* self)
{
    PsyVisualStimulusPrivate* priv = psy_visual_stimulus_get_instance_private(self);
    g_return_val_if_fail(PSY_IS_VISUAL_STIMULUS(self), -1);
    
    return priv->start_frame;
}

/**
 * psy_visual_stimulus_get_x:
 * @self: an instance of `PsyVisualStimulus`
 *
 * Get the x coordinate of the stimulus.
 *
 * Returns: the x coordinate of the stimulus
 */
gfloat
psy_visual_stimulus_get_x(PsyVisualStimulus* self)
{
    PsyVisualStimulusPrivate* priv = psy_visual_stimulus_get_instance_private(self);
    g_return_val_if_fail(PSY_IS_VISUAL_STIMULUS(self), NAN);

    return priv->x;
}

/**
 * psy_visual_stimulus_set_x:
 * @self: an instance of `PsyVisualStimulus`
 * @x: a `gfloat` representing the x coordinate.
 *
 * Set the new value for the x-coordinate
 */
void
psy_visual_stimulus_set_x(PsyVisualStimulus* self, gfloat x)
{
    PsyVisualStimulusPrivate* priv = psy_visual_stimulus_get_instance_private(self);
    g_return_if_fail(PSY_IS_VISUAL_STIMULUS(self));

    priv->x = x;
}

/**
 * psy_visual_stimulus_get_y:
 * @self: an instance of `PsyVisualStimulus`
 *
 * Get the y coordinate of the stimulus.
 *
 * Returns: the y coordinate of the stimulus
 */
gfloat
psy_visual_stimulus_get_y(PsyVisualStimulus* self)
{
    PsyVisualStimulusPrivate* priv = psy_visual_stimulus_get_instance_private(self);
    g_return_val_if_fail(PSY_IS_VISUAL_STIMULUS(self), NAN);

    return priv->y;
}

/**
 * psy_visual_stimulus_set_y:
 * @self: an instance of `PsyVisualStimulus`
 * @y: a `gfloat` representing the y coordinate.
 *
 * Set the new value for the y-coordinate
 */
void
psy_visual_stimulus_set_y(PsyVisualStimulus* self, gfloat y)
{
    PsyVisualStimulusPrivate* priv = psy_visual_stimulus_get_instance_private(self);
    g_return_if_fail(PSY_IS_VISUAL_STIMULUS(self));

    priv->y = y;
}

/**
 * psy_visual_stimulus_get_z:
 * @self: an instance of `PsyVisualStimulus`
 *
 * Get the z coordinate of the stimulus.
 *
 * Returns: the z coordinate of the stimulus
 */
gfloat
psy_visual_stimulus_get_z(PsyVisualStimulus* self)
{
    PsyVisualStimulusPrivate* priv = psy_visual_stimulus_get_instance_private(self);
    g_return_val_if_fail(PSY_IS_VISUAL_STIMULUS(self), NAN);

    return priv->z;
}

/**
 * psy_visual_stimulus_set_z:
 * @self: an instance of `PsyVisualStimulus`
 * @z: a `gfloat` representing the z coordinate.
 *
 * Set the new value for the z-coordinate
 */
void
psy_visual_stimulus_set_z(PsyVisualStimulus* self, gfloat z)
{
    PsyVisualStimulusPrivate* priv = psy_visual_stimulus_get_instance_private(self);
    g_return_if_fail(PSY_IS_VISUAL_STIMULUS(self));

    priv->z = z;
}

/**
 * psy_visual_stimulus_get_color:
 * @self: An instance of `PsyVisualStimulus`
 *
 * Get the color of the window.
 *
 * Returns:(transfer none): the `PsyColor` of used to fill the stimuli
 */
PsyColor*
psy_visual_stimulus_get_color(PsyVisualStimulus* self)
{
    PsyVisualStimulusPrivate* priv = psy_visual_stimulus_get_instance_private(self);
    g_return_val_if_fail(PSY_IS_VISUAL_STIMULUS(self), NULL);

    return priv->color;
}

/**
 * psy_visual_stimulus_set_color:
 * @self: an instance of `PsyVisualStimulus`
 * @color: An instance of `PsyVisualStimulus` that is going to
 *         be used in order to fill the shape of the stimulus
 *
 * Set the fill color of the stimulus, this color is used to fill the stimulus
 */
void
psy_visual_stimulus_set_color(PsyVisualStimulus* self, PsyColor* color)
{
    PsyVisualStimulusPrivate* priv = psy_visual_stimulus_get_instance_private(self);

    g_return_if_fail(PSY_IS_VISUAL_STIMULUS(self) && PSY_IS_COLOR(color));

    g_clear_object(&priv->color);
    priv->color = g_object_ref(color);    
}


