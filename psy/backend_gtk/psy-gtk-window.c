
#include <epoxy/gl.h>
#include <gtk/gtk.h>

#include "../gl/psy-gl-context.h"
#include "../gl/psy-gl-program.h"
#include "../gl/psy-gl-utilities.h"
#include "psy-artist.h"
#include "psy-circle.h"
#include "psy-clock.h"
#include "psy-drawing-context.h"
#include "psy-duration.h"
#include "psy-gtk-window.h"
#include "psy-program.h"
#include "psy-window.h"

/* forward declarations */
static void GLAPIENTRY
gl_debug_cb(GLenum        source,
            GLenum        type,
            guint         id,
            GLenum        severity,
            GLsizei       length,
            const GLchar *message,
            const void   *object);

static void
psy_gtk_window_set_last_frame_time(PsyGtkWindow *self,
                                   PsyTimePoint *frame_time);
static void
psy_gtk_window_compute_frame_stats(PsyGtkWindow *self,
                                   PsyTimePoint *frame_time);

struct _PsyGtkWindow {
    PsyWindow  parent;
    GtkWidget *window;
    GtkWidget *darea;
    bool       enable_debug;

    gint frames_lapsed; // number of frames lapsed since the last frame

    PsyTimePoint *frame_time;
};

G_DEFINE_TYPE_WITH_CODE(PsyGtkWindow, psy_gtk_window, PSY_TYPE_WINDOW, {
    // Initialize GTK when creating the first Gtk window
    gtk_init();
})

typedef enum GtkWindowProperty {
    PROP_0,
    PROP_ENABLE_DEBUG,
    NUM_PROPS, // number of properties, keep this one last.
} GtkWindowProperty;

typedef enum GtkWindowSignals {
    SIG_DEBUG_MESSAGE,
    NUM_SIGNALS
} GtkWindowSingals;

static GParamSpec *gtk_window_props[NUM_PROPS]     = {NULL};
static guint       gtk_window_signals[NUM_SIGNALS] = {0};

static void
psy_gtk_window_set_property(GObject      *object,
                            guint         property_id,
                            const GValue *value,
                            GParamSpec   *pspec)
{
    PsyGtkWindow *self = PSY_GTK_WINDOW(object);

    switch ((GtkWindowProperty) property_id) {
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
psy_gtk_window_get_property(GObject    *object,
                            guint       property_id,
                            GValue     *value,
                            GParamSpec *pspec)
{
    PsyGtkWindow *self = PSY_GTK_WINDOW(object);

    switch ((GtkWindowProperty) property_id) {
    case PROP_ENABLE_DEBUG:
        g_value_set_boolean(value, self->enable_debug);
        break;
    default:
        /* We don't have any other property... */
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static gboolean
tick_callback(GtkWidget *d_area, GdkFrameClock *clock, gpointer data)
{
    PsyGtkWindow *window = PSY_GTK_WINDOW(data);
    GtkGLArea    *canvas = GTK_GL_AREA(d_area);
    gtk_gl_area_make_current(canvas);

    // Check if there is anything to draw to
    if (glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER)
        != GL_FRAMEBUFFER_COMPLETE) {
        return G_SOURCE_CONTINUE;
    }

    // gtk_gl_area_attach_buffers(canvas); // yields OpenGL error within Gtk
    GError *error = gtk_gl_area_get_error(GTK_GL_AREA(canvas));
    if (error) {
        g_critical("An OpenGL error occurred: %s", error->message);
        return G_SOURCE_REMOVE;
    }

    PsyCanvasClass *canvas_class = PSY_CANVAS_GET_CLASS(window);

    GdkFrameTimings *timings = gdk_frame_clock_get_current_timings(clock);
    gint64           predicted
        = gdk_frame_timings_get_predicted_presentation_time(timings);
    gint64 frame_count = gdk_frame_timings_get_frame_counter(timings);

    PsyTimePoint *tp = psy_time_point_new_monotonic(predicted);

    canvas_class->draw(PSY_CANVAS(window), frame_count, tp);

    psy_gtk_window_compute_frame_stats(window, tp);
    psy_gtk_window_set_last_frame_time(window, tp);

    // Queues a new frame. Otherwise the frame clock doesn't update
    gtk_widget_queue_draw(GTK_WIDGET(canvas));

    return G_SOURCE_CONTINUE;
}

static void
on_canvas_resize(GtkDrawingArea *darea, gint width, gint height, gpointer data)
{
    PsyGtkWindow *self = data;
    gtk_gl_area_make_current(GTK_GL_AREA(darea));

    g_signal_emit_by_name(
        self, "resize", width, height); // allow clients to update

    glViewport(0,
               0,
               width > 0 ? (GLsizei) width : 0,
               height > 0 ? (GLsizei) height : 0);
}

static void
create_drawing_context(PsyGtkWindow *window)
{
    PsyGlContext *context = psy_gl_context_new();
    psy_canvas_set_context(PSY_CANVAS(window), PSY_DRAWING_CONTEXT(context));
}

static void
init_shaders(PsyGtkWindow *self, GError **error)
{
    // Uniform color program
    PsyDrawingContext *context = psy_canvas_get_context(PSY_CANVAS(self));

    PsyProgram *program = psy_drawing_context_create_program(context);

    psy_program_set_vertex_shader_from_path(
        program, "./psy/uniform-color.vert", error);
    if (*error)
        goto fail;

    psy_program_set_fragment_shader_from_path(
        program, "./psy/uniform-color.frag", error);
    if (*error)
        goto fail;

    psy_program_link(program, error);
    if (*error)
        goto fail;

    psy_drawing_context_register_program(
        context, PSY_UNIFORM_COLOR_PROGRAM_NAME, program, error);
    if (*error)
        return;

    g_clear_object(&program);

    // Picture program
    program = psy_drawing_context_create_program(context);

    psy_program_set_vertex_shader_from_path(
        program, "./psy/picture.vert", error);
    if (*error)
        goto fail;

    psy_program_set_fragment_shader_from_path(
        program, "./psy/picture.frag", error);
    if (*error)
        goto fail;

    psy_program_link(program, error);
    if (*error)
        goto fail;

    psy_drawing_context_register_program(
        context, PSY_PICTURE_PROGRAM_NAME, program, error);

    g_clear_object(&program);
    return;

fail:
    g_object_unref(program);
}

static void
on_canvas_realize(GtkGLArea *canvas, PsyGtkWindow *window)
{
    GError *error = NULL;
    gtk_gl_area_make_current(canvas);

    if (gtk_gl_area_get_error(canvas) != NULL)
        return;

    if (window->enable_debug) {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(&gl_debug_cb, window);
        glDebugMessageControl(
            GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
    }

    init_shaders(window, &error);

    if (error) {
        g_critical("Unable to init shaders %s\n", error->message);
        g_error_free(error);
        return;
    }

    gtk_widget_add_tick_callback(
        GTK_WIDGET(canvas), tick_callback, window, NULL);
}

static void
on_canvas_unrealize(GtkGLArea *area, PsyGtkWindow *self)
{
    gtk_gl_area_make_current(area);
    PsyDrawingContext *context = psy_canvas_get_context(PSY_CANVAS(self));
    psy_drawing_context_free_resources(context);
}

static void GLAPIENTRY
gl_debug_cb(GLenum        source,
            GLenum        type,
            guint         id,
            GLenum        severity,
            GLsizei       length,
            const GLchar *message,
            const void   *object)
{
    const char *source_str   = NULL;
    const char *type_str     = NULL;
    const char *severity_str = NULL;
    (void) length;

    switch (source) {
    case GL_DEBUG_SOURCE_API:
        source_str = "API";
        break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
        source_str = "WINDOW_SYSTEM";
        break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
        source_str = "SHADER_COMPILER";
        break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:
        source_str = "THIRD_PARTY";
        break;
    case GL_DEBUG_SOURCE_APPLICATION:
        source_str = "APPLICATION";
        break;
    case GL_DEBUG_SOURCE_OTHER:
        source_str = "OTHER";
        break;
    default:
        source_str = "unexpected <file bug> with the value of source";
    }

    switch (type) {
    case GL_DEBUG_TYPE_ERROR:
        type_str = "ERROR";
        break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        type_str = "DEPRECATED_BEHAVIOR";
        break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        type_str = "UNDEFINED_BEHAVIOR";
        break;
    case GL_DEBUG_TYPE_PORTABILITY:
        type_str = "PORTABILITY";
        break;
    case GL_DEBUG_TYPE_PERFORMANCE:
        type_str = "PERFORMANCE";
        break;
    case GL_DEBUG_TYPE_MARKER:
        type_str = "MARKER";
        break;
    case GL_DEBUG_TYPE_PUSH_GROUP:
        type_str = "PUSH_GROUP";
        break;
    case GL_DEBUG_TYPE_POP_GROUP:
        type_str = "POP_GROUP";
        break;
    case GL_DEBUG_TYPE_OTHER:
        type_str = "OTHER";
        break;
    default:
        type_str = "unexpected <file bug> with the value of type";
    }

    switch (severity) {
    case GL_DEBUG_SEVERITY_HIGH:
        severity_str = "HIGH";
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        severity_str = "MEDIUM";
        break;
    case GL_DEBUG_SEVERITY_LOW:
        severity_str = "LOW";
        break;
    case GL_DEBUG_SEVERITY_NOTIFICATION:
        severity_str = "NOTIFICATION";
        break;
    default:
        type_str = "unexpected <file bug> with the value of severity";
    }

    // Cast const away
    PsyGtkWindow *self = (gpointer) object;

    g_signal_emit(self,
                  gtk_window_signals[SIG_DEBUG_MESSAGE],
                  0,
                  source,
                  type,
                  id,
                  severity,
                  message,
                  source_str,
                  type_str,
                  severity_str,
                  NULL);
}

static void
psy_canvas_create_context_cb(GtkGLArea *darea, gpointer user_data)
{
    GtkGLArea    *canvas = darea;
    PsyGtkWindow *self   = PSY_GTK_WINDOW(user_data);
    // Prepare common OpenGL setup prior to realization.
    gtk_gl_area_set_required_version(GTK_GL_AREA(canvas), 4, 4);
    // gtk_gl_area_set_has_alpha(GTK_GL_AREA(self->darea), TRUE);
    gtk_gl_area_set_has_depth_buffer(GTK_GL_AREA(canvas), TRUE);

    // Setup debugging stuff
    if (self->enable_debug) {
        g_assert(GTK_IS_GL_AREA(canvas));
        GdkGLContext *gdk_context
            = gtk_gl_area_get_context(GTK_GL_AREA(canvas));
        g_assert(gdk_context != NULL);
        g_assert(GDK_IS_GL_CONTEXT(gdk_context));
        gdk_gl_context_set_debug_enabled(gdk_context, TRUE);
    }
    gtk_gl_area_set_has_stencil_buffer(GTK_GL_AREA(canvas), FALSE);
}

static void
psy_gtk_window_constructed(GObject *obj)
{
    PsyGtkWindow *self = PSY_GTK_WINDOW(obj);

    GtkWidget *canvas = gtk_gl_area_new();
    self->darea       = canvas;

    g_signal_connect_after(canvas,
                           "create-context",
                           G_CALLBACK(psy_canvas_create_context_cb),
                           self);
    g_signal_connect(canvas, "realize", G_CALLBACK(on_canvas_realize), self);
    g_signal_connect(
        canvas, "unrealize", G_CALLBACK(on_canvas_unrealize), self);
    g_signal_connect(canvas, "resize", G_CALLBACK(on_canvas_resize), self);

    gtk_window_set_child(GTK_WINDOW(self->window), canvas);

    G_OBJECT_CLASS(psy_gtk_window_parent_class)->constructed(obj);
}

static void
psy_gtk_window_init(PsyGtkWindow *self)
{
    // Setup a window with a OpenGL canvas as child.
    self->window = gtk_window_new();

    gtk_window_set_decorated(GTK_WINDOW(self->window), FALSE);

    create_drawing_context(self);

    gtk_widget_show(GTK_WIDGET(self->window));
}

static void
psy_gtk_window_dispose(GObject *gobject)
{
    PsyGtkWindow *self = PSY_GTK_WINDOW(gobject);

    g_clear_object(&self->window);

    G_OBJECT_CLASS(psy_gtk_window_parent_class)->dispose(gobject);
}

static void
psy_gtk_window_finalize(GObject *gobject)
{
    PsyGtkWindow *self = PSY_GTK_WINDOW(gobject);

    g_clear_pointer(&self->frame_time, psy_time_point_free);

    G_OBJECT_CLASS(psy_gtk_window_parent_class)->finalize(gobject);
}

static void
set_monitor(PsyWindow *self, gint nth_monitor)
{
    gint width_mm, height_mm;

    g_return_if_fail(PSY_IS_GTK_WINDOW(self));
    PsyGtkWindow *psywindow = PSY_GTK_WINDOW(self);

    GdkDisplay *display = gdk_display_get_default();
    guint       num_monitors
        = g_list_model_get_n_items(gdk_display_get_monitors(display));
    g_return_if_fail(nth_monitor >= 0);
    g_return_if_fail((guint) nth_monitor < num_monitors);

    GListModel *monitors = gdk_display_get_monitors(display);
    GdkMonitor *monitor  = g_list_model_get_item(monitors, (guint) nth_monitor);

    width_mm         = gdk_monitor_get_width_mm(monitor);
    height_mm        = gdk_monitor_get_height_mm(monitor);
    gdouble      mHz = gdk_monitor_get_refresh_rate(monitor); // milli Hertz
    gdouble      Hz  = mHz / 1000;
    PsyDuration *frame_duration = psy_duration_new(1 / Hz);

    gtk_window_fullscreen_on_monitor(GTK_WINDOW(psywindow->window), monitor);
    psy_canvas_set_width_mm(PSY_CANVAS(self), width_mm);
    psy_canvas_set_height_mm(PSY_CANVAS(self), height_mm);

    PSY_CANVAS_CLASS(psy_gtk_window_parent_class)
        ->set_frame_dur(PSY_CANVAS(self), frame_duration);
    psy_duration_free(frame_duration);

    PSY_WINDOW_CLASS(psy_gtk_window_parent_class)
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

    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

static void
draw_stimuli(PsyCanvas *self, guint64 nth_frame, PsyTimePoint *tp)
{
    PSY_CANVAS_CLASS(psy_gtk_window_parent_class)
        ->draw_stimuli(self, nth_frame, tp);
}

static void
update_frame_stats(PsyCanvas *canvas, PsyFrameCount *stats)
{
    PsyGtkWindow *self = PSY_GTK_WINDOW(canvas);

    stats->missed_frames += (self->frames_lapsed - 1);
    stats->tot_frames += (self->frames_lapsed);
    stats->num_frames++;

    self->frames_lapsed = 0;
}

static void
upload_projection_matrices(PsyCanvas *self)
{
    // In psy-gl-utilities as PsyGlCanvas also needs it.
    psy_gl_canvas_upload_projection_matrices(self);
}

static void
psy_gtk_window_class_init(PsyGtkWindowClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    object_class->constructed  = psy_gtk_window_constructed;
    object_class->dispose      = psy_gtk_window_dispose;
    object_class->finalize     = psy_gtk_window_finalize;
    object_class->get_property = psy_gtk_window_get_property;
    object_class->set_property = psy_gtk_window_set_property;

    PsyWindowClass *psy_window_class = PSY_WINDOW_CLASS(klass);
    psy_window_class->set_monitor    = set_monitor;

    PsyCanvasClass *psy_canvas_class             = PSY_CANVAS_CLASS(klass);
    psy_canvas_class->clear                      = clear;
    psy_canvas_class->draw_stimuli               = draw_stimuli;
    psy_canvas_class->update_frame_stats         = update_frame_stats;
    psy_canvas_class->upload_projection_matrices = upload_projection_matrices;

    /**
     * PsyGtkWindow:enable-debug:
     *
     * This boolean may be set when constructing the window. This enables
     * some extra debugging features at the expense of runtime performance.
     *
     * It may be handy, to put a breakpoint in a debugger at gl_debug_cb in
     * this file. Additionally, the SIG_DEBUG signal will be emitted when
     * something is happening, so than one can get some extra information
     * about the error that occurs.
     */
    gtk_window_props[PROP_ENABLE_DEBUG] = g_param_spec_boolean(
        "enable-debug",
        "EnableDebug",
        "optionally obtain extra debugging info from OpenGL calls.",
        FALSE,
        G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

    g_object_class_install_properties(
        object_class, NUM_PROPS, gtk_window_props);

    /**
     * PsyGtkWindow::debug-message:
     * @self: An instance of `PsyGtkWindow`
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
    gtk_window_signals[SIG_DEBUG_MESSAGE]
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
 * psy_gtk_window_new:(constructor):
 *
 * Returns a new #PsyGtkWindow instance on the first monitor.
 *
 * Returns: a newly initialized window it may be freed with psy_gtk_window_free
 * or g_object_unref
 */
PsyGtkWindow *
psy_gtk_window_new(void)
{
    PsyGtkWindow *window = psy_gtk_window_new_for_monitor(0);
    return window;
}

/**
 * psy_gtk_window_new_for_montitor:(constructor):
 * @n: the number of the monitor on which you want to display
 *     the newly created window. n will be clipped to the available
 *     range which is [0, n) where n is the number of monitors connected.
 *
 * Creates a new window, it should appear on the window that is specified
 * by @n. If a number larger than n is chosen it will appear on n minus one
 * where n is the number of connected monitors.
 *
 * Returns: a newly initialized window it may be freed with psy_gtk_window_free
 * or g_object_unref
 */
PsyGtkWindow *
psy_gtk_window_new_for_monitor(gint n)
{
    // First create a window as this initializes gtk when it hasn't been
    // initialized before.
    PsyGtkWindow *window
        = g_object_new(PSY_TYPE_GTK_WINDOW, "n-monitor", n, NULL);

    return window;
}

/**
 * psy_gtk_window_set_last_frame_time:
 * @self: An instance of [class@PsyGtkWindow]
 * @frame_time:(transfer full): The time of the current frame.
 *
 * Stores the frame time of the current frame. This may be used by next
 * iteration of the tick call back to determine whether frames are being missed.
 */
static void
psy_gtk_window_set_last_frame_time(PsyGtkWindow *self, PsyTimePoint *frame_time)
{
    psy_time_point_free(self->frame_time);
    self->frame_time = frame_time;
}

/**
 * psy_gtk_window_compute_frame_stats:
 * @self: An instance of [class@PsyGtkWindow]
 * @frame_time:(transfer none): The time of the current frame.
 *
 * This clears and sets the internal statistics.
 */
static void
psy_gtk_window_compute_frame_stats(PsyGtkWindow *self, PsyTimePoint *tp_new)
{
    if (self->frame_time) { // there was a previous frame
        PsyDuration *time_lapsed
            = psy_time_point_subtract(tp_new, self->frame_time);
        PsyDuration *frame_dur = psy_canvas_get_frame_dur(PSY_CANVAS(self));

        gint64 num_frames = psy_duration_divide_rounded(time_lapsed, frame_dur);
        self->frames_lapsed = num_frames;

        psy_duration_free(time_lapsed);
    }
    else {
        self->frames_lapsed = 1;
    }
}

/**
 * psy_gtk_window_free:(skip)
 *
 * Destroys a psy window created with psy_gtk_window*new family of functions
 */
void
psy_gtk_window_free(PsyGtkWindow *self)
{
    g_return_if_fail(PSY_IS_GTK_WINDOW(self));
    g_object_unref(self);
}
