
#include <gtk/gtk.h>
#include <epoxy/gl.h>

#include "psy-clock.h"
#include "psy-duration.h"
#include "psy-gtk-window.h"
#include "../gl/psy-gl-program.h"
#include "psy-window.h"

struct _PsyGtkWindow {
    PsyWindow       parent;
    GtkWidget      *window;
    GtkWidget      *darea;
    PsyProgram     *uniform_color_program;
    PsyProgram     *picture_program;
};

G_DEFINE_TYPE_WITH_CODE(
        PsyGtkWindow,
        psy_gtk_window,
        PSY_TYPE_WINDOW,
        {
            // Initialize GTK when creating the first Gtk window
            gtk_init();
        }
    )

typedef enum {
    // Add props here starting at 1
    N_PROPS = 1
} PsyGtkWindowProperty;

static gboolean
tick_callback(GtkWidget       *d_area,
              GdkFrameClock   *clock,
              gpointer         data)
{
    (void) clock;
    PsyGtkWindow* window = PSY_GTK_WINDOW(data);
    (void) window;
    GtkGLArea* canvas = GTK_GL_AREA(d_area);
    gtk_gl_area_make_current(canvas);
    GError* error = gtk_gl_area_get_error(GTK_GL_AREA(canvas));
    if (error) {
        g_critical("An OpenGL error occurred: %s", error->message);
        return G_SOURCE_REMOVE;
    }

    PsyWindowClass* window_class = PSY_WINDOW_GET_CLASS(window);

    PsyTimePoint* tp = psy_time_point_new(gdk_frame_clock_get_frame_time(clock));
    window_class->draw(PSY_WINDOW(window), tp);

    // Queues a new frame.
    gtk_widget_queue_draw(GTK_WIDGET(canvas));

    return G_SOURCE_CONTINUE;
}

static void
on_canvas_resize(GtkDrawingArea* darea, gint width, gint height, gpointer data)
{
    PsyGtkWindow *self = data;
    (void) self;
    gtk_gl_area_make_current(GTK_GL_AREA(darea));
    glViewport(
            0,
            0,
            width > 0 ? (GLsizei) width : 0,
            height > 0 ? (GLsizei) height : 0
            );
    g_print("width %d, height %d, data %p\n", width, height, data);
}

static void
init_shaders(PsyGtkWindow* self, GError **error)
{
    // Uniform color program
    PsyGlProgram *program = psy_gl_program_new();
    self->uniform_color_program = PSY_PROGRAM(program);

    psy_program_set_vertex_shader_from_path(
            self->uniform_color_program, "./psy/uniform-color.vert", error
            );
    if (*error)
        goto fail;
    psy_program_set_fragment_shader_from_path(
            self->uniform_color_program, "./psy/uniform-color.frag", error);
    if (*error)
        goto fail;

    psy_program_link(self->uniform_color_program, error);
    if (*error)
        goto fail;

    // Picture program
    program = psy_gl_program_new();
    self->picture_program = PSY_PROGRAM(program);

    psy_program_set_vertex_shader_from_path(
            self->picture_program, "./psy/picture.vert", error
            );
    if (*error)
        goto fail;

    psy_program_set_fragment_shader_from_path(
            self->picture_program, "./psy/picture.frag", error
            );
    if (*error)
        goto fail;

    psy_program_link(self->picture_program, error);
    if (*error)
        goto fail;

    return;

    fail:
    g_object_unref(program);
}

static void
on_canvas_realize(GtkGLArea* canvas, PsyGtkWindow* window)
{
    GError* error = NULL;
    gtk_gl_area_make_current(canvas);

    if (gtk_gl_area_get_error(canvas) != NULL)
        return;

    init_shaders(window, &error);

    if (error) {
        gtk_gl_area_set_error(canvas, error);
        return;
    }
    
    gtk_widget_add_tick_callback(
            GTK_WIDGET(canvas), tick_callback, window, NULL
            );
}

static void
on_canvas_unrealize(GtkGLArea* area, PsyGtkWindow* self)
{
    gtk_gl_area_make_current(area);

    g_clear_object(&self->uniform_color_program);
    g_clear_object(&self->picture_program);
}

// The Gtk backend doesn't have any properties the regular window doesn't have.
//
//static void
//psy_gtk_window_set_property(GObject        *object,
//                            guint           property_id,
//                            const GValue   *value,
//                            GParamSpec     *spec)
//{
//    PsyGtkWindow* self = PSY_GTK_WINDOW(object);
//
//    switch((PsyGtkWindowProperty) property_id) {
//        default:
//            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, spec);
//    }
//}
//
//static void
//psy_gtk_window_get_property(GObject        *object,
//                            guint           property_id,
//                            GValue         *value,
//                            GParamSpec     *spec)
//{
//    PsyGtkWindow* self = PSY_GTK_WINDOW(object);
//
//    switch((PsyGtkWindowProperty) property_id) {
//        default:
//            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, spec);
//    }
//}

static void
psy_gtk_window_init(PsyGtkWindow* self)
{
    self->window = gtk_window_new();
    GtkWidget* canvas = gtk_gl_area_new();
    gtk_window_set_child(GTK_WINDOW(self->window), canvas);

    // Prepare common OpenGL setup prior to realization.
    gtk_gl_area_set_required_version(GTK_GL_AREA(canvas), 4, 4);
    // gtk_gl_area_set_has_alpha(GTK_GL_AREA(self->darea), TRUE);
    gtk_gl_area_set_has_depth_buffer(GTK_GL_AREA(canvas), TRUE);
    gtk_gl_area_set_has_stencil_buffer(GTK_GL_AREA(canvas), FALSE);

    gtk_window_set_decorated(GTK_WINDOW(self->window), FALSE);

    g_signal_connect(
            canvas,
            "realize",
            G_CALLBACK(on_canvas_realize),
            self);
    g_signal_connect(
            canvas,
            "unrealize",
            G_CALLBACK(on_canvas_unrealize),
            self);

    g_signal_connect(
            canvas,
            "resize",
            G_CALLBACK(on_canvas_resize),
            self
            );

    gtk_widget_show(GTK_WIDGET(self->window));
}

static void
psy_gtk_window_dispose(GObject* gobject)
{
    PsyGtkWindow* self = PSY_GTK_WINDOW(gobject);
    g_clear_object(&self->window);
    g_clear_object(&self->uniform_color_program);
    g_clear_object(&self->picture_program);

    G_OBJECT_CLASS(psy_gtk_window_parent_class)->dispose(gobject);
}

static void
psy_gtk_window_finalize(GObject* gobject)
{
    PsyGtkWindow* self = PSY_GTK_WINDOW(gobject);
    (void) self;

    G_OBJECT_CLASS(psy_gtk_window_parent_class)->finalize(gobject);
}

// static GParamSpec* obj_properties[N_PROPS];

static void
set_monitor(PsyWindow* self, gint nth_monitor) {

    gint width_mm, height_mm;

    g_return_if_fail(PSY_IS_GTK_WINDOW(self));
    PsyGtkWindow* psywindow = PSY_GTK_WINDOW(self);

    GdkDisplay* display = gdk_display_get_default();
    guint num_monitors = g_list_model_get_n_items(
            gdk_display_get_monitors(display)
            );
    g_return_if_fail(nth_monitor >= 0);
    g_return_if_fail((guint)nth_monitor < num_monitors);

    GListModel* monitors = gdk_display_get_monitors(display);
    GdkMonitor* monitor = g_list_model_get_item(monitors, (guint) nth_monitor);

    width_mm = gdk_monitor_get_width_mm(monitor);
    height_mm= gdk_monitor_get_height_mm(monitor);
    g_print("%s: %d %d\n", __PRETTY_FUNCTION__, width_mm, height_mm);

    gtk_window_fullscreen_on_monitor(GTK_WINDOW(psywindow->window),  monitor);
    psy_window_set_width_mm(self, width_mm);
    psy_window_set_height_mm(self, height_mm);
    PSY_WINDOW_CLASS(psy_gtk_window_parent_class)->set_monitor(
            self, nth_monitor
            );
}

static void
clear(PsyWindow* self)
{
    gfloat bg_color[4];
    psy_window_get_background_color_values(self, bg_color);

    glClearColor(bg_color[0], bg_color[1], bg_color[2], bg_color[3]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

static void
draw_stimuli(PsyWindow* self, guint64 nth_frame, PsyTimePoint* tp)
{
    PsyClock* clk = psy_clock_new();
    PsyTimePoint* tref = psy_clock_now(clk);
    PsyDuration* dur = psy_time_point_subtract(tp, tref);
    gdouble dsec = psy_duration_get_seconds(dur);

    g_print("Difference between tp and tref = %lf seconds\n", dsec);
    g_object_unref(clk);
    g_object_unref(tref);
    g_object_unref(dur);
}

static void
psy_gtk_window_class_init(PsyGtkWindowClass* klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

//    object_class->set_property  = psy_gtk_window_set_property;
//    object_class->get_property  = psy_gtk_window_get_property;
    object_class->dispose           = psy_gtk_window_dispose;
    object_class->finalize          = psy_gtk_window_finalize;

    PsyWindowClass* psy_window_class = PSY_WINDOW_CLASS(klass);
    psy_window_class->set_monitor   = set_monitor;
    psy_window_class->clear         = clear;
    psy_window_class->draw_stimuli  = draw_stimuli;

//    g_object_class_install_properties(object_class, N_PROPS, obj_properties);    
}

/**
 * psy_gtk_window_new:(constructor):
 *
 * Returns a new #PsyGtkWindow instance on the first monitor.
 * @return
 */
PsyGtkWindow*
psy_gtk_window_new(void)
{
    PsyGtkWindow* window = psy_gtk_window_new_for_monitor(0);
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
 * Returns: a newly initialized window
 */
PsyGtkWindow*
psy_gtk_window_new_for_monitor(gint n)
{
    // First create a window as this initializes gtk when it hasn't been
    // initialized before.
    PsyGtkWindow* window = g_object_new(
            PSY_TYPE_GTK_WINDOW,
            "n-monitor", n,
            NULL);

    return window;
}

