
#include "psy-windows.h"

#include "psy-clock.h"
#include "psy-display-info.h"
#include "psy-drawing-context.h"
#include "psy-duration.h"
#include "psy-utils.h"
#include "psy-win-window.h"
#include "psy-window.h"

#include "psy-win-window-private.h"

static LRESULT
psy_win_window_window_proc(
    PsyWinWindow *self, HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

static const char *g_win_class_name = "PsyWinWindow";
static WNDCLASSEX *g_win_class      = NULL;
static ATOM        g_win_class_atom = 0;
HINSTANCE          g_module_handle  = NULL;

static int    g_init_count = 0;
static GMutex g_mutex;

static LRESULT CALLBACK
connect_winproc_to_self(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
    LONG_PTR      ptr  = GetWindowLongPtr(hwnd, GWLP_USERDATA);
    PsyWinWindow *self = PSY_WIN_WINDOW((gpointer) ptr);

    return psy_win_window_window_proc(self, hwnd, message, wparam, lparam);
}

static LRESULT CALLBACK
startup_winproc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
    if (message == WM_NCCREATE) {
        const CREATESTRUCT *cs = (const CREATESTRUCT *) lparam;

        PsyWinWindow *self = PSY_WIN_WINDOW(cs->lpCreateParams);
        // setup
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR) self);
        SetWindowLongPtr(
            hwnd, GWLP_WNDPROC, (LONG_PTR) &connect_winproc_to_self);

        return psy_win_window_window_proc(self, hwnd, message, wparam, lparam);
    }

    return DefWindowProc(hwnd, message, wparam, lparam);
}

static void
create_window_class(void)
{
    g_module_handle = GetModuleHandle(NULL);

    WNDCLASSEX win_class = {
        .cbSize        = sizeof(win_class),
        .style         = CS_OWNDC,
        .lpfnWndProc   = startup_winproc,
        .hCursor       = LoadCursor(NULL, IDC_ARROW),
        .lpszClassName = g_win_class_name,
    };

    g_win_class  = g_malloc0(sizeof(WNDCLASSEX));
    *g_win_class = win_class;

    g_win_class_atom = RegisterClassEx(g_win_class);
    if (g_win_class_atom == 0) {
        char error_buf[1024];
        psy_strerr(GetLastError(), error_buf, sizeof(error_buf));
        g_critical("Unable to create WindowClass '%s': %s",
                   g_win_class_name,
                   error_buf);
    }
}

static void
destroy_window_class(void)
{
    UnregisterClass(g_win_class_name, NULL);
    g_free(g_win_class);
    g_win_class      = NULL;
    g_win_class_atom = 0;
}

struct _PsyWinWindow {
    PsyWindow parent;
    HWND      window;
    gboolean  is_running;
    gint      frames_lapsed; // number of frames lapsed since the last frame
    gboolean  enable_debug;  // enables extra debugging on the Direct3D context

    PsyClock     *clock;
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
win_window_set_property(GObject      *object,
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
win_window_get_property(GObject    *object,
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
    self->is_running = TRUE;

    g_mutex_lock(&g_mutex);
    if (++g_init_count == 1) {
        create_window_class();
    }
    g_assert(g_init_count > 0);
    g_mutex_unlock(&g_mutex);

    self->clock = psy_clock_new();
}

static void
win_window_constructed(GObject *object)
{
    PsyWinWindow *self = PSY_WIN_WINDOW(object);

    DWORD win_style    = WS_OVERLAPPEDWINDOW | WS_SIZEBOX;
    DWORD win_style_ex = 0;

    int def_width  = 640;
    int def_height = 480;
    int x = 100, y = 100;

    RECT r = {
        .bottom = y + def_height,
        .top    = y,
        .right  = x + def_width,
        .left   = x,
    };
    AdjustWindowRectEx(&r, win_style, FALSE, win_style_ex);

    self->window = CreateWindowEx(0,
                                  g_win_class_name,
                                  "psy_window",
                                  win_style,
                                  CW_USEDEFAULT,
                                  CW_USEDEFAULT,
                                  r.right - r.left,
                                  r.bottom - r.top,
                                  NULL,
                                  NULL,
                                  g_module_handle,
                                  self);
    if (!self->window) {
        char error_buff[1024];
        psy_strerr(GetLastError(), error_buff, sizeof(error_buff));
        g_critical("Unable to create window: %s", error_buff);
        return;
    }

    BOOL success = ShowWindow(self->window, SW_SHOWNORMAL | SW_SHOW);
    if (!success) {
        char error[1024];
        psy_strerr(GetLastError(), error, sizeof(error));
        g_critical("Unable to show window: %s", error);
    }
    success = SetForegroundWindow(self->window);
    if (!success) {
        char error[1024];
        psy_strerr(GetLastError(), error, sizeof(error));
        g_critical("Unable to set window to foreground: %s", error);
    }

    success
        = SetWindowPos(self->window,
                       HWND_TOP,
                       0,
                       0,
                       0,
                       0,
                       SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW | SWP_NOREDRAW);
    if (!success) {
        char error_buf[1024];
        psy_strerr(GetLastError(), error_buf, sizeof(error_buf));
        g_critical("Unable to set window pos: %s", error_buf);
    }

    // create_drawing_context(self);
#warning "Create a Direct3D context here";
    G_OBJECT_CLASS(psy_win_window_parent_class)->constructed(object);
}

static void
win_window_dispose(GObject *gobject)
{
    PsyWinWindow *self = PSY_WIN_WINDOW(gobject);

    g_clear_pointer(&self->window, DestroyWindow);

    g_mutex_lock(&g_mutex);
    if (--g_init_count == 0) {
        destroy_window_class();
    }
    g_mutex_unlock(&g_mutex);

    g_clear_object(&self->clock);

    G_OBJECT_CLASS(psy_win_window_parent_class)->dispose(gobject);
}

static void
win_window_finalize(GObject *gobject)
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

    GPtrArray *monitor_infos = psy_win_window_enumerate_displays();
    if (nth_monitor < 0)
        nth_monitor = 0;
    else if (nth_monitor >= monitor_infos->len)
        nth_monitor = monitor_infos->len - 1;

    if (nth_monitor < 0)
        g_critical("No monitors found");

    PsyDisplayInfo *info = monitor_infos->pdata[nth_monitor];

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
win_window_resize(PsyCanvas *canvas, gint width, gint height)
{
    // Make sure the direct 3d resource buffers are resized.
    g_print("%s: width = %d, height = %d\n", __func__, width, height);

    PSY_CANVAS_CLASS(psy_win_window_parent_class)
        ->resize(canvas, width, height);
}

static LRESULT
psy_win_window_window_proc(
    PsyWinWindow *self, HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
    LONG          msg_time_stamp = GetMessageTime();
    gint64        time_stamp_us  = ((gint64) msg_time_stamp) * 1000;
    PsyTimePoint *msg_time       = psy_time_point_new_monotonic(time_stamp_us);

    switch (message) {
    case WM_SIZE:
    {
        gint width  = LOWORD(lparam);
        gint height = HIWORD(lparam);
        psy_canvas_resize(PSY_CANVAS(self), width, height);
        break;
    };
    case WM_CLOSE:
        g_debug("%s", "WM_CLOSE");
        DestroyWindow(self->window);
        break;
    case WM_DESTROY:
        g_debug("%s", "WM_DESTROY");
        PostQuitMessage(0);
        break;
    case WM_CREATE:
        g_debug("%s", "WM_CREATE");
        break;
    case WM_SHOWWINDOW:
    {
        g_debug("%s", "WM_SHOW");
        // move to the top
        break;
    }
    default:
        return DefWindowProc(hwnd, message, wparam, lparam);
    };

    psy_time_point_free(msg_time);
    return 0;
}

static void
psy_win_window_class_init(PsyWinWindowClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    object_class->constructed  = win_window_constructed;
    object_class->dispose      = win_window_dispose;
    object_class->finalize     = win_window_finalize;
    object_class->get_property = win_window_get_property;
    object_class->set_property = win_window_set_property;

    PsyWindowClass *psy_window_class = PSY_WINDOW_CLASS(klass);
    psy_window_class->set_monitor    = set_monitor;

    PsyCanvasClass *psy_canvas_class             = PSY_CANVAS_CLASS(klass);
    psy_canvas_class->clear                      = clear;
    psy_canvas_class->draw_stimuli               = draw_stimuli;
    psy_canvas_class->update_frame_stats         = update_frame_stats;
    psy_canvas_class->upload_projection_matrices = upload_projection_matrices;
    psy_canvas_class->resize                     = win_window_resize;

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

void
psy_win_window_start_message_loop(PsyWinWindow *self)
{
    g_return_if_fail(PSY_IS_WIN_WINDOW(self));

    MSG msg = {0};

    while (self->is_running) {

        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) > 0) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT) {
                self->is_running = FALSE;
                break;
            }
        }

        Sleep(1); // remove for drawing stuff
    }
}