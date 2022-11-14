
#pragma once

#include <glib-object.h>
#include <gio/gio.h>
#include "psy-time-point.h"

G_BEGIN_DECLS

#define PSY_TYPE_STIMULUS psy_stimulus_get_type()
G_DECLARE_DERIVABLE_TYPE(PsyStimulus, psy_stimulus, PSY, STIMULUS, GObject)

typedef struct _PsyStimulusClass {
    GObjectClass parent;
    void (*play) (PsyStimulus* self, PsyTimePoint *start_time);
    void (*set_duration) (PsyStimulus* self, PsyDuration* duration);
    void (*started) (PsyStimulus* self, PsyTimePoint *start_time);
    void (*stopped) (PsyStimulus* self, PsyTimePoint *stop_time);
} PsyStimulusClass;

G_MODULE_EXPORT void
psy_stimulus_play(PsyStimulus* self, PsyTimePoint *start_time);

G_MODULE_EXPORT void
psy_stimulus_stop(PsyStimulus* self, PsyTimePoint *stop_time);

G_MODULE_EXPORT void
psy_stimulus_play_for(PsyStimulus   *self,
                      PsyTimePoint  *start_time,
                      PsyDuration   *duration);

G_MODULE_EXPORT void
psy_stimulus_play_until(PsyStimulus     *self,
                        PsyTimePoint    *start_time,
                        PsyTimePoint    *stop_time);

G_MODULE_EXPORT void
psy_stimulus_set_duration(PsyStimulus* self, PsyDuration *duration);

G_MODULE_EXPORT PsyDuration*
psy_stimulus_get_duration(PsyStimulus* self);

G_MODULE_EXPORT PsyTimePoint*
psy_stimulus_get_start_time(PsyStimulus* self);

G_MODULE_EXPORT PsyTimePoint*
psy_stimulus_get_stop_time(PsyStimulus* self);

G_MODULE_EXPORT gboolean
psy_stimulus_get_is_started(PsyStimulus* self);

G_MODULE_EXPORT void
psy_stimulus_set_is_started(PsyStimulus* self, PsyTimePoint* start_time);

G_MODULE_EXPORT gboolean
psy_stimulus_get_is_finished(PsyStimulus* self);

G_MODULE_EXPORT void
psy_stimulus_set_is_finished(PsyStimulus* self, PsyTimePoint* stop_time);

G_END_DECLS
