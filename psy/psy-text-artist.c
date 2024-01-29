
#include "psy-text-artist.h"
#include "psy-artist.h"
#include "psy-drawing-context.h"
#include "psy-duration.h"
#include "psy-matrix4.h"
#include "psy-shader-program.h"
#include "psy-text.h"
#include "psy-texture.h"
#include "psy-vbuffer.h"
#include "psy-window.h"

/**
 * PsyTextArtist:
 *
 * The artist that is capable of drawing instances of [class@Text]
 * to a canvas.
 */

typedef struct _PsyTextArtist {
    PsyArtist   parent_instance;
    gfloat      width, height;
    PsyVBuffer *vertices;
    PsyTexture *texture;
} PsyTextArtist;

G_DEFINE_TYPE(PsyTextArtist, psy_text_artist, PSY_TYPE_ARTIST)

static void
psy_text_artist_init(PsyTextArtist *self)
{
    (void) self;
}

static void
psy_text_artist_constructed(GObject *self)
{
    PsyArtist *artist = PSY_ARTIST(self);
    G_OBJECT_CLASS(psy_text_artist_parent_class)->constructed(self);

    PsyTextArtist     *text_artist = PSY_TEXT_ARTIST(self);
    PsyDrawingContext *context     = psy_artist_get_context(artist);

    text_artist->vertices = psy_drawing_context_create_vbuffer(context);
    text_artist->texture  = psy_drawing_context_create_texture(context);
}

static void
psy_text_artist_dispose(GObject *object)
{
    PsyTextArtist *self = PSY_TEXT_ARTIST(object);

    g_clear_object(&self->vertices);
    g_clear_object(&self->texture);

    G_OBJECT_CLASS(psy_text_artist_parent_class)->dispose(object);
}

static PsyShaderProgram *
text_artist_get_program(PsyArtist *artist)
{
    PsyDrawingContext *context = psy_artist_get_context(artist);
    return psy_drawing_context_get_program(context, PSY_PICTURE_PROGRAM_NAME);
}

static void
text_artist_draw(PsyArtist *self)
{
    PSY_ARTIST_CLASS(psy_text_artist_parent_class)->draw(self);

    gfloat   width, height; // dimensions of the rectangle
    gboolean is_dirty;      // Whether or not the TextStim changed

    GError     *error          = NULL;
    gboolean    store_vertices = FALSE;
    const guint num_vertices   = 4;

    PsyTextArtist *artist = PSY_TEXT_ARTIST(self);
    PsyText       *text   = PSY_TEXT(psy_artist_get_stimulus(self));

    // clang-format off
    g_object_get(text,
            "width", &width,
            "height", &height,
            "is-dirty", &is_dirty,
            NULL
            );
    // clang-format on

    if (is_dirty) {
        PsyImage *img = psy_text_create_stimulus(text);
        psy_texture_upload_image(artist->texture, img, &error);
        psy_text_set_is_dirty(text, FALSE);
    }

    psy_texture_bind(artist->texture, &error);

    if (error) {
        g_critical(
            "%s: Unable to bind texture: %s\n", __func__, error->message);
        g_clear_error(&error);
    }

    if (psy_vbuffer_get_nvertices(artist->vertices) != num_vertices) {
        psy_vbuffer_set_nvertices(artist->vertices, num_vertices);
        store_vertices = TRUE;
    }

    width  = psy_rectangle_get_width(PSY_RECTANGLE(text));
    height = psy_rectangle_get_height(PSY_RECTANGLE(text));
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
                .pos = {-half_width, half_height, 0}, // left top
                .texture_pos = {0.0, 0.0}
            },
            {
                .pos = {half_width, half_height, 0}, // right top
                .texture_pos = {1.0, 0.0}
            },
            {
                .pos = {half_width, -half_height, 0}, // right bottom
                .texture_pos = {1.0, 1.0}
            },
            {
                .pos = {-half_width, -half_height, 0}, // left bottom
                .texture_pos = {0.0, 1.0}
            }
        };
        // clang-format on

        psy_vbuffer_set_from_data(artist->vertices, vertices, num_vertices);

        psy_vbuffer_upload(artist->vertices, &error);
        if (error) {
            g_critical("PsyTextArtist: unable to upload vertices: %s",
                       error->message);
            g_error_free(error);
        }
    }
    psy_vbuffer_draw_triangle_fan(artist->vertices, &error);
    if (error) {
        g_critical("PsyTextArtist: unable to draw triangle_fan: %s",
                   error->message);
        g_error_free(error);
    }
}

static void
psy_text_artist_class_init(PsyTextArtistClass *class)
{
    GObjectClass   *gobject_class = G_OBJECT_CLASS(class);
    PsyArtistClass *artist_class  = PSY_ARTIST_CLASS(class);

    gobject_class->dispose     = psy_text_artist_dispose;
    gobject_class->constructed = psy_text_artist_constructed;

    artist_class->draw        = text_artist_draw;
    artist_class->get_program = text_artist_get_program;
}

/* ************ public functions ******************** */

/**
 * psy_text_artist_new:
 * @canvas: The window on which this text artist will draw its[class@Text]
 *          stimuli
 * @stimulus: An instance of [class@Text], which this artist will be drawing
 *
 * Create a new [class@PsyTextArtist] that is going to draw @stimulus.
 */
PsyTextArtist *
psy_text_artist_new(PsyCanvas *canvas, PsyVisualStimulus *stimulus)
{
    g_return_val_if_fail(PSY_IS_CANVAS(canvas), NULL);
    g_return_val_if_fail(PSY_IS_VISUAL_STIMULUS(stimulus), NULL);

    // clang-format off
    PsyTextArtist *text_artist = g_object_new(
          PSY_TYPE_TEXT_ARTIST,
          "canvas", canvas,
          "stimulus", stimulus,
          NULL
          );
    // clang-format on

    return text_artist;
}

/**
 * psy_text_artist_free:(skip)
 *
 * Frees an text artist
 */
void
psy_text_artist_free(PsyTextArtist *self)
{
    g_return_if_fail(PSY_IS_TEXT_ARTIST(self));
    g_object_unref(self);
}
