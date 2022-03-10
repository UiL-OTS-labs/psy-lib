
#include <epoxy/gl.h>
#include <epoxy/glx.h>
#include "ddd-window.h"

#include "ddd-gl-program.h"
#include "ddd-gl-vbuffer.h"
#include "ddd-gl-texture.h"

struct _DddWindow {
    GtkWindow       parent;
    GtkWidget      *darea;
    gfloat          clear_color[4];
    guint           monitor;
    DddProgram     *gl_program;
    DddProgram     *gl_picture_program;
    DddVBuffer     *vertices;
    DddVBuffer     *picture_vertices;
    DddTexture     *gl_texture;
    guint           n_frames;
    guint           fps_timeout;
};

G_DEFINE_TYPE(DddWindow, ddd_window, GTK_TYPE_WINDOW)

typedef enum {
    N_MONITOR = 1,
    N_PROPS
} DddWindowProperty;

static gboolean
tick_callback(GtkWidget       *widget,
              GdkFrameClock   *clock,
              gpointer         data)
{
    (void) data;
    DddWindow* win3d = DDD_WINDOW(widget);
    gtk_gl_area_make_current(GTK_GL_AREA(win3d->darea));
    //glViewport(0, 0, 600, 600);
    GError* error = gtk_gl_area_get_error(GTK_GL_AREA(win3d->darea));
    if (error) {
        g_critical("An OpenGL error occurred: %s", error->message);
        return G_SOURCE_REMOVE;
    }

    gfloat *color = win3d->clear_color;
    gint64 us = gdk_frame_clock_get_frame_time(clock);
    gfloat seconds = (gfloat) (us / 1e6);

//    color[0] = (float) sin(seconds * 1.00) / 2 + .5;
//    color[1] = (float) sin(seconds * 0.50) / 2 + .5;
//    color[2] = (float) sin(seconds * 0.25) / 2 + .5;
//    color[3] = 1.0;

    glClearColor(color[0], color[1], color[2], color[3]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    gint color_id = glGetUniformLocation(
            ddd_gl_program_get_object_id(DDD_GL_PROGRAM(win3d->gl_program)),
            "ourColor"
            );

    ddd_program_use(win3d->gl_program, &error);
    if (error) {
        gtk_gl_area_set_error(GTK_GL_AREA(win3d->darea), error);
        return G_SOURCE_REMOVE;
    }
    float tc;
    //tc = sinf(seconds * 2.0f * 3.141592654f) / 2.0 + .5f;
    tc = sinf(seconds) * .25 / 2.0 + .5f;
    glUniform4f(color_id, tc, tc, tc, 1.0f);

    ddd_vbuffer_draw_triangles(win3d->vertices, &error);
    if (error) {
        gtk_gl_area_set_error(GTK_GL_AREA(win3d->darea), error);
        return G_SOURCE_REMOVE;
    }

    ddd_program_use(win3d->gl_picture_program, &error);
    if (error) {
        gtk_gl_area_set_error(GTK_GL_AREA(win3d->darea), error);
        return G_SOURCE_REMOVE;
    }

    ddd_texture_bind(win3d->gl_texture, &error);
    if (error) {
        gtk_gl_area_set_error(GTK_GL_AREA(win3d->darea), error);
        return G_SOURCE_REMOVE;
    }

    ddd_vbuffer_draw_triangle_fan(win3d->picture_vertices, &error);
    if (error) {
        gtk_gl_area_set_error(GTK_GL_AREA(win3d->darea), error);
        return G_SOURCE_REMOVE;
    }

    glFlush();

    gtk_widget_queue_draw(win3d->darea);

    win3d->n_frames++;

    return G_SOURCE_CONTINUE;
}

static void
on_darea_resize(GtkDrawingArea* darea, gint width, gint height, gpointer data)
{
    DddWindow *self = data;
    (void) self;
    gtk_gl_area_make_current(GTK_GL_AREA(darea));
    g_print("width %d, height %d, data %p\n", width, height, data);
    (void) width; (void) height;    //glViewport(-width/2, -height/2, width, height);
}

static void
init_textures(DddWindow* self, GError **error)
{
    gtk_gl_area_make_current(GTK_GL_AREA(self->darea));
    DddGlTexture  *texture  = ddd_gl_texture_new();
    const gchar   *path     = "./share/Itálica_Owl.jpg";
    ddd_texture_set_num_channels(DDD_TEXTURE(texture), 3);
    ddd_texture_set_path(DDD_TEXTURE(texture), path);
    ddd_texture_upload(DDD_TEXTURE(texture), error);

    self->gl_texture = DDD_TEXTURE(texture);
}

static void
init_shaders(DddWindow* self, GError **error)
{
    // Uniform color program
    DddGlProgram *program = ddd_gl_program_new();
    self->gl_program = DDD_PROGRAM(program);

    ddd_program_set_vertex_shader_from_path(
            self->gl_program, "./psy/uniform-color.vert", error
            );
    if (*error)
        goto fail;
    ddd_program_set_fragment_shader_from_path(
            self->gl_program, "./psy/uniform-color.frag", error);
    if (*error)
        goto fail;

    ddd_program_link(self->gl_program, error);
    if (*error)
        goto fail;

    // Picture program
    program = ddd_gl_program_new();
    self->gl_picture_program = DDD_PROGRAM(program);

    ddd_program_set_vertex_shader_from_path(
            self->gl_picture_program, "./psy/picture.vert", error
            );
    if (*error)
        goto fail;

    ddd_program_set_fragment_shader_from_path(
            self->gl_picture_program, "./psy/picture.frag", error
            );
    if (*error)
        goto fail;

    ddd_program_link(self->gl_picture_program, error);
    if (*error)
        goto fail;

    return;

    fail:
    g_object_unref(program);
}

static void
init_vertices(DddWindow *self, GError **error)
{
    self->vertices = DDD_VBUFFER(ddd_gl_vbuffer_new());
    g_assert(self->vertices);

    self->picture_vertices = DDD_VBUFFER(ddd_gl_vbuffer_new());
    g_assert(self->picture_vertices);

    DddVertex array[] = {
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

    DddVertex pic_verts[] = {
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

    ddd_vbuffer_set_from_data(self->vertices, array, 3);

    ddd_vbuffer_set_from_data(self->picture_vertices,
                              pic_verts,
                              sizeof(pic_verts)/sizeof(pic_verts[0])
                              );
    g_assert(sizeof(pic_verts) == ddd_vbuffer_get_size(self->picture_vertices));

    for (gsize i = 0; i < sizeof(pic_verts)/sizeof(pic_verts[0]); i++) {
        DddVertexPos pos;
        DddVertexColor color;
        DddVertexTexPos tpos;
        ddd_vbuffer_get_pos(self->picture_vertices, i, &pos);
        ddd_vbuffer_get_color(self->picture_vertices, i, &color);
        ddd_vbuffer_get_texture_pos(self->picture_vertices, i, &tpos);

        g_print("%f %f %f\t%f %f %f %f\t%f %f\n",
                pos.x, pos.y, pos.z,
                color.r, color.b, color.g, color.a,
                tpos.s, tpos.t);
    }

    gtk_gl_area_make_current(GTK_GL_AREA(self->darea));

    ddd_vbuffer_upload(self->vertices, error);
    if (*error) {
        return;
    }
    ddd_vbuffer_upload(self->picture_vertices, error);
    if (*error) {
        return;
    }

    g_assert(ddd_vbuffer_is_uploaded(self->vertices));
    g_assert(ddd_vbuffer_is_uploaded(self->picture_vertices));
}

static void
on_darea_realize(GtkGLArea* darea, DddWindow* window)
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
}

static void
on_darea_unrealize(GtkGLArea* area, DddWindow* self)
{
    gtk_gl_area_make_current(area);

    g_clear_object(&self->gl_program);
    g_clear_object(&self->gl_picture_program);
    g_clear_object(&self->vertices);
    g_clear_object(&self->picture_vertices);
    g_clear_object(&self->gl_texture);
}

static void
ddd_window_set_property(GObject        *object,
                        guint           property_id,
                        const GValue   *value,
                        GParamSpec     *spec)
{
    DddWindow* self = DDD_WINDOW(object);

    switch((DddWindowProperty) property_id) {
        case N_MONITOR:
            ddd_window_set_monitor(self, g_value_get_uint(value));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, spec);
    }
}

static void
ddd_window_get_property(GObject        *object,
                        guint           property_id,
                        GValue         *value,
                        GParamSpec     *spec)
{
    DddWindow* self = DDD_WINDOW(object);

    switch((DddWindowProperty) property_id) {
        case N_MONITOR:
            g_value_set_uint(value, ddd_window_get_monitor(self));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, spec);
    }
}

static gboolean
print_fps(DddWindow* self)
{
    g_print("fps = %d\n", self->n_frames);
    self->n_frames = 0;
    return G_SOURCE_CONTINUE;
}

static void
ddd_window_init(DddWindow* self)
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

    gtk_widget_add_tick_callback(GTK_WIDGET(self), tick_callback, NULL, NULL);

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
ddd_window_dispose(GObject* gobject)
{
    DddWindow* self = DDD_WINDOW(gobject);
    (void) self;

    if (self->fps_timeout) {
        g_source_remove(self->fps_timeout);
        self->fps_timeout = 0;
    }

    G_OBJECT_CLASS(ddd_window_parent_class)->dispose(gobject);
}

static void
ddd_window_finalize(GObject* gobject)
{
    DddWindow* self = DDD_WINDOW(gobject);
    (void) self;

    G_OBJECT_CLASS(ddd_window_parent_class)->finalize(gobject);
}

static GParamSpec* obj_properties[N_PROPS];

static void
ddd_window_class_init(DddWindowClass* klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    object_class->set_property  = ddd_window_set_property;
    object_class->get_property  = ddd_window_get_property;
    object_class->dispose       = ddd_window_dispose;
    object_class->finalize      = ddd_window_finalize;

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

DddWindow*
ddd_window_new(void)
{
    DddWindow* window = g_object_new(DDD_TYPE_WINDOW, NULL);
    return window;
}

DddWindow*
ddd_window_new_for_monitor(guint n)
{
    GdkDisplay* disp = gdk_display_get_default();
    GListModel* monitors = gdk_display_get_monitors(disp);
    guint max = g_list_model_get_n_items(monitors);

    if (n >= max)
        n = max - 1;
    
    DddWindow* window = g_object_new(
            DDD_TYPE_WINDOW,
            "n-monitor", n,
            NULL);

    return window;
}

void
ddd_window_set_monitor(DddWindow* self, guint monitor_num)
{
    g_return_if_fail(DDD_IS_WINDOW(self));
    GdkDisplay* display = gdk_display_get_default();
    guint num_monitors = g_list_model_get_n_items(gdk_display_get_monitors(display));
    g_return_if_fail(monitor_num < num_monitors);

    GListModel* monitors = gdk_display_get_monitors(display);
    GdkMonitor* monitor = g_list_model_get_item(monitors, monitor_num);

    gtk_window_fullscreen_on_monitor(GTK_WINDOW(self),  monitor);
    self->monitor = monitor_num;
}

guint
ddd_window_get_monitor(DddWindow* self)
{
    g_return_val_if_fail(DDD_IS_WINDOW(self), -1);
    return self->monitor;
}
