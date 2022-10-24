
#include "psy-visual-stimulus.h"
#include "psy-stimulus.h"
#include "psy-window.h"

typedef struct PsyVisualStimulusPrivate {
     PsyWindow *window;
     gint64     nth_frame;
     gint64     num_frames;
} PsyVisualStimulusPrivate;

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE(
        PsyVisualStimulus, psy_visual_stimulus, PSY_TYPE_STIMULUS
        )

typedef enum {
    PROP_NULL,      // not used required by GObject
    PROP_WINDOW,    // The window on which this stimulus should be drawn
    PROP_NUM_FRAMES,// The number of frames the stimulus will be presented
    PROP_NTH_FRAME, // the frame for wich we are rendering
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
        case PROP_NUM_FRAMES: // gettable only
        case PROP_NTH_FRAME:  // gettable only
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
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

static void
psy_visual_stimulus_init(PsyVisualStimulus* self)
{
    PsyVisualStimulusPrivate *priv = psy_visual_stimulus_get_instance_private(self);
    priv->window     = NULL;
    priv->nth_frame  = -1;
    priv->num_frames = -1;
}

static void
visual_stimulus_update(PsyVisualStimulus* self, PsyTimePoint* frame_time, gint64 nth_frame)
{
    PsyVisualStimulusPrivate* priv = psy_visual_stimulus_get_instance_private(self);
    (void) frame_time;
    priv->nth_frame = nth_frame;
}

static void
visual_stimulus_play(PsyStimulus* stimulus, PsyTimePoint* start_time)
{
    PsyVisualStimulus* vstim = PSY_VISUAL_STIMULUS(stimulus);

    PsyWindow* window = psy_visual_stimulus_get_window(vstim);

    psy_window_schedule_stimulus(window, PSY_VISUAL_STIMULUS(stimulus));

    PSY_STIMULUS_CLASS(psy_visual_stimulus_parent_class)->play(
            stimulus, start_time
            );
}


static void
psy_visual_stimulus_class_init(PsyVisualStimulusClass* klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    object_class->get_property = psy_visual_stimulus_get_property;
    object_class->set_property = psy_visual_stimulus_set_property;

    PsyStimulusClass* stimulus_class = PSY_STIMULUS_CLASS(klass);
    stimulus_class->play        = visual_stimulus_play;

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
     * #psy_visual_stimulus_update.
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
     * #psy_visual_stimulus_update.
     */
    visual_stimulus_properties[PROP_NTH_FRAME] = g_param_spec_int64(
            "nth-frame",
            "NthFrame",
            "The nth frame of this stimulus.",
            -1, G_MAXINT64, -1,
            G_PARAM_READABLE
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
     * The class handler updates the values of:
     *     #
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
 * @window: a `PsyWindow` to draw this stimulus on.
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
    priv->window = window;
}

/**
 * psy_visual_stimulus_update:
 * @self the #PsyVisualStimulus instance in need of an update for a comming frame.
 * @frame_time:(transfer none): The time at which the next frame when the new
 *      frame is to be presented.
 * @frame_time: the number of the frame, at each presentation the number of the frame
 *              should be incremented with 1.
 * @stability:private
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
psy_visual_stimulus_update (
        PsyVisualStimulus  *self,
        PsyTimePoint       *frame_time,
        gint64              nth_frame
        )
{
    g_return_if_fail(PSY_IS_STIMULUS(self));
    g_return_if_fail(PSY_IS_TIME_POINT(frame_time));

    g_signal_emit(
            self,
            SIG_UPDATE,
            0,
            frame_time,
            nth_frame
            );
}



