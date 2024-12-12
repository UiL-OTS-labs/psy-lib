
#include "psy-win-window.h"
#include "psy-artist.h"
#include "psy-circle.h"
#include "psy-clock.h"
#include "psy-drawing-context.h"
#include "psy-duration.h"
#include "psy-window.h"

#include "psy-windows.h"

struct _PsyWinWindow {
    PsyWindow parent;
    HWND      window;
    gchar    *name;
    gint      frames_lapsed; // number of frames lapsed since the last frame
    gboolean  enable_debug;  // enables extra debugging on the Direct3D context

    PsyTimePoint *frame_time;
};

G_DEFINE_TYPE(PsyWinWindow, psy_win_window, PSY_TYPE_WINDOW)

typedef enum WinWindowProperty {
    PROP_0,
    PROP_ENABLE_DEBUG,
    NUM_PROPS, // number of properties, keep this one last.
} WinWindowProperty;

typedef enum WinWindowSignals {
    SIG_DEBUG_MESSAGE,
    NUM_SIGNALS
} WinWindowSingals;

static GParamSpec *win_window_props[NUM_PROPS]     = {NULL};
static guint       win_window_signals[NUM_SIGNALS] = {0};

static void
psy_win_window_set_property(GObject      *object,
                            guint         property_id,
                            const GValue *value,
                            GParamSpec   *pspec)
{
    PsyWinWindow *self = PSY_WIN_WINDOW(object);

    switch ((WinWindowProperty) property_id) {
    case PROP_ENABLE_DEBUG:
        self->enable_debug = g_value_get_boolean(value);
        break;
    default:
        /* We don't have any other property... */
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static void
psy_win_window_get_property(GObject    *object,
                            guint       property_id,
                            GValue     *value,
                            GParamSpec *pspec)
{
    PsyWinWindow *self = PSY_WIN_WINDOW(object);

    switch ((WinWindowProperty) property_id) {
    case PROP_ENABLE_DEBUG:
        g_value_set_boolean(value, self->enable_debug);
        break;
    default:
        /* We don't have any other property... */
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static void
psy_win_window_init(PsyWinWindow *self)
{
// create_drawing_context(self);

// or in constructed.
#warning "Create a Direct3D context here";
}

static void
psy_win_window_dispose(GObject *gobject)
{
    PsyWinWindow *self = PSY_WIN_WINDOW(gobject);

    g_clear_pointer(&self->window, DestroyWindow);

    G_OBJECT_CLASS(psy_win_window_parent_class)->dispose(gobject);
}

static void
psy_win_window_finalize(GObject *gobject)
{
    PsyWinWindow *self = PSY_WIN_WINDOW(gobject);

    g_clear_pointer(&self->frame_time, psy_time_point_free);

    G_OBJECT_CLASS(psy_win_window_parent_class)->finalize(gobject);
}

static void
set_monitor(PsyWindow *self, gint nth_monitor)
{
    gint width_mm, height_mm;

    g_return_if_fail(PSY_IS_WIN_WINDOW(self));
    PsyWinWindow *psywindow = PSY_WIN_WINDOW(self);

    // Tell the parent canvas class about the changed parameters
    PsyDuration *frame_duration = NULL;

    PSY_CANVAS_CLASS(psy_win_window_parent_class)
        ->set_frame_dur(PSY_CANVAS(self), frame_duration);
    psy_duration_free(frame_duration);

    PSY_WINDOW_CLASS(psy_win_window_parent_class)
        ->set_monitor(PSY_WINDOW(self), nth_monitor);
}

static void
clear(PsyCanvas *self)
{
    gfloat    r, b, g, a;
    PsyColor *color = psy_canvas_get_background_color(self);

    // clang-format off
    g_object_get(color,
                 "r", &r,
                 "g", &b,
                 "b", &g,
                 "a", &a,
                 NULL);
    // clang-format on

#warning "clear background here";
}

static void
draw_stimuli(PsyCanvas *self, guint64 nth_frame, PsyTimePoint *tp)
{
    PSY_CANVAS_CLASS(psy_win_window_parent_class)
        ->draw_stimuli(self, nth_frame, tp);
}

static void
update_frame_stats(PsyCanvas *canvas, PsyFrameCount *stats)
{
    PsyWinWindow *self = PSY_WIN_WINDOW(canvas);

    stats->missed_frames += (self->frames_lapsed - 1);
    stats->tot_frames += (self->frames_lapsed);
    stats->num_frames++;

    self->frames_lapsed = 0;
}

static void
upload_projection_matrices(PsyCanvas *self)
{
    // In psy-gl-utilities as PsyGlCanvas also needs it.
    // psy_gl_canvas_upload_projection_matrices(self);
#warning "implement this properly using Direct3D"
}

static void
psy_win_window_class_init(PsyWinWindowClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    // object_class->constructed  = psy_win_window_constructed;
    object_class->dispose      = psy_win_window_dispose;
    object_class->finalize     = psy_win_window_finalize;
    object_class->get_property = psy_win_window_get_property;
    object_class->set_property = psy_win_window_set_property;

    PsyWindowClass *psy_window_class = PSY_WINDOW_CLASS(klass);
    psy_window_class->set_monitor    = set_monitor;

    PsyCanvasClass *psy_canvas_class             = PSY_CANVAS_CLASS(klass);
    psy_canvas_class->clear                      = clear;
    psy_canvas_class->draw_stimuli               = draw_stimuli;
    psy_canvas_class->update_frame_stats         = update_frame_stats;
    psy_canvas_class->upload_projection_matrices = upload_projection_matrices;

    /**
     * PsyWinWindow:enable-debug:
     *
     * This boolean may be set when constructing the window. This enables
     * some extra debugging features at the expense of runtime performance.
     *
     * It may be handy, to put a breakpoint in a debugger at gl_debug_cb in
     * this file. Additionally, the SIG_DEBUG signal will be emitted when
     * something is happening, so than one can get some extra information
     * about the error that occurs.
     */
    win_window_props[PROP_ENABLE_DEBUG] = g_param_spec_boolean(
        "enable-debug",
        "EnableDebug",
        "optionally obtain extra debugging info from OpenGL calls.",
        FALSE,
        G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

    g_object_class_install_properties(
        object_class, NUM_PROPS, win_window_props);

    /**
     * PsyWinWindow::debug-message:
     * @self: An instance of `PsyWinWindow`
     * @source: A GLenum that specifies the source of the error
     * @type: A GLenum that specifies the type of the error
     * @id: The OpenGL id of the object where an error occurred.
     * @severity: The severity of the error
     * @message:the error message in string format
     * @source_str: The string version of @source
     * @type_str: The string version of @type
     * @severity_str: The string version of @severity
     * @data: a user specified pointer to data when the signal was connected
     *
     * This signal is emitted when the window is created with the "enable-debug"
     * property set to true. This signal is raised when the OpenGL debugging
     * context encounters something weird. It is mostly useful for debugging
     * errors related to opengl.
     */
    win_window_signals[SIG_DEBUG_MESSAGE]
        = g_signal_new("debug-message",
                       G_TYPE_FROM_CLASS(klass),
                       G_SIGNAL_RUN_FIRST | G_SIGNAL_NO_RECURSE,
                       0,
                       NULL,
                       NULL,
                       NULL,
                       G_TYPE_NONE,
                       8,
                       G_TYPE_UINT,
                       G_TYPE_UINT,
                       G_TYPE_UINT,
                       G_TYPE_UINT,
                       G_TYPE_STRING,
                       G_TYPE_STRING,
                       G_TYPE_STRING,
                       G_TYPE_STRING);
}

/**
 * psy_win_window_new:(constructor):
 *
 * Returns a new #PsyWinWindow instance on the first monitor.
 *
 * Returns: a newly initialized window it may be freed with psy_win_window_free
 * or g_object_unref
 */
PsyWinWindow *
psy_win_window_new(void)
{
    PsyWinWindow *window = psy_win_window_new_for_monitor(0);
    return window;
}

/**
 * psy_win_window_new_for_montitor:(constructor):
 * @n: the number of the monitor on which you want to display
 *     the newly created window. n will be clipped to the available
 *     range which is [0, n) where n is the number of monitors connected.
 *
 * Creates a new window, it should appear on the window that is specified
 * by @n. If a number larger than n is chosen it will appear on n minus one
 * where n is the number of connected monitors.
 *
 * Returns: a newly initialized window it may be freed with psy_win_window_free
 * or g_object_unref
 */
PsyWinWindow *
psy_win_window_new_for_monitor(gint n)
{
    // First create a window as this initializes gtk when it hasn't been
    // initialized before.
    PsyWinWindow *window
        = g_object_new(PSY_TYPE_WIN_WINDOW, "n-monitor", n, NULL);

    return window;
}

/**
 * psy_win_window_set_last_frame_time:
 * @self: An instance of [class@PsyWinWindow]
 * @frame_time:(transfer full): The time of the current frame.
 *
 * Stores the frame time of the current frame. This may be used by next
 * iteration of the tick call back to determine whether frames are being missed.
 */
static void
psy_win_window_set_last_frame_time(PsyWinWindow *self, PsyTimePoint *frame_time)
{
    psy_time_point_free(self->frame_time);
    self->frame_time = frame_time;
}

// /**
//  * psy_win_window_compute_frame_stats:
//  * @self: An instance of [class@PsyWinWindow]
//  * @frame_time:(transfer none): The time of the current frame.
//  *
//  * This clears and sets the internal statistics.
//  */
// static void
// psy_win_window_compute_frame_stats(PsyWinWindow *self, PsyTimePoint *tp_new)
// {
//     if (self->frame_time) { // there was a previous frame
//         PsyDuration *time_lapsed
//             = psy_time_point_subtract(tp_new, self->frame_time);
//         PsyDuration *frame_dur = psy_canvas_get_frame_dur(PSY_CANVAS(self));
//
//         gint64 num_frames = psy_duration_divide_rounded(time_lapsed,
//         frame_dur); self->frames_lapsed = num_frames;
//
//         psy_duration_free(time_lapsed);
//     }
//     else {
//         self->frames_lapsed = 1;
//     }
// }

/**
 * psy_win_window_free:(skip)
 *
 * Destroys a psy window created with psy_win_window*new family of functions
 */
void
psy_win_window_free(PsyWinWindow *self)
{
    g_return_if_fail(PSY_IS_WIN_WINDOW(self));
    g_object_unref(self);
}
