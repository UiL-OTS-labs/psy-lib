#ifndef PSY_PICTURE_ARTIST_H
#define PSY_PICTURE_ARTIST_H

#include <psy-artist.h>
#include <psy-window.h>

G_BEGIN_DECLS

#define PSY_TYPE_PICTURE_ARTIST psy_picture_artist_get_type()
G_DECLARE_FINAL_TYPE(
    PsyPictureArtist, psy_picture_artist, PSY, PICTURE_ARTIST, PsyArtist)

G_MODULE_EXPORT PsyPictureArtist *
psy_picture_artist_new(PsyWindow *window, PsyVisualStimulus *stimulus);

G_END_DECLS

#endif
