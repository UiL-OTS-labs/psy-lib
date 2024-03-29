
#include "psy-picture-artist.h"
#include "psy-artist.h"
#include "psy-drawing-context.h"
#include "psy-matrix4.h"
#include "psy-picture.h"
#include "psy-shader-program.h"
#include "psy-vbuffer.h"
#include "psy-window.h"

/**
 * PsyPictureArtist:
 *
 * The artist that is capable of drawing instances of [class@Picture]
 * to a canvas.
 */

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

static PsyShaderProgram *
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
    gchar      *fn           = NULL;

    PsyPictureArtist  *artist  = PSY_PICTURE_ARTIST(self);
    PsyPicture        *picture = PSY_PICTURE(psy_artist_get_stimulus(self));
    PsyCanvas         *canvas  = psy_artist_get_canvas(self);
    PsyDrawingContext *context = psy_canvas_get_context(canvas);

    gint strategy;

    // clang-format off
    g_object_get(picture,
            "width", &width,
            "height", &height,
            "filename", &fn,
            "size-strategy", &strategy,
            NULL
            );
    // clang-format on

    PsyTexture *texture = psy_drawing_context_get_texture(context, fn);
    if (!texture) {
        g_critical("Unable to obtain texture \"%s\"from drawing context", fn);
        g_free(fn);
        return;
    }
    g_free(fn);

    if (!psy_texture_is_uploaded(texture)) {
        static int warn_once = 0;
        if (!warn_once) {
            g_warning("Texture %s is not uploaded, we recommend to do so "
                      "this warning is emitted once.",
                      psy_texture_get_filename(texture));
            warn_once = 1;
        }
        psy_texture_upload(texture, &error);
        if (error) {
            g_critical("Unable to upload texture %s",
                       psy_texture_get_filename(texture));
            g_clear_error(&error);
        }
    }

    psy_texture_bind(texture, &error);
    if (error) {
        g_critical("Unable to bind texture %s",
                   psy_texture_get_filename(texture));
        g_clear_error(&error);
    }

    if (strategy == PSY_PICTURE_STRATEGY_AUTOMATIC) {
        g_debug("Texture size = %d * %d",
                psy_texture_get_width(texture),
                psy_texture_get_height(texture));
        g_signal_emit_by_name(picture,
                              "auto-resize",
                              (float) psy_texture_get_width(texture),
                              (float) psy_texture_get_height(texture));
    }

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

/**
 * psy_picture_artist_new:
 * @canvas: The window on which this picture artist will draw its[class@Picture]
 *          stimuli
 * @stimulus: An instance of [class@Picture], which this artist will be drawing
 *
 * Create a new [class@PsyPictureArtist] that is going to draw @stimulus.
 *
 * Returns: A new instance to be freed with g_object_unref
 *          or[method@PictureArtist.free]
 */
PsyPictureArtist *
psy_picture_artist_new(PsyCanvas *canvas, PsyVisualStimulus *stimulus)
{
    g_return_val_if_fail(PSY_IS_CANVAS(canvas), NULL);
    g_return_val_if_fail(PSY_IS_VISUAL_STIMULUS(stimulus), NULL);

    // clang-format off
    PsyPictureArtist *picture_artist = g_object_new(
          PSY_TYPE_PICTURE_ARTIST,
          "canvas", canvas,
          "stimulus", stimulus,
          NULL
          );
    // clang-format on

    return picture_artist;
}

/**
 * psy_picture_artist_free:(skip)
 *
 * Frees a PictureArtist previously allocated with [ctor@PictureArtist.new]
 */
void
psy_picuture_artist_free(PsyPictureArtist *self)
{
    g_return_if_fail(PSY_IS_PICTURE_ARTIST(self));
    g_object_unref(self);
}
