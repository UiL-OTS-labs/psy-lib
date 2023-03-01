
#include "psy-picture-artist.h"
#include "psy-artist.h"
#include "psy-drawing-context.h"
#include "psy-matrix4.h"
#include "psy-picture.h"
#include "psy-program.h"
#include "psy-vbuffer.h"
#include "psy-window.h"

typedef struct _PsyPictureArtist {
    PsyArtist   parent_instance;
    PsyVBuffer *vertices;
    gfloat      width, height;
    gchar      *fn;
} PsyPictureArtist;

G_DEFINE_TYPE(PsyPictureArtist, psy_picture_artist, PSY_TYPE_ARTIST)

static void
psy_picture_artist_init(PsyPictureArtist *self)
{
    (void) self;
}

static void
psy_picture_artist_constructed(GObject *self)
{
    PsyArtist *artist = PSY_ARTIST(self);
    G_OBJECT_CLASS(psy_picture_artist_parent_class)->constructed(self);

    PsyPictureArtist  *ra      = PSY_PICTURE_ARTIST(self);
    PsyDrawingContext *context = psy_artist_get_context(artist);
    ra->vertices               = psy_drawing_context_create_vbuffer(context);
}

static void
psy_picture_artist_dispose(GObject *object)
{
    PsyPictureArtist *self = PSY_PICTURE_ARTIST(object);

    g_clear_object(&self->vertices);

    G_OBJECT_CLASS(psy_picture_artist_parent_class)->dispose(object);
}

static void
psy_picture_artist_finalize(GObject *object)
{
    PsyPictureArtist *self = PSY_PICTURE_ARTIST(object);

    g_free(self->fn);
    G_OBJECT_CLASS(psy_picture_artist_parent_class)->finalize(object);
}

static PsyProgram *
picture_artist_get_program(PsyArtist *artist)
{
    PsyDrawingContext *context = psy_artist_get_context(artist);
    return psy_drawing_context_get_program(context, PSY_PICTURE_PROGRAM_NAME);
}

static void
picture_artist_draw(PsyArtist *self)
{
    PSY_ARTIST_CLASS(psy_picture_artist_parent_class)->draw(self);

    GError     *error          = NULL;
    gboolean    store_vertices = FALSE;
    gfloat      width, height;
    const guint num_vertices = 4;

    PsyPictureArtist  *artist  = PSY_PICTURE_ARTIST(self);
    PsyPicture        *picture = PSY_PICTURE(psy_artist_get_stimulus(self));
    PsyWindow         *window  = psy_artist_get_window(self);
    PsyDrawingContext *context = psy_window_get_context(window);

    PsyProgram *program = psy_drawing_context_get_program(
        context, PSY_UNIFORM_COLOR_PROGRAM_NAME);

    psy_program_use(program, &error);
    if (error) {
        g_critical("PsyPicture unable to use program: %s", error->message);
        g_error_free(error);
        error = NULL;
    }

    // clang-format off
    g_object_get(picture,
            "width", &width,
            "height", &height,
            NULL
            );
    // clang-format on

    if (psy_vbuffer_get_nvertices(artist->vertices) != num_vertices) {
        psy_vbuffer_set_nvertices(artist->vertices, num_vertices);
        store_vertices = TRUE;
    }

    width  = psy_rectangle_get_width(PSY_RECTANGLE(picture));
    height = psy_rectangle_get_height(PSY_RECTANGLE(picture));
    if (width != artist->width || height != artist->height) {
        artist->width  = width;
        artist->height = height;
        store_vertices = TRUE;
    }

    if (store_vertices) {
        gfloat half_width  = width / 2;
        gfloat half_height = height / 2;

        // clang-format off
        PsyVertex vertices[4] = {
            {
                .pos = {-half_width, half_width, 0},
                .texture_pos = {0.0, 0.0}
            },
            {
                .pos = {half_width, half_height, 0},
                .texture_pos = {1.0, 0.0}
            },
            {
                .pos = {half_width, -half_height, 0},
                .texture_pos = {1.0, 1.0}
            },
            {
                .pos = {-half_width, -half_height, 0},
                .texture_pos = {0.0, 1.0}
            }
        };
        // clang-format on

        psy_vbuffer_set_from_data(artist->vertices, vertices, num_vertices);

        psy_vbuffer_upload(artist->vertices, &error);
        if (error) {
            g_critical("PsyPictureArtist: unable to upload vertices: %s",
                       error->message);
            g_error_free(error);
        }
    }
    psy_vbuffer_draw_triangle_fan(artist->vertices, &error);
    if (error) {
        g_critical("PsyPictureArtist: unable to draw triangle_fan: %s",
                   error->message);
        g_error_free(error);
    }
}

static void
psy_picture_artist_class_init(PsyPictureArtistClass *class)
{
    GObjectClass   *gobject_class = G_OBJECT_CLASS(class);
    PsyArtistClass *artist_class  = PSY_ARTIST_CLASS(class);

    gobject_class->finalize    = psy_picture_artist_finalize;
    gobject_class->dispose     = psy_picture_artist_dispose;
    gobject_class->constructed = psy_picture_artist_constructed;

    artist_class->draw        = picture_artist_draw;
    artist_class->get_program = picture_artist_get_program;
}

/* ************ public functions ******************** */

PsyPictureArtist *
psy_picture_artist_new(PsyWindow *window, PsyVisualStimulus *stimulus)
{
    g_return_val_if_fail(PSY_IS_WINDOW(window), NULL);
    g_return_val_if_fail(PSY_IS_VISUAL_STIMULUS(stimulus), NULL);

    // clang-format off
    PsyPictureArtist *picture_artist = g_object_new(
          PSY_TYPE_PICTURE_ARTIST,
          "window", window,
          "stimulus", stimulus,
          NULL
          );
    // clang-format on

    return picture_artist;
}
