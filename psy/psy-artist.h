
#pragma once

#include <glib-object.h>
#include "psy-visual-stimulus.h"

G_BEGIN_DECLS

#define PSY_TYPE_ARTIST psy_artist_get_type()
G_DECLARE_DERIVABLE_TYPE(
        PsyArtist,
        psy_artist,
        PSY,
        ARTIST,
        PsyVisualStimulus
        )

typedef struct _PsyArtistClass {
    PsyVisualStimulusClass parent;
} PsyArtistClass;

G_MODULE_EXPORT PsyArtist*
psy_artist_new(PsyVisualStimulus* stimulus);

G_MODULE_EXPORT void
psy_artist_set_stimulus(PsyArtist* self, PsyVisualStimulus* stimulus);

G_MODULE_EXPORT PsyVisualStimulus*
psy_artist_get_stimulus(PsyArtist* self);

G_END_DECLS

