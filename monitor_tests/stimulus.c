
#include <stdlib.h>

#include "frame-stats.h"
#include "stimulus.h"

Stimulus *
stimulus_new(PsyDuration        *frame_dur,
             stim_scheduled_func scheduled,
             stim_present_func   present,
             stim_finished_func  finished,
             void               *data)
{
    Stimulus *self = calloc(1, sizeof(Stimulus));
    if (!self)
        abort();

    self->start_frame = -1;
    self->num_frames  = -1;

    self->frame_dur = psy_duration_copy(frame_dur);

    self->present   = present;
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
    PsyDuration *wait_frames = NULL;

    int64_t num_frames = psy_duration_divide_rounded(wait_dur, self->frame_dur);
    wait_frames = psy_duration_multiply_scalar(self->frame_dur, num_frames);

    self->start_frame = stats->last_known_frame + num_frames;

    self->num_frames = psy_duration_divide_rounded(dur, self->frame_dur);

    g_clear_pointer(&self->onset, psy_time_point_free);
    self->onset = psy_time_point_add(stats->last_frame_time, wait_frames);

    psy_duration_free(wait_dur);
    psy_duration_free(wait_frames);

    if (self->scheduled)
        self->scheduled(self, self->data);
}

static void
stimulus_finish(Stimulus *self)
{
    self->start_frame = -1;
    self->num_frames  = -1;

    self->finished(self, self->data);
}

static bool
stimulus_is_running(Stimulus *self)
{
    return self->start_frame >= 0 && self->num_frames >= 0;
}

void
stimulus_present(Stimulus *self, const FrameStats *stats)
{
    if (stimulus_is_running(self)) {
        if (self->start_frame <= stats->nth_frame
            && (self->start_frame + self->num_frames) > stats->nth_frame)
            self->present(self->data);

        if (stats->nth_frame >= self->start_frame + self->num_frames)
            stimulus_finish(self);
    }
}
