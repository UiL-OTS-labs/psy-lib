#ifndef PSY_TEXT_ARTIST_H
#define PSY_TEXT_ARTIST_H

#include <psy-artist.h>
#include <psy-canvas.h>

G_BEGIN_DECLS

#define PSY_TYPE_TEXT_ARTIST psy_text_artist_get_type()
G_DECLARE_FINAL_TYPE(
    PsyTextArtist, psy_text_artist, PSY, TEXT_ARTIST, PsyArtist)

G_MODULE_EXPORT PsyTextArtist *
psy_text_artist_new(PsyCanvas *canvas, PsyVisualStimulus *stimulus);

G_MODULE_EXPORT void
psy_text_artist_free(PsyTextArtist *self);

G_END_DECLS

#endif
