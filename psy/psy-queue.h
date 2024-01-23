
#pragma once

#include <gio/gio.h>
#include <glib.h>

#include "psy-config.h"

G_BEGIN_DECLS

typedef struct PsyAudioQueue PsyAudioQueue;

G_MODULE_EXPORT PsyAudioQueue *
psy_audio_queue_new(guint num_samples);

G_MODULE_EXPORT void
psy_audio_queue_free(PsyAudioQueue *self);

G_MODULE_EXPORT guint
psy_audio_queue_size(PsyAudioQueue *self);

G_MODULE_EXPORT guint
psy_audio_queue_capacity(PsyAudioQueue *self);

G_MODULE_EXPORT guint
psy_audio_queue_push_samples(PsyAudioQueue *self,
                             guint          num_samples,
                             const gfloat  *samples);

G_MODULE_EXPORT guint
psy_audio_queue_pop_samples(PsyAudioQueue *self,
                            guint          num_samples,
                            gfloat        *samples);

G_MODULE_EXPORT void
psy_audio_queue_clear(PsyAudioQueue *self);

G_END_DECLS
