
#pragma once

#include <glib-object.h>
#include <gio/gio.h>
#include "ddd-time-point.h"

G_BEGIN_DECLS

#define DDD_TYPE_STIMULUS ddd_stimulus_get_type()
G_DECLARE_DERIVABLE_TYPE(DddStimulus, ddd_stimulus, DDD, STIMULUS, GObject)

typedef struct _DddStimulusClass {
    GObjectClass parent;
    void (*play) (DddStimulus* self, DddTimePoint *desired_start_time);
    void (*stop) (DddStimulus* self, DddTimePoint *desired_stop_time);
    void (*started) (DddStimulus* self, DddTimePoint *start_time);
    void (*stopped) (DddStimulus* self, DddTimePoint *stop_time);
} DddStimulusClass;

G_MODULE_EXPORT void
ddd_stimulus_play(DddStimulus* self, DddTimePoint *start_time);

G_MODULE_EXPORT void
ddd_stimulus_stop(DddStimulus* self, DddTimePoint *stop_time);

G_MODULE_EXPORT void
ddd_stimulus_play_for(DddStimulus   *self,
                      DddTimePoint  *start_time,
                      DddDuration   *dur);

G_MODULE_EXPORT void
ddd_stimulus_play_until(DddStimulus     *self,
                        DddTimePoint    *start_time,
                        DddTimePoint    *stop_time);

G_MODULE_EXPORT void
ddd_stimulus_set_duration(DddStimulus* self, DddDuration *duration);

G_MODULE_EXPORT DddDuration*
ddd_stimulus_get_duration(DddStimulus* self);

G_MODULE_EXPORT DddTimePoint*
ddd_stimulus_get_start_time(DddStimulus* self);

G_MODULE_EXPORT DddTimePoint*
ddd_stimulus_get_stop_time(DddStimulus* self);

G_END_DECLS
