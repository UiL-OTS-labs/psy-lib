
#include <math.h>
#include <epoxy/gl.h>
#include <graphene.h>

#include "graphene-matrix.h"
#include "psy-circle.h"
#include "psy-gl-circle.h"
#include "psy-gl-vbuffer.h"
#include "psy-matrix4.h"
#include "psy-program.h"
#include "psy-vbuffer.h"
#include "psy-window.h"

typedef struct _PsyGlCircle {
    PsyArtist parent_instance;
    PsyVBuffer* vertices;
    gfloat radius;
} PsyGlCircle;

G_DEFINE_TYPE(PsyGlCircle, psy_gl_circle, PSY_TYPE_ARTIST)

//typedef enum {
//    PROP_NULL,
//    NUM_PROPERTIES
//} PsyGlCircleProperty;
//
//static GParamSpec* gl_circle_properties[NUM_PROPERTIES];
//
//static void
//psy_gl_circle_set_property(GObject        *object,
//                           guint           prop_id,
//                           const GValue   *value,
//                           GParamSpec     *pspec)
//{
//    PsyGlCircle* self = PSY_GL_CIRCLE(object);
//    (void) self, (void) value;
//
//    switch((PsyGlCircleProperty) prop_id) {
//        default:
//            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
//    }
//}
//
//static void
//psy_gl_circle_get_property(GObject    *object,
//                           guint       prop_id,
//                           GValue     *value,
//                           GParamSpec *pspec)
//{
//    PsyGlCircle* self = PSY_GL_CIRCLE(object);
//
//    switch((PsyGlCircleProperty) prop_id) {
//        default:
//            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
//    }
//}

static void
psy_gl_circle_init(PsyGlCircle *self)
{
    self->vertices = PSY_VBUFFER(psy_gl_vbuffer_new());
}

static void
psy_gl_circle_dispose(GObject* object)
{
    PsyGlCircle* self = PSY_GL_CIRCLE(object);

    g_clear_object(&self->vertices);

    G_OBJECT_CLASS(psy_gl_circle_parent_class)->dispose(object);
}

static void
psy_gl_circle_finalize(GObject* object)
{
    G_OBJECT_CLASS(psy_gl_circle_parent_class)->finalize(object);
}

static void
gl_circle_draw(PsyArtist* self)
{
    PsyGlCircle* artist = PSY_GL_CIRCLE(self);
    PsyCircle* circle = PSY_CIRCLE(psy_artist_get_stimulus(self));
    PsyWindow* window = psy_artist_get_window(self);
    GError* error = NULL;

    PsyProgram* program = psy_window_get_shader_program(
            window,
            PSY_PROGRAM_UNIFORM_COLOR
            );

    psy_program_use(program, &error);
    if (error) {
        g_critical("PsyCircle unable to use program: %s", error->message);
        g_error_free(error);
        error = NULL;
    }

    gboolean store_vertices = FALSE;
    gfloat radius;
    
    guint num_vertices = psy_circle_get_num_vertices(circle);
    if (num_vertices != psy_vbuffer_get_nvertices(artist->vertices)) {
        psy_vbuffer_set_nvertices(artist->vertices, num_vertices);
        store_vertices = TRUE;
    }

    radius = psy_circle_get_radius(circle);
    if (radius != artist->radius) {
        artist->radius = radius;
        store_vertices = TRUE;
    }

    if (store_vertices) {
        gfloat x = 0, y = 0, z = 0;
        float twopi = M_PI * 2.0;
        for (gsize i = 0; i < num_vertices; i++) {
            x = cos(i * twopi / num_vertices) * radius;
            y = sin(i * twopi / num_vertices) * radius;
            psy_vbuffer_set_xyz(artist->vertices, i, x, y, z);
        }
        psy_vbuffer_upload(artist->vertices, &error);
        if (error) {
            g_critical("PsyGLCircle: unable to upload vertices: %s",
                    error->message
                    );
            g_error_free(error);
        }
    }
    psy_vbuffer_draw_triangle_fan(artist->vertices, &error);
}

static void
psy_gl_circle_class_init(PsyGlCircleClass* class)
{
    GObjectClass      *gobject_class = G_OBJECT_CLASS(class);
    PsyArtistClass    *artist_class  = PSY_ARTIST_CLASS(class);

    gobject_class->finalize     = psy_gl_circle_finalize;
    gobject_class->dispose      = psy_gl_circle_dispose;

    artist_class->draw          = gl_circle_draw;

//    g_object_class_install_properties(gobject_class,
//                                      NUM_PROPERTIES,
//                                      gl_circle_properties);
}

/* ************ public functions ******************** */

PsyGlCircle*
psy_gl_circle_new(PsyWindow* window, PsyVisualStimulus* stimulus)
{
    PsyGlCircle *gl_circle = g_object_new(PSY_TYPE_GL_CIRCLE,
          "window", window,
          "stimulus", stimulus,
          NULL
          );

    return gl_circle;
}

