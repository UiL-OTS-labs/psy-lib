
#pragma once

#include <gio/gio.h>
#include <glib.h>

#include "psy-config.h"

#if defined(HAVE_BOOST_LOCKFREE_SPSC_QUEUE_HPP)

G_BEGIN_DECLS

typedef struct PsyAudioQueue PsyAudioQueue;

G_MODULE_EXPORT PsyAudioQueue *
psy_audio_queue_new(gsize num_samples);

G_MODULE_EXPORT void
psy_audio_queue_free(PsyAudioQueue *self);

G_MODULE_EXPORT gsize
psy_audio_queue_size(PsyAudioQueue *self);

G_MODULE_EXPORT gsize
psy_audio_queue_capacity(PsyAudioQueue *self);

G_MODULE_EXPORT gsize
psy_audio_queue_push_samples(PsyAudioQueue *self,
                             guint          num_samples,
                             const gfloat  *samples);

G_MODULE_EXPORT gsize
psy_audio_queue_pop_samples(PsyAudioQueue *self,
                            guint          num_samples,
                            gfloat        *samples);

G_END_DECLS

#endif // defined HAVE_BOOST_LOCKFREE_SPSC_QUEUE_HPP
