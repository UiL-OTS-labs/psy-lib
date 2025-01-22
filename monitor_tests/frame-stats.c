
#include <stdlib.h>

#include "frame-stats.h"

FrameStats *
frame_stats_new(void)
{
    FrameStats *stats = calloc(1, sizeof(FrameStats));
    if (!stats)
        abort();

    stats->nth_frame = -1;

    return stats;
}

void
frame_stats_free(FrameStats *stats)
{
    if (stats->last_frame_time)
        psy_time_point_free(stats->last_frame_time);

    free(stats);
}

void
update_frame_stats(FrameStats   *self,
                   int64_t       n,
                   int64_t       n_missed_frames,
                   PsyTimePoint *tp_next_frame)
{
    if (!self) {
        g_critical("Self pointer is NULL");
        return;
    }

    self->last_known_frame = self->nth_frame + 1;

    self->nth_frame += (n + n_missed_frames);
    self->n_missed_frames += n_missed_frames;

    if (self->last_frame_time)
        psy_time_point_free(self->last_frame_time);

    self->last_frame_time = tp_next_frame;
}
