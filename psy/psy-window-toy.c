
#include <epoxy/gl.h>
#include <epoxy/gl_generated.h>
#include <epoxy/glx.h>
#include "psy-window-toy.h"

#include "gl/psy-gl-program.h"
#include "gl/psy-gl-vbuffer.h"
#include "gl/psy-gl-texture.h"

struct _PsyWindowToy {
    GtkWindow       parent;
    GtkWidget      *darea;
    gfloat          clear_color[4];
    guint           monitor;
    PsyProgram     *gl_program;
    PsyProgram     *gl_picture_program;
    PsyVBuffer     *vertices;
    PsyVBuffer     *picture_vertices;
    PsyTexture     *gl_texture;
    guint           n_frames;
    guint           fps_timeout;
    guint           tick_id; // In order to remove the tick callback
};

G_DEFINE_TYPE(PsyWindowToy, psy_window_toy, GTK_TYPE_WINDOW)

typedef enum {
    N_MONITOR = 1,
    N_PROPS
} PsyWindowToyProperty;

static gboolean
tick_callback(GtkWidget       *darea,
              GdkFrameClock   *clock,
              gpointer         data)
{
    (void) data;
    PsyWindowToy* win3d = PSY_WINDOW_TOY(data);
    gtk_gl_area_make_current(GTK_GL_AREA(darea));
    //glViewport(0, 0, 600, 600);
    GError* error = gtk_gl_area_get_error(GTK_GL_AREA(darea));
    if (error) {
        g_critical("An OpenGL error occurred: %s", error->message);
        return G_SOURCE_REMOVE;
    }

    gfloat *color = win3d->clear_color;
    gint64 us = gdk_frame_clock_get_frame_time(clock);
    gfloat seconds = (gfloat)us / 1e6;

    color[0] = (float) sin(seconds * 1.00) / 2 + .5;
    color[1] = (float) sin(seconds * 0.50) / 2 + .5;
    color[2] = (float) sin(seconds * 0.25) / 2 + .5;
    color[3] = 1.0;

    glClearColor(color[0], color[1], color[2], color[3]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    GLenum gl_error = glGetError();
    if (gl_error != GL_NO_ERROR)
        return G_SOURCE_CONTINUE;

    gint color_id = glGetUniformLocation(
            psy_gl_program_get_object_id(PSY_GL_PROGRAM(win3d->gl_program)),
            "ourColor"
            );

    psy_program_use(win3d->gl_program, &error);
    if (error) {
        gtk_gl_area_set_error(GTK_GL_AREA(darea), error);
        return G_SOURCE_REMOVE;
    }
    float tc;
    //tc = sinf(seconds * 2.0f * 3.141592654f) / 2.0 + .5f;
    tc = sinf(seconds) * .25 / 2.0 + .5f;
    glUniform4f(color_id, tc, tc, tc, 1.0f);

    psy_vbuffer_draw_triangles(win3d->vertices, &error);
    if (error) {
        gtk_gl_area_set_error(GTK_GL_AREA(darea), error);
        return G_SOURCE_REMOVE;
    }

    psy_program_use(win3d->gl_picture_program, &error);
    if (error) {
        gtk_gl_area_set_error(GTK_GL_AREA(darea), error);
        return G_SOURCE_REMOVE;
    }

    psy_texture_bind(win3d->gl_texture, &error);
    if (error) {
        gtk_gl_area_set_error(GTK_GL_AREA(darea), error);
        return G_SOURCE_REMOVE;
    }

    psy_vbuffer_draw_triangle_fan(win3d->picture_vertices, &error);
    if (error) {
        gtk_gl_area_set_error(GTK_GL_AREA(darea), error);
        return G_SOURCE_REMOVE;
    }

    glFlush();

    gtk_widget_queue_draw(darea);

    win3d->n_frames++;

    return G_SOURCE_CONTINUE;
}

static void
on_darea_resize(GtkDrawingArea* darea, gint width, gint height, gpointer data)
{
    PsyWindowToy *self = data;
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
init_textures(PsyWindowToy* self, GError **error)
{
    gtk_gl_area_make_current(GTK_GL_AREA(self->darea));
    PsyGlTexture  *texture  = psy_gl_texture_new();
    const gchar   *path     = "./share/Itálica_Owl.jpg";
    psy_texture_set_num_channels(PSY_TEXTURE(texture), 3);
    psy_texture_set_path(PSY_TEXTURE(texture), path);
    psy_texture_upload(PSY_TEXTURE(texture), error);

    self->gl_texture = PSY_TEXTURE(texture);
}

static void
init_shaders(PsyWindowToy* self, GError **error)
{
    // Uniform color program
    PsyGlProgram *program = psy_gl_program_new();
    self->gl_program = PSY_PROGRAM(program);

    psy_program_set_vertex_shader_from_path(
            self->gl_program, "./psy/uniform-color.vert", error
            );
    if (*error)
        goto fail;
    psy_program_set_fragment_shader_from_path(
            self->gl_program, "./psy/uniform-color.frag", error);
    if (*error)
        goto fail;

    psy_program_link(self->gl_program, error);
    if (*error)
        goto fail;

    // Picture program
    program = psy_gl_program_new();
    self->gl_picture_program = PSY_PROGRAM(program);

    psy_program_set_vertex_shader_from_path(
            self->gl_picture_program, "./psy/picture.vert", error
            );
    if (*error)
        goto fail;

    psy_program_set_fragment_shader_from_path(
            self->gl_picture_program, "./psy/picture.frag", error
            );
    if (*error)
        goto fail;

    psy_program_link(self->gl_picture_program, error);
    if (*error)
        goto fail;

    return;

    fail:
    g_object_unref(program);
}

static void
init_vertices(PsyWindowToy *self, GError **error)
{
    self->vertices = PSY_VBUFFER(psy_gl_vbuffer_new());
    g_assert(self->vertices);

    self->picture_vertices = PSY_VBUFFER(psy_gl_vbuffer_new());
    g_assert(self->picture_vertices);

    PsyVertex array[] = {
        {
            .pos = {-0.5f, -0.5f, 0.0f}, // left
            .color = {0},
            .texture_pos ={0}
        },
        {
            {0.5f, -0.5f, 0.0f}, // right
            {0, 0, 0, 0},
            {0, 0}
        },
        {
            {0.0f,  0.5f, 0.0f}, // top
            {0, 0 , 0, 0},
            {0, 0}
        }
    };

    PsyVertex pic_verts[] = {
        {
            .pos = {-0.6f,  0.6f, 0.001f}, // left top
            .color = {1.0f, 0.0f, 0.0f, 1.0f},
            .texture_pos = {0.0f, 0.0f}
        },
        {
            .pos = {-0.6f, -0.6f, 0}, // left bottom
            .color = {0.0f, 1.0f, 0.0f, 1.0f},
            .texture_pos = {0.0f, 2.0f}
        },
        {
            .pos = {0.6f, -0.6f, 0.0f}, // right bottom
            .color = {0.0f, 0.0f, 1.0f, 1.0f},
            .texture_pos = {1.0f, 2.0f}
        },
        {
            .pos = {0.6f, 0.6f, 0.0f}, // right top
            .color = {1.0f, 1.0f, 0.0f, 1.0f},
            .texture_pos = {1.0f, 0.0f}
        }
    };

    psy_vbuffer_set_from_data(self->vertices, array, 3);

    psy_vbuffer_set_from_data(self->picture_vertices,
                              pic_verts,
                              sizeof(pic_verts)/sizeof(pic_verts[0])
                              );
    g_assert(sizeof(pic_verts) == psy_vbuffer_get_size(self->picture_vertices));

    for (gsize i = 0; i < sizeof(pic_verts)/sizeof(pic_verts[0]); i++) {
        PsyVertexPos pos;
        PsyVertexColor color;
        PsyVertexTexPos tpos;
        psy_vbuffer_get_pos(self->picture_vertices, i, &pos);
        psy_vbuffer_get_color(self->picture_vertices, i, &color);
        psy_vbuffer_get_texture_pos(self->picture_vertices, i, &tpos);

        g_print("%f %f %f\t%f %f %f %f\t%f %f\n",
                pos.x, pos.y, pos.z,
                color.r, color.b, color.g, color.a,
                tpos.s, tpos.t);
    }

    gtk_gl_area_make_current(GTK_GL_AREA(self->darea));

    psy_vbuffer_upload(self->vertices, error);
    if (*error) {
        return;
    }
    psy_vbuffer_upload(self->picture_vertices, error);
    if (*error) {
        return;
    }

    g_assert(psy_vbuffer_is_uploaded(self->vertices));
    g_assert(psy_vbuffer_is_uploaded(self->picture_vertices));
}

static void
on_darea_realize(GtkGLArea* darea, PsyWindowToy* window)
{
    GError* error = NULL;
    gtk_gl_area_make_current(darea);

    if (gtk_gl_area_get_error(darea) != NULL)
        return;

    init_shaders(window, &error);

    if (error) {
        gtk_gl_area_set_error(darea, error);
        return;
    }

    init_vertices(window, &error);
    if (error) {
        gtk_gl_area_set_error(darea, error);
        return;
    }

    init_textures(window, &error);
    if(error) {
        gtk_gl_area_set_error(darea, error);
        return;
    }
    
    window->tick_id = gtk_widget_add_tick_callback(
		    GTK_WIDGET(darea), tick_callback, window, NULL
		    );
}

static void
on_darea_unrealize(GtkGLArea* area, PsyWindowToy* self)
{
    gtk_gl_area_make_current(area);

    g_clear_object(&self->gl_program);
    g_clear_object(&self->gl_picture_program);
    g_clear_object(&self->vertices);
    g_clear_object(&self->picture_vertices);
    g_clear_object(&self->gl_texture);
    
    gtk_widget_remove_tick_callback(GTK_WIDGET(self), self->tick_id);
}

static void
psy_window_toy_set_property(GObject        *object,
                        guint           property_id,
                        const GValue   *value,
                        GParamSpec     *spec)
{
    PsyWindowToy* self = PSY_WINDOW_TOY(object);

    switch((PsyWindowToyProperty) property_id) {
        case N_MONITOR:
            psy_window_toy_set_monitor(self, g_value_get_uint(value));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, spec);
    }
}

static void
psy_window_toy_get_property(GObject        *object,
                        guint           property_id,
                        GValue         *value,
                        GParamSpec     *spec)
{
    PsyWindowToy* self = PSY_WINDOW_TOY(object);

    switch((PsyWindowToyProperty) property_id) {
        case N_MONITOR:
            g_value_set_uint(value, psy_window_toy_get_monitor(self));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, spec);
    }
}

static gboolean
print_fps(PsyWindowToy* self)
{
    g_print("fps = %d\n", self->n_frames);
    self->n_frames = 0;
    return G_SOURCE_CONTINUE;
}

static void
psy_window_toy_init(PsyWindowToy* self)
{
    gfloat clearcolor[] = { 0.5, 0.5, 0.5, 1.0};
    memcpy(self->clear_color, clearcolor, sizeof(clearcolor));
    self->darea = gtk_gl_area_new();

    // Prepare common OpenGL setup prior to realization.
    gtk_gl_area_set_required_version(GTK_GL_AREA(self->darea), 4, 4);
    // gtk_gl_area_set_has_alpha(GTK_GL_AREA(self->darea), TRUE);
    gtk_gl_area_set_has_depth_buffer(GTK_GL_AREA(self->darea), TRUE);
    gtk_gl_area_set_has_stencil_buffer(GTK_GL_AREA(self->darea), FALSE);

    gtk_window_set_decorated(GTK_WINDOW(self), FALSE);

    gtk_window_set_child (GTK_WINDOW(self), self->darea);


    g_signal_connect(
            self->darea,
            "realize",
            G_CALLBACK(on_darea_realize),
            self);
    g_signal_connect(
            self->darea,
            "unrealize",
            G_CALLBACK(on_darea_unrealize),
            self);

    g_signal_connect(
            self->darea,
            "resize",
            G_CALLBACK(on_darea_resize),
            self
            );

    self->fps_timeout = g_timeout_add(1000, G_SOURCE_FUNC(print_fps), self);

    gtk_widget_show(GTK_WIDGET(self));
}

static void
psy_window_toy_dispose(GObject* gobject)
{
    PsyWindowToy* self = PSY_WINDOW_TOY(gobject);
    (void) self;

    if (self->fps_timeout) {
        g_source_remove(self->fps_timeout);
        self->fps_timeout = 0;
    }

    G_OBJECT_CLASS(psy_window_toy_parent_class)->dispose(gobject);
}

static void
psy_window_toy_finalize(GObject* gobject)
{
    PsyWindowToy* self = PSY_WINDOW_TOY(gobject);
    (void) self;

    G_OBJECT_CLASS(psy_window_toy_parent_class)->finalize(gobject);
}

static GParamSpec* obj_properties[N_PROPS];

static void
psy_window_toy_class_init(PsyWindowToyClass* klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    object_class->set_property  = psy_window_toy_set_property;
    object_class->get_property  = psy_window_toy_get_property;
    object_class->dispose       = psy_window_toy_dispose;
    object_class->finalize      = psy_window_toy_finalize;

    /**
     * PsyWindowToy:n-monitor:
     *
     * The number of the monitor on which the PsyWindowToy should be
     * presented.
     */
    obj_properties[N_MONITOR] =
        g_param_spec_uint("n-monitor",
                          "nmonitor",
                          "The number of the monitor to use for this window",
                          0,
                          G_MAXINT32,
                          0,
                          G_PARAM_CONSTRUCT | G_PARAM_READWRITE
                          );

    g_object_class_install_properties(object_class, N_PROPS, obj_properties);    
}

/**
 * psy_window_toy_new:(constructor):
 *
 * Returns a new #PsyWindowToy instance on the first monitor.
 * @return
 */
PsyWindowToy*
psy_window_toy_new(void)
{
    PsyWindowToy* window = g_object_new(PSY_TYPE_WINDOW_TOY, NULL);
    return window;
}

/**
 * psy_window_toy_new_for_montitor:(constructor):
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
PsyWindowToy*
psy_window_toy_new_for_monitor(guint n)
{
    GdkDisplay* disp = gdk_display_get_default();
    GListModel* monitors = gdk_display_get_monitors(disp);
    guint max = g_list_model_get_n_items(monitors);

    if (n >= max)
        n = max - 1;
    
    PsyWindowToy* window = g_object_new(
            PSY_TYPE_WINDOW_TOY,
            "n-monitor", n,
            NULL);

    return window;
}

/**
 * psy_window_toy_set_monitor:
 * @self:A #PsyWindowToy instance
 * @nth_monitor: A number between 0 and n - 1 where n is the number
 *               of available monitors.
 *
 * Display the the window on monitor monitor_num
 */
void
psy_window_toy_set_monitor(PsyWindowToy* self, guint nth_monitor)
{
    g_return_if_fail(PSY_IS_WINDOW_TOY(self));
    GdkDisplay* display = gdk_display_get_default();
    guint num_monitors = g_list_model_get_n_items(gdk_display_get_monitors(display));
    g_return_if_fail(nth_monitor < num_monitors);

    GListModel* monitors = gdk_display_get_monitors(display);
    GdkMonitor* monitor = g_list_model_get_item(monitors, nth_monitor);

    gtk_window_fullscreen_on_monitor(GTK_WINDOW(self),  monitor);
    self->monitor = nth_monitor;
}

/**
 * psy_window_toy_get_monitor:
 * @self: a #PsyWindowToy instance
 *
 * Returns the number of the monitor this window is being presented.
 * The result should b 0 for the first monitor to n - 1 for the last
 * where n is the number of monitors available to psy-lib.
 *
 * Returns: 0 to n - 1 where n is the number of monitors connected or
 *          -1 when self is not a valid monitor.
 */
guint
psy_window_toy_get_monitor(PsyWindowToy* self)
{
    g_return_val_if_fail(PSY_IS_WINDOW_TOY(self), -1);
    return self->monitor;
}
