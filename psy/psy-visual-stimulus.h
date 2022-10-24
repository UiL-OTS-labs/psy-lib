
#pragma once

#include "psy-stimulus.h"
#include "psy-time-point.h"
#include "psy-window.h"

G_BEGIN_DECLS

#define PSY_TYPE_VISUAL_STIMULUS psy_visual_stimulus_get_type()
G_DECLARE_DERIVABLE_TYPE(
        PsyVisualStimulus,
        psy_visual_stimulus,
        PSY,
        VISUAL_STIMULUS,
        PsyStimulus
        )

typedef struct _PsyVisualStimulusClass {
    PsyStimulusClass parent;
    void (*update) (PsyVisualStimulus* stim, PsyTimePoint* frame_time, gint64 nth_frame);
} PsyVisualStimulusClass;

G_MODULE_EXPORT PsyWindow*
psy_visual_stimulus_get_window(PsyVisualStimulus* stimulus);

G_MODULE_EXPORT void 
psy_visual_stimulus_set_window(PsyVisualStimulus* stimulus, PsyWindow* window);

G_MODULE_EXPORT
void psy_visual_stimulus_update (
        PsyVisualStimulus  *stim,
        PsyTimePoint       *frame_time,
        gint64              nth_frame
        );

G_END_DECLS
