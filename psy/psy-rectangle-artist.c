
#include <math.h>

#include "psy-rectangle-artist.h"
#include "psy-rectangle.h"
#include "psy-drawing-context.h"
#include "psy-vbuffer.h"
#include "psy-matrix4.h"
#include "psy-program.h"
#include "psy-window.h"

typedef struct _PsyRectangleArtist {
    PsyArtist parent_instance;
    PsyVBuffer* vertices;
    gfloat x, y, z;
    gfloat width, height;
} PsyRectangleArtist;

G_DEFINE_TYPE(PsyRectangleArtist, psy_rectangle_artist, PSY_TYPE_ARTIST)

static void
psy_rectangle_artist_init(PsyRectangleArtist *self)
{
    (void) self;
}

static void
psy_cirlcle_artist_constructed(GObject* self)
{
    PsyRectangleArtist* ra = PSY_RECTANGLE_ARTIST(self);
    PsyWindow* window = psy_artist_get_window(PSY_ARTIST(self));
    PsyDrawingContext* context = psy_window_get_context(window);
    ra->vertices = psy_drawing_context_create_vbuffer(context);
}

static void
psy_rectangle_artist_dispose(GObject* object)
{
    PsyRectangleArtist* self = PSY_RECTANGLE_ARTIST(object);

    g_clear_object(&self->vertices);

    G_OBJECT_CLASS(psy_rectangle_artist_parent_class)->dispose(object);
}

static void
psy_rectangle_artist_finalize(GObject* object)
{
    G_OBJECT_CLASS(psy_rectangle_artist_parent_class)->finalize(object);
}

static void
rectangle_artist_draw(PsyArtist* self)
{
    gfloat x, y, z;
    gfloat rgba[4] = {0.0, 0.0, 0.0, 1.0};
    PsyColor* color = NULL;
    GError* error = NULL;
    gboolean store_vertices = FALSE;
    gfloat width, height;
    const gchar* color_name = "ourColor";
    const guint num_vertices = 4;

    PsyRectangleArtist* artist = PSY_RECTANGLE_ARTIST(self);
    PsyRectangle* rectangle = PSY_RECTANGLE(psy_artist_get_stimulus(self));
    PsyWindow* window = psy_artist_get_window(self);
    PsyDrawingContext* context = psy_window_get_context(window);

    PsyProgram* program = psy_drawing_context_get_program(
            context,
            PSY_UNIFORM_COLOR_PROGRAM_NAME
            );

    psy_program_use(program, &error);
    if (error) {
        g_critical("PsyRectangle unable to use program: %s", error->message);
        g_error_free(error);
        error = NULL;
    }

    g_object_get(rectangle,
            "x", &x,
            "y", &y,
            "z", &z,
            "width", &width,
            "height", &height,
            "color", &color,
            NULL
            );

    if (color) {
        g_object_get(color,
                "r", &rgba[0],
                "g", &rgba[1],
                "b", &rgba[2],
                "a", &rgba[3],
                NULL);
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

    if (x != artist->x || y != artist->y || z != artist->z) {
        artist->x = x;
        artist->y = y;
        artist->z = z;
        store_vertices = TRUE;
    }

    width = psy_rectangle_get_width(rectangle);
    height = psy_rectangle_get_height(rectangle);
    if (width != artist->width || height != artist->height) {
        artist->width = width;
        artist->height = height;
        store_vertices = TRUE;
    }

    if (store_vertices) {
        gfloat half_width = width/2;
        gfloat half_height = height/2;
        psy_vbuffer_set_xyz(artist->vertices, 0, x - half_width, y + half_height, z);
        psy_vbuffer_set_xyz(artist->vertices, 1, x + half_width, y + half_height, z);
        psy_vbuffer_set_xyz(artist->vertices, 2, x + half_width, y - half_height, z);
        psy_vbuffer_set_xyz(artist->vertices, 3, x - half_width, y - half_height, z);


        psy_vbuffer_upload(artist->vertices, &error);
        if (error) {
            g_critical("PsyRectangleArtist: unable to upload vertices: %s",
                    error->message
                    );
            g_error_free(error);
        }
    }
    psy_vbuffer_draw_triangle_fan(artist->vertices, &error);
    if (error) {
        g_critical("PsyRectangleArtist: unable to draw triangle_fan: %s",
                error->message
                );
        g_error_free(error);
    }

}

static void
psy_rectangle_artist_class_init(PsyRectangleArtistClass* class)
{
    GObjectClass      *gobject_class = G_OBJECT_CLASS(class);
    PsyArtistClass    *artist_class  = PSY_ARTIST_CLASS(class);

    gobject_class->finalize     = psy_rectangle_artist_finalize;
    gobject_class->dispose      = psy_rectangle_artist_dispose;
    gobject_class->constructed  = psy_cirlcle_artist_constructed;

    artist_class->draw          = rectangle_artist_draw;
}

/* ************ public functions ******************** */

PsyRectangleArtist*
psy_rectangle_artist_new(PsyWindow* window, PsyVisualStimulus* stimulus)
{
    g_return_val_if_fail(PSY_IS_WINDOW(window), NULL);
    g_return_val_if_fail(PSY_IS_VISUAL_STIMULUS(stimulus), NULL);

    PsyRectangleArtist *rectangle_artist = g_object_new(PSY_TYPE_RECTANGLE_ARTIST,
          "window", window,
          "stimulus", stimulus,
          NULL
          );

    return rectangle_artist;
}

