
#include <math.h>

#include "psy-circle-artist.h"
#include "psy-circle.h"
#include "psy-drawing-context.h"
#include "psy-matrix4.h"
#include "psy-shader-program.h"
#include "psy-vbuffer.h"
#include "psy-window.h"

typedef struct _PsyCircleArtist {
    PsyArtist   parent_instance;
    PsyVBuffer *vertices;
    gfloat      x, y, z;
    gfloat      radius;
} PsyCircleArtist;

G_DEFINE_TYPE(PsyCircleArtist, psy_circle_artist, PSY_TYPE_ARTIST)

static void
psy_circle_artist_init(PsyCircleArtist *self)
{
    (void) self;
}

static void
psy_circle_artist_constructed(GObject *self)
{
    G_OBJECT_CLASS(psy_circle_artist_parent_class)->constructed(self);

    PsyCircleArtist   *ca      = PSY_CIRCLE_ARTIST(self);
    PsyDrawingContext *context = psy_artist_get_context(PSY_ARTIST(self));
    ca->vertices               = psy_drawing_context_create_vbuffer(context);
}

static void
psy_circle_artist_dispose(GObject *object)
{
    PsyCircleArtist *self = PSY_CIRCLE_ARTIST(object);

    g_clear_object(&self->vertices);

    G_OBJECT_CLASS(psy_circle_artist_parent_class)->dispose(object);
}

static void
psy_circle_artist_finalize(GObject *object)
{
    G_OBJECT_CLASS(psy_circle_artist_parent_class)->finalize(object);
}

static void
circle_artist_draw(PsyArtist *self)
{
    // First let the psy artist setup the model matrix
    PSY_ARTIST_CLASS(psy_circle_artist_parent_class)->draw(self);

    gfloat       rgba[4]        = {0.0, 0.0, 0.0, 1.0};
    PsyColor    *color          = NULL;
    GError      *error          = NULL;
    gboolean     store_vertices = FALSE;
    gfloat       radius;
    guint        num_vertices;
    const gchar *color_name = "ourColor";

    PsyCircleArtist *artist = PSY_CIRCLE_ARTIST(self);
    PsyCircle       *circle = PSY_CIRCLE(psy_artist_get_stimulus(self));

    PsyShaderProgram *program = psy_artist_get_program(self);

    // clang-format off
    g_object_get(circle,
            "num-vertices", &num_vertices,
            "radius", &radius,
            "color", &color,
            NULL
            );
    // clang-format on

    if (color) {
        // clang-format off
        g_object_get(color,
                "r", &rgba[0],
                "g", &rgba[1],
                "b", &rgba[2],
                "a", &rgba[3],
                NULL);
        // clang-format on
    }

    psy_shader_program_set_uniform_4f(program, color_name, rgba, &error);
    if (error) {
        g_critical("%s: Unable to set the color: %s", __func__, error->message);
        g_clear_error(&error);
    }

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
        gfloat twopi = M_PI * 2.0;
        gfloat nx, ny;
        for (gsize i = 0; i < num_vertices; i++) {
            nx = cos(i * twopi / num_vertices) * radius;
            ny = sin(i * twopi / num_vertices) * radius;
            psy_vbuffer_set_xyz(artist->vertices, i, nx, ny, 0);
        }
        psy_vbuffer_upload(artist->vertices, &error);
        if (error) {
            g_critical("PsyCircleArtist: unable to upload vertices: %s",
                       error->message);
            g_error_free(error);
        }
    }
    psy_vbuffer_draw_triangle_fan(artist->vertices, &error);
    if (error) {
        g_critical("PsyCircleArtist: unable to draw triangle_fan: %s",
                   error->message);
        g_error_free(error);
    }

    g_object_unref(color);
}

static void
psy_circle_artist_class_init(PsyCircleArtistClass *class)
{
    GObjectClass   *gobject_class = G_OBJECT_CLASS(class);
    PsyArtistClass *artist_class  = PSY_ARTIST_CLASS(class);

    gobject_class->finalize    = psy_circle_artist_finalize;
    gobject_class->dispose     = psy_circle_artist_dispose;
    gobject_class->constructed = psy_circle_artist_constructed;

    artist_class->draw = circle_artist_draw;
}

/* ************ public functions ******************** */

PsyCircleArtist *
psy_circle_artist_new(PsyCanvas *canvas, PsyVisualStimulus *stimulus)
{
    g_return_val_if_fail(PSY_IS_CANVAS(canvas), NULL);
    g_return_val_if_fail(PSY_IS_VISUAL_STIMULUS(stimulus), NULL);

    PsyCircleArtist *circle_artist = g_object_new(
        PSY_TYPE_CIRCLE_ARTIST, "canvas", canvas, "stimulus", stimulus, NULL);

    return circle_artist;
}

void
psy_circle_artist_free(PsyCircleArtist *self)
{
    g_return_if_fail(PSY_IS_CIRCLE_ARTIST(self));
    g_object_unref(self);
}
