
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
    gfloat x, y, z;
    gfloat radius;
} PsyGlCircle;

G_DEFINE_TYPE(PsyGlCircle, psy_gl_circle, PSY_TYPE_ARTIST)

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

    gfloat x, y, z;
    gboolean store_vertices = FALSE;
    gfloat radius;
    guint num_vertices;

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

    g_object_get(circle,
            "x", &x,
            "y", &y,
            "z", &z,
            "num-vertices", &num_vertices,
            "radius", &radius,
            NULL
            );

    if (num_vertices != psy_vbuffer_get_nvertices(artist->vertices)) {
        psy_vbuffer_set_nvertices(artist->vertices, num_vertices);
        store_vertices = TRUE;
    }

    if (x != artist->x || y != artist->y || z != artist->z) {
        artist->x = x;
        artist->y = y;
        artist->z = z;
        store_vertices = TRUE;
    }

    radius = psy_circle_get_radius(circle);
    if (radius != artist->radius) {
        artist->radius = radius;
        store_vertices = TRUE;
    }

    if (store_vertices) {
        gfloat twopi = M_PI * 2.0;
        gfloat nx, ny;
        for (gsize i = 0; i < num_vertices; i++) {
            nx = x + cos(i * twopi / num_vertices) * radius;
            ny = y + sin(i * twopi / num_vertices) * radius;
            psy_vbuffer_set_xyz(artist->vertices, i, nx, ny, z);
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
    if (error) {
        g_critical("PsyGLCircle: unable to draw triangle_fan: %s",
                error->message
                );
        g_error_free(error);
    }

}

static void
psy_gl_circle_class_init(PsyGlCircleClass* class)
{
    GObjectClass      *gobject_class = G_OBJECT_CLASS(class);
    PsyArtistClass    *artist_class  = PSY_ARTIST_CLASS(class);

    gobject_class->finalize     = psy_gl_circle_finalize;
    gobject_class->dispose      = psy_gl_circle_dispose;

    artist_class->draw          = gl_circle_draw;
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

