
#include <math.h>

#include "psy-drawing-context.h"
#include "psy-matrix4.h"
#include "psy-program.h"
#include "psy-rectangle-artist.h"
#include "psy-rectangle.h"
#include "psy-vbuffer.h"

typedef struct _PsyRectangleArtist {
    PsyArtist   parent_instance;
    PsyVBuffer *vertices;
    gfloat      width, height;
} PsyRectangleArtist;

G_DEFINE_TYPE(PsyRectangleArtist, psy_rectangle_artist, PSY_TYPE_ARTIST)

static void
psy_rectangle_artist_init(PsyRectangleArtist *self)
{
    (void) self;
}

static void
psy_rectangle_artist_constructed(GObject *self)
{
    PsyArtist *artist = PSY_ARTIST(self);
    G_OBJECT_CLASS(psy_rectangle_artist_parent_class)->constructed(self);

    PsyRectangleArtist *ra      = PSY_RECTANGLE_ARTIST(self);
    PsyDrawingContext  *context = psy_artist_get_context(artist);
    ra->vertices                = psy_drawing_context_create_vbuffer(context);
}

static void
psy_rectangle_artist_dispose(GObject *object)
{
    PsyRectangleArtist *self = PSY_RECTANGLE_ARTIST(object);

    g_clear_object(&self->vertices);

    G_OBJECT_CLASS(psy_rectangle_artist_parent_class)->dispose(object);
}

static void
psy_rectangle_artist_finalize(GObject *object)
{
    G_OBJECT_CLASS(psy_rectangle_artist_parent_class)->finalize(object);
}

static void
rectangle_artist_draw(PsyArtist *self)
{
    PSY_ARTIST_CLASS(psy_rectangle_artist_parent_class)->draw(self);

    gfloat       rgba[4]        = {0.0, 0.0, 0.0, 1.0};
    PsyColor    *color          = NULL;
    GError      *error          = NULL;
    gboolean     store_vertices = FALSE;
    gfloat       width, height;
    const gchar *color_name   = "ourColor";
    const guint  num_vertices = 4;

    PsyRectangleArtist *artist = PSY_RECTANGLE_ARTIST(self);
    PsyRectangle *rectangle    = PSY_RECTANGLE(psy_artist_get_stimulus(self));

    PsyProgram *program = psy_artist_get_program(self);

    // clang-format off
    g_object_get(rectangle,
            "width", &width,
            "height", &height,
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
    psy_program_set_uniform_4f(program, color_name, rgba, &error);
    if (error) {
        g_critical("%s: Unable to set the color: %s", __func__, error->message);
        g_clear_error(&error);
    }

    if (psy_vbuffer_get_nvertices(artist->vertices) != num_vertices) {
        psy_vbuffer_set_nvertices(artist->vertices, num_vertices);
        store_vertices = TRUE;
    }

    width  = psy_rectangle_get_width(rectangle);
    height = psy_rectangle_get_height(rectangle);
    if (width != artist->width || height != artist->height) {
        artist->width  = width;
        artist->height = height;
        store_vertices = TRUE;
    }

    if (store_vertices) {
        gfloat half_width  = width / 2;
        gfloat half_height = height / 2;

        psy_vbuffer_set_xyz(artist->vertices, 0, -half_width, half_height, 0);
        psy_vbuffer_set_xyz(artist->vertices, 1, half_width, half_height, 0);
        psy_vbuffer_set_xyz(artist->vertices, 2, half_width, -half_height, 0);
        psy_vbuffer_set_xyz(artist->vertices, 3, -half_width, -half_height, 0);

        psy_vbuffer_upload(artist->vertices, &error);
        if (error) {
            g_critical("PsyRectangleArtist: unable to upload vertices: %s",
                       error->message);
            g_error_free(error);
        }
    }
    psy_vbuffer_draw_triangle_fan(artist->vertices, &error);
    if (error) {
        g_critical("PsyRectangleArtist: unable to draw triangle_fan: %s",
                   error->message);
        g_error_free(error);
    }

    g_object_unref(color);
}

static void
psy_rectangle_artist_class_init(PsyRectangleArtistClass *class)
{
    GObjectClass   *gobject_class = G_OBJECT_CLASS(class);
    PsyArtistClass *artist_class  = PSY_ARTIST_CLASS(class);

    gobject_class->finalize    = psy_rectangle_artist_finalize;
    gobject_class->dispose     = psy_rectangle_artist_dispose;
    gobject_class->constructed = psy_rectangle_artist_constructed;

    artist_class->draw = rectangle_artist_draw;
}

/* ************ public functions ******************** */

PsyRectangleArtist *
psy_rectangle_artist_new(PsyCanvas *canvas, PsyVisualStimulus *stimulus)
{
    g_return_val_if_fail(PSY_IS_CANVAS(canvas), NULL);
    g_return_val_if_fail(PSY_IS_VISUAL_STIMULUS(stimulus), NULL);

    // clang-format off
    PsyRectangleArtist *rectangle_artist = g_object_new(
          PSY_TYPE_RECTANGLE_ARTIST,
          "canvas", canvas,
          "stimulus", stimulus,
          NULL
          );
    // clang-format on

    return rectangle_artist;
}

/**
 * psy_rectangle_artist_free:(skip)
 *
 * Frees instances of [class@RectangleArtist]
 */
void
psy_rectangle_artist_free(PsyRectangleArtist *self)
{
    g_return_if_fail(PSY_IS_RECTANGLE_ARTIST(self));
    g_object_unref(self);
}
