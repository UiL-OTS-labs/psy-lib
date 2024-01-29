#ifndef PSY_CIRCLE_ARTIST_H
#define PSY_CIRCLE_ARTIST_H

#include <psy-artist.h>
#include <psy-canvas.h>
#include <psy-circle.h>

G_BEGIN_DECLS

#define PSY_TYPE_CIRCLE_ARTIST psy_circle_artist_get_type()
G_DECLARE_FINAL_TYPE(
    PsyCircleArtist, psy_circle_artist, PSY, CIRCLE_ARTIST, PsyArtist)

G_MODULE_EXPORT PsyCircleArtist *
psy_circle_artist_new(PsyCanvas *canvas, PsyVisualStimulus *stimulus);

G_MODULE_EXPORT void
psy_circle_artist_free(PsyCircleArtist *self);

G_MODULE_EXPORT guint
psy_circle_artist_get_object_id(PsyCircleArtist *circle);

G_END_DECLS

#endif
