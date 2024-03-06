#ifndef PSY_RECTANGLE_ARTIST_H
#define PSY_RECTANGLE_ARTIST_H

#include <psy-artist.h>
#include <psy-canvas.h>
#include <psy-rectangle.h>

G_BEGIN_DECLS

#define PSY_TYPE_RECTANGLE_ARTIST psy_rectangle_artist_get_type()

G_MODULE_EXPORT
G_DECLARE_FINAL_TYPE(
    PsyRectangleArtist, psy_rectangle_artist, PSY, RECTANGLE_ARTIST, PsyArtist)

G_MODULE_EXPORT PsyRectangleArtist *
psy_rectangle_artist_new(PsyCanvas *canvas, PsyVisualStimulus *stimulus);

G_MODULE_EXPORT void
psy_rectangle_artist_free(PsyRectangleArtist *self);

G_MODULE_EXPORT guint
psy_rectangle_artist_get_object_id(PsyRectangleArtist *rectangle);

G_END_DECLS

#endif
