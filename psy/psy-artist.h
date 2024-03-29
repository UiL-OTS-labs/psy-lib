
#pragma once

#include "psy-canvas.h"
#include "psy-drawing-context.h"
#include "psy-shader-program.h"
#include "psy-visual-stimulus.h"

#include <glib-object.h>

G_BEGIN_DECLS

// forward declarations
struct _PsyVisualStimulus;
typedef struct _PsyVisualStimulus PsyVisualStimulus;
struct _PsyCanvas;
typedef struct _PsyCanvas PsyCanvas;

#define PSY_TYPE_ARTIST psy_artist_get_type()
G_MODULE_EXPORT
G_DECLARE_DERIVABLE_TYPE(PsyArtist, psy_artist, PSY, ARTIST, GObject)

/**
 * PsyArtistClass:
 * @parent:The gobject class
 * @draw: the function that draws the stimulus, deriving classes are expected
 *        to overload this, but chain up, because this class sets up the
 *        transformation matrix.
 *        The default program loads a program that draws using a projection
 *        and a transformation matrix using a uniform color. Deriving classes
 *        may override the get_program function to draw with another program,
 *        but take care that the Artist.draw() expect the uniform and the
 *        color matrices to be available.
 * @get_program: This function obtains the shader program for this object. It
 *               is virtual, so that deriving classes may choose their own
 *               shader.
 */
typedef struct _PsyArtistClass {
    GObjectClass parent;

    void (*draw)(PsyArtist *self);
    PsyShaderProgram *(*get_program)(PsyArtist *self);

    gpointer reserved[16];

} PsyArtistClass;

G_MODULE_EXPORT PsyArtist *
psy_artist_new(PsyVisualStimulus *stimulus);

G_MODULE_EXPORT void
psy_artist_set_canvas(PsyArtist *self, PsyCanvas *canvas);

G_MODULE_EXPORT PsyCanvas *
psy_artist_get_canvas(PsyArtist *self);

G_MODULE_EXPORT void
psy_artist_set_stimulus(PsyArtist *self, PsyVisualStimulus *stimulus);

G_MODULE_EXPORT PsyVisualStimulus *
psy_artist_get_stimulus(PsyArtist *self);

G_MODULE_EXPORT void
psy_artist_draw(PsyArtist *self);

G_MODULE_EXPORT PsyDrawingContext *
psy_artist_get_context(PsyArtist *self);

G_MODULE_EXPORT PsyShaderProgram *
psy_artist_get_program(PsyArtist *self);

G_END_DECLS
