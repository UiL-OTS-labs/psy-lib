
#pragma once

#include "psy-auditory-stimulus.h"

G_BEGIN_DECLS

typedef struct PsyAudioQueue PsyAudioQueue;

typedef enum QueueStatus {
    PSY_QUEUE_OK,
    PSY_QUEUE_FULL,
    PSY_QUEUE_EMPTY,
} PsyQueueStatus;

G_MODULE_EXPORT PsyAudioQueue *
psy_audio_queue_new(gint num_samples);

G_MODULE_EXPORT void
psy_audio_queue_free(PsyAudioQueue *self);

G_MODULE_EXPORT gint
psy_audio_queue_size(PsyAudioQueue *self);

G_MODULE_EXPORT gint
psy_audio_queue_capacity(PsyAudioQueue *self);

G_MODULE_EXPORT PsyQueueStatus
psy_audio_queue_push_samples(PsyAudioQueue *self,
                             gint           num_samples,
                             const gfloat  *samples);

G_MODULE_EXPORT PsyQueueStatus
psy_audio_queue_pop_samples(PsyAudioQueue *self,
                            gint           num_samples,
                            gfloat        *samples);

G_END_DECLS
