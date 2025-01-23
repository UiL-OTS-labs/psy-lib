
#pragma once

#include <psylib.h>
#include <stdint.h>

#include "frame-stats.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Stimulus Stimulus;

typedef void (*stim_present_func)(void *);
typedef void (*stim_scheduled_func)(Stimulus *stim, void *);
typedef void (*stim_finished_func)(Stimulus *stim, void *);

struct Stimulus {

    int64_t start_frame; // The number of the frame this stimulus should start
    int64_t num_frames;  // The number of frames this stimulus last

    PsyTimePoint *onset;
    PsyDuration  *dur;

    PsyDuration *frame_dur;

    stim_present_func   present;
    stim_scheduled_func scheduled;
    stim_finished_func  finished;

    void *data;
};

Stimulus *
stimulus_new(PsyDuration        *frame_dur,
             stim_scheduled_func scheduled,
             stim_present_func   present,
             stim_finished_func  finished,
             void               *data);

void
stimulus_free(Stimulus *stim);

void
stimulus_schedule(Stimulus         *self,
                  const FrameStats *stats,
                  PsyTimePoint     *start,
                  PsyDuration      *dur);

void
stimulus_present(Stimulus *self, const FrameStats *stats);

#ifdef __cplusplus
}
#endif
