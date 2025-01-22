
#pragma once

#include <psylib.h>

G_BEGIN_DECLS

typedef struct FrameStats FrameStats;

struct FrameStats {
    int64_t nth_frame;        // nth_frame to be presented starts a -1
    int64_t n_missed_frames;  // the number of missed frames
    int64_t last_known_frame; // the number of the last known frame.

    PsyTimePoint *last_frame_time;
};

FrameStats *
frame_stats_new(void);

void
frame_stats_free(FrameStats *stats);

void
update_frame_stats(FrameStats   *self,
                   int64_t       n,
                   int64_t       n_missed_frames,
                   PsyTimePoint *tp_next_frame);

G_END_DECLS
