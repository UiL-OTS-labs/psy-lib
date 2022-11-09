

#include "psy-cross-artist.h"
#include "psy-cross.h"
#include "psy-drawing-context.h"
#include "psy-vbuffer.h"
#include "psy-matrix4.h"
#include "psy-program.h"
#include "psy-window.h"

typedef struct _PsyCrossArtist {
    PsyArtist parent_instance;
    PsyVBuffer* vertices;
    gfloat x, y, z;
    gfloat xwidth, ywidth;
    gfloat xlength, ylength;
} PsyCrossArtist;

G_DEFINE_TYPE(PsyCrossArtist, psy_cross_artist, PSY_TYPE_ARTIST)

static void
psy_cross_artist_init(PsyCrossArtist *self)
{
    (void) self;
}

static void
psy_cross_artist_constructed(GObject* self)
{
    PsyCrossArtist* ca = PSY_CROSS_ARTIST(self);
    PsyWindow* window = psy_artist_get_window(PSY_ARTIST(self));
    PsyDrawingContext* context = psy_window_get_context(window);
    ca->vertices = psy_drawing_context_create_vbuffer(context);
}

static void
psy_cross_artist_dispose(GObject* object)
{
    PsyCrossArtist* self = PSY_CROSS_ARTIST(object);

    g_clear_object(&self->vertices);

    G_OBJECT_CLASS(psy_cross_artist_parent_class)->dispose(object);
}

static void
psy_cross_artist_finalize(GObject* object)
{
    G_OBJECT_CLASS(psy_cross_artist_parent_class)->finalize(object);
}

static void
cross_artist_draw(PsyArtist* self)
{
    PsyCrossArtist* artist = PSY_CROSS_ARTIST(self);
    PsyCross* cross = PSY_CROSS(psy_artist_get_stimulus(self));
    PsyWindow* window = psy_artist_get_window(self);
    PsyDrawingContext* context = psy_window_get_context(window);
    GError* error = NULL;
    const guint nverts = 14; // origin + 12 corners

    gfloat x, y, z;
    gboolean store_vertices = FALSE;
    gfloat line_length_x, line_length_y;
    gfloat line_width_x, line_width_y;

    PsyProgram* program = psy_drawing_context_get_program(
            context,
            PSY_UNIFORM_COLOR_PROGRAM_NAME
            );

    psy_program_use(program, &error);
    if (error) {
        g_critical("PsyCross unable to use program: %s", error->message);
        g_error_free(error);
        error = NULL;
    }

    g_object_get(cross,
            "x", &x,
            "y", &y,
            "z", &z,
            "line-length-x", &line_length_x,
            "line-length-y", &line_length_y,
            "line-width-x", &line_width_x,
            "line-width-y", &line_width_y,
            NULL
            );

    if (psy_vbuffer_get_nvertices(artist->vertices) != nverts) {
        psy_vbuffer_set_nvertices(artist->vertices, nverts);
        store_vertices = TRUE;
    }

    if (x != artist->x || y != artist->y || z != artist->z) {
        artist->x = x;
        artist->y = y;
        artist->z = z;
        store_vertices = TRUE;
    }

    if (artist->xwidth != line_width_x || artist->ywidth != line_length_x ||
        artist->xlength != line_length_x || artist->ylength + line_length_y) {
        artist->xlength = line_length_x;
        artist->ylength = line_length_y;
        artist->xwidth = line_width_x;
        artist->ywidth = line_width_y;
    }

    if (store_vertices) {
        // set origin and add 13 vertices in a clockwise manner
        psy_vbuffer_set_xyz (artist->vertices, 0, x, y, z);

        gfloat x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12;
        gfloat y1, y2, y3, y4, y5, y6, y7, y8, y9, y10, y11, y12;

        x1 = x + artist->ywidth/2.0;
        y1 = y + artist->ylength/2.0;
        
        x2 = x + artist->ywidth/2.0;
        y2 = y + artist->xwidth/2.0;

        x3 = x + artist->xlength/2.0;
        y3 = y + artist->xwidth/2.0;
        
        x4 = x + artist->xlength/2.0;
        y4 = y - artist->xwidth/2.0;
        
        x5 = x + artist->ywidth/2.0;
        y5 = y - artist->xwidth/2.0;
        
        x6 = x + artist->ywidth/2.0;
        y6 = y - artist->xlength/2.0;

        x7 = x - artist->ywidth/2.0;
        y7 = y - artist->xlength/2.0;

        x8 = x - artist->ywidth/2.0;
        y8 = y - artist->xwidth/2.0;

        x9 = x - artist->xlength/2.0;
        y9 = y - artist->xwidth/2.0;

        x10 = x - artist->xlength/2.0;
        y10 = y + artist->xwidth/2.0;

        x11 = x - artist->ywidth/2.0;
        y11 = y + artist->xwidth/2.0;
        
        x12 = x - artist->ywidth/2.0;
        y12 = y + artist->ylength/2.0;

        psy_vbuffer_set_xyz(artist->vertices, 1, x1, y1, z);
        psy_vbuffer_set_xyz(artist->vertices, 2, x2, y2, z);
        psy_vbuffer_set_xyz(artist->vertices, 3, x3, y3, z);
        psy_vbuffer_set_xyz(artist->vertices, 4, x4, y4, z);
        psy_vbuffer_set_xyz(artist->vertices, 5, x5, y5, z);
        psy_vbuffer_set_xyz(artist->vertices, 6, x6, y6, z);
        psy_vbuffer_set_xyz(artist->vertices, 7, x7, y7, z);
        psy_vbuffer_set_xyz(artist->vertices, 8, x8, y8, z);
        psy_vbuffer_set_xyz(artist->vertices, 9, x9, y9, z);
        psy_vbuffer_set_xyz(artist->vertices, 10, x10, y10, z);
        psy_vbuffer_set_xyz(artist->vertices, 11, x11, y11, z);
        psy_vbuffer_set_xyz(artist->vertices, 12, x12, y12, z);
        psy_vbuffer_set_xyz(artist->vertices, 13, x1, y1, z); // close the loop

        psy_vbuffer_upload(artist->vertices, &error);
        if (error) {
            g_critical("PsyCrossArtist: unable to upload vertices: %s",
                    error->message
                    );
            g_error_free(error);
        }
    }
    psy_vbuffer_draw_triangle_fan(artist->vertices, &error);
    if (error) {
        g_critical("PsyCrossArtist: unable to draw triangle_fan: %s",
                error->message
                );
        g_error_free(error);
    }
}

static void
psy_cross_artist_class_init(PsyCrossArtistClass* class)
{
    GObjectClass      *gobject_class = G_OBJECT_CLASS(class);
    PsyArtistClass    *artist_class  = PSY_ARTIST_CLASS(class);

    gobject_class->finalize     = psy_cross_artist_finalize;
    gobject_class->dispose      = psy_cross_artist_dispose;
    gobject_class->constructed  = psy_cross_artist_constructed;

    artist_class->draw          = cross_artist_draw;
}

/* ************ public functions ******************** */

PsyCrossArtist*
psy_cross_artist_new(PsyWindow* window, PsyVisualStimulus* stimulus)
{
    g_return_val_if_fail(PSY_IS_WINDOW(window), NULL);
    g_return_val_if_fail(PSY_IS_VISUAL_STIMULUS(stimulus), NULL);

    PsyCrossArtist *cross_artist = g_object_new(PSY_TYPE_CROSS_ARTIST,
          "window", window,
          "stimulus", stimulus,
          NULL
          );

    return cross_artist;
}

