
#include <stdlib.h>

#include "frame-stats.h"
#include "stimulus.h"

Stimulus *
stimulus_new(PsyDuration        *frame_dur,
             stim_scheduled_func scheduled,
             stim_finished_func  finished,
             void               *data)
{
    Stimulus *self = calloc(1, sizeof(Stimulus));
    if (!self)
        abort();

    self->start_frame = -1;
    self->num_frames  = -1;

    self->frame_dur = psy_duration_copy(frame_dur);

    self->finished  = finished;
    self->scheduled = scheduled;

    self->data = data;

    return self;
}

void
stimulus_free(Stimulus *stim)
{
    if (!stim)
        return;

    g_clear_pointer(&stim->onset, psy_time_point_free);
    g_clear_pointer(&stim->dur, psy_duration_free);
    g_clear_pointer(&stim->frame_dur, psy_duration_free);

    free(stim);
}

void
stimulus_schedule(Stimulus         *self,
                  const FrameStats *stats,
                  PsyTimePoint     *start,
                  PsyDuration      *dur)
{
    PsyDuration *wait_dur
        = psy_time_point_subtract(start, stats->last_frame_time);

    int64_t num_frames = psy_duration_divide_rounded(wait_dur, self->frame_dur);

    self->start_frame = stats->last_known_frame + num_frames;

    self->num_frames = psy_duration_divide_rounded(dur, self->frame_dur);

    psy_duration_free(wait_dur);
}
