#ifndef PSY_CROSS_ARTIST_H
#define PSY_CROSS_ARTIST_H

#include <psy-artist.h>
#include <psy-cross.h>
#include <psy-window.h>

G_BEGIN_DECLS

#define PSY_TYPE_CROSS_ARTIST psy_cross_artist_get_type()
G_DECLARE_FINAL_TYPE(
    PsyCrossArtist, psy_cross_artist, PSY, CROSS_ARTIST, PsyArtist)

G_MODULE_EXPORT PsyCrossArtist *
psy_cross_artist_new(PsyWindow *window, PsyVisualStimulus *stimulus);

G_MODULE_EXPORT guint
psy_cross_artist_get_object_id(PsyCrossArtist *cross);

G_END_DECLS

#endif
