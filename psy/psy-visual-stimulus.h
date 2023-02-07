
#pragma once

#include "psy-color.h"
#include "psy-stimulus.h"
#include "psy-time-point.h"

G_BEGIN_DECLS

/*
 * Forward declaration in order to avoid cyclic header inclusion.
 */
struct _PsyWindow;
typedef struct _PsyWindow PsyWindow;

#define PSY_TYPE_VISUAL_STIMULUS psy_visual_stimulus_get_type()
G_DECLARE_DERIVABLE_TYPE(
    PsyVisualStimulus, psy_visual_stimulus, PSY, VISUAL_STIMULUS, PsyStimulus)

/**
 * PsyVisualStimulusClass:
 * @parent: the parentclass of PsyVisualStimulus it derives form PsyStimulus.
 * @update: signal emitted just prior to the object should be painted again
 */
typedef struct _PsyVisualStimulusClass {
    PsyStimulusClass parent;
    void (*update)(PsyVisualStimulus *stim,
                   PsyTimePoint      *frame_time,
                   gint64             nth_frame);
} PsyVisualStimulusClass;

G_MODULE_EXPORT PsyWindow *
psy_visual_stimulus_get_window(PsyVisualStimulus *stimulus);

G_MODULE_EXPORT void
psy_visual_stimulus_set_window(PsyVisualStimulus *stimulus, PsyWindow *window);

G_MODULE_EXPORT gint64
psy_visual_stimulus_get_num_frames(PsyVisualStimulus *self);

G_MODULE_EXPORT gint64
psy_visual_stimulus_get_nth_frame(PsyVisualStimulus *self);

G_MODULE_EXPORT void
psy_visual_stimulus_emit_update(PsyVisualStimulus *stim,
                                PsyTimePoint      *frame_time,
                                gint64             nth_frame);

G_MODULE_EXPORT gboolean
psy_visual_stimulus_is_scheduled(PsyVisualStimulus *stimulus);

G_MODULE_EXPORT void
psy_visual_stimulus_set_start_frame(PsyVisualStimulus *self, gint64 frame_num);

G_MODULE_EXPORT gint64
psy_visual_stimulus_get_start_frame(PsyVisualStimulus *self);

G_MODULE_EXPORT gfloat
psy_visual_stimulus_get_x(PsyVisualStimulus *self);
G_MODULE_EXPORT void
psy_visual_stimulus_set_x(PsyVisualStimulus *self, gfloat x);

G_MODULE_EXPORT gfloat
psy_visual_stimulus_get_y(PsyVisualStimulus *self);
G_MODULE_EXPORT void
psy_visual_stimulus_set_y(PsyVisualStimulus *self, gfloat y);

G_MODULE_EXPORT gfloat
psy_visual_stimulus_get_z(PsyVisualStimulus *self);

G_MODULE_EXPORT void
psy_visual_stimulus_set_z(PsyVisualStimulus *self, gfloat z);

G_MODULE_EXPORT gfloat
psy_visual_stimulus_get_scale_x(PsyVisualStimulus *self);
G_MODULE_EXPORT void
psy_visual_stimulus_set_scale_x(PsyVisualStimulus *self, gfloat x);

G_MODULE_EXPORT gfloat
psy_visual_stimulus_get_scale_y(PsyVisualStimulus *self);
G_MODULE_EXPORT void
psy_visual_stimulus_set_scale_y(PsyVisualStimulus *self, gfloat y);

G_MODULE_EXPORT gfloat
psy_visual_stimulus_get_rotation(PsyVisualStimulus *self);
G_MODULE_EXPORT void
psy_visual_stimulus_set_rotation(PsyVisualStimulus *self, gfloat rotation);

G_MODULE_EXPORT PsyColor *
psy_visual_stimulus_get_color(PsyVisualStimulus *self);

G_MODULE_EXPORT void
psy_visual_stimulus_set_color(PsyVisualStimulus *self, PsyColor *color);

G_END_DECLS
