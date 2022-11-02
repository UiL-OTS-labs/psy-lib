
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

G_MODULE_EXPORT gint64
psy_visual_stimulus_get_num_frames(PsyVisualStimulus* stimulus);

G_MODULE_EXPORT gint64
psy_visual_stimulus_get_nth_frame(PsyVisualStimulus* stimulus);

G_MODULE_EXPORT void
psy_visual_stimulus_update (
        PsyVisualStimulus  *stim,
        PsyTimePoint       *frame_time,
        gint64              nth_frame
        );

G_MODULE_EXPORT gboolean
psy_visual_stimulus_is_scheduled(PsyVisualStimulus* stimulus);

G_MODULE_EXPORT void
psy_visual_stimulus_set_start_frame(
        PsyVisualStimulus* stimulus, gint64 frame_num
        );

G_MODULE_EXPORT gint64
psy_visual_stimulus_get_start_frame(PsyVisualStimulus* stimulus);

G_MODULE_EXPORT gfloat
psy_visual_stimulus_get_x(PsyVisualStimulus* stimulus);
G_MODULE_EXPORT void 
psy_visual_stimulus_set_x(PsyVisualStimulus* stimulus, gfloat x);

G_MODULE_EXPORT gfloat
psy_visual_stimulus_get_y(PsyVisualStimulus* stimulus);
G_MODULE_EXPORT void 
psy_visual_stimulus_set_y(PsyVisualStimulus* stimulus, gfloat y);

G_MODULE_EXPORT gfloat
psy_visual_stimulus_get_z(PsyVisualStimulus* stimulus);
G_MODULE_EXPORT void 
psy_visual_stimulus_set_z(PsyVisualStimulus* stimulus, gfloat z);


G_END_DECLS
