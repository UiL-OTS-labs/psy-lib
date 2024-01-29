#ifndef PSY_PICTURE_ARTIST_H
#define PSY_PICTURE_ARTIST_H

#include <psy-artist.h>
#include <psy-canvas.h>

G_BEGIN_DECLS

#define PSY_TYPE_PICTURE_ARTIST psy_picture_artist_get_type()
G_DECLARE_FINAL_TYPE(
    PsyPictureArtist, psy_picture_artist, PSY, PICTURE_ARTIST, PsyArtist)

G_MODULE_EXPORT PsyPictureArtist *
psy_picture_artist_new(PsyCanvas *canvas, PsyVisualStimulus *stimulus);

G_MODULE_EXPORT void
psy_picture_artist_free(PsyPictureArtist *self);

G_END_DECLS

#endif
