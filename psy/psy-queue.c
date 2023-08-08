
#include "psy-queue.h"

#include <stdatomic.h>

typedef struct PsySpinLock {
    atomic_flag lock;
} PsySpinLock;

void
psy_spin_lock_lock(PsySpinLock *lock)
{
    while (atomic_flag_test_and_set_explicit(&lock->lock, memory_order_acquire))
        ;
}

void
psy_spin_lock_release(PsySpinLock *lock)
{
    atomic_flag_clear(&lock->lock);
}

/**
 * PsyAudioQueue:(skip)
 *
 * This queue is used to queue audio inside of psylib.
 * Stability: private
 */

struct PsyAudioQueue {
    PsySpinLock lock;
    gint        capacity;
    gint        size;
    gint        imask; // mask for valid indices.
    gint        read_index, write_index;
    gfloat     *samples;
};

/**
 * psy_audio_queue_new:(skip)
 *
 * Allocates a new queue with a capacity that is a power of two. The
 * returned value should be freed with [method@Psy.AudioQueue.free].
 *
 * Returns: A new queue, that is empty.
 * Stability: private
 */
PsyAudioQueue *
psy_audio_queue_new(gint num_samples)
{
    if (num_samples >= (1 << 30) - 1) {
        g_critical("Queue: to many sample and/or channels requested.");
        return NULL;
    }

    gint power_of_2 = 1;
    while (power_of_2 < (gint) num_samples)
        power_of_2 <<= 1;

    PsyAudioQueue *queue = g_slice_new(PsyAudioQueue);
    queue->capacity      = power_of_2;
    queue->imask         = queue->capacity - 1;
    queue->read_index    = 0;
    queue->write_index   = 0;
    queue->samples       = g_malloc(sizeof(float) * queue->capacity);
    queue->lock          = (PsySpinLock){.lock = ATOMIC_FLAG_INIT};

    g_assert(queue->capacity >= (int) num_samples);

    return queue;
}

/**
 * psy_audio_queue_free:
 *
 * Free's an instance of [struct@AudioQueue]
 *
 * Stability: private
 */
void
psy_audio_queue_free(PsyAudioQueue *self)
{
    g_free(self->samples);
    g_slice_free(PsyAudioQueue, self);
}

/**
 * psy_audio_queue_size_priv:(skip)
 *
 * Returns: the number of samples hold in the queue in total (sum of samples,
 * for each channel).
 *
 * Stability: private
 */
gint
psy_audio_queue_size_priv(PsyAudioQueue *self)
{
    return self->size;
}

/**
 * psy_audio_queue_size:(skip)
 */
gint
psy_audio_queue_size(PsyAudioQueue *self)
{
    psy_spin_lock_lock(&self->lock);
    gint size = psy_audio_queue_size_priv(self);
    psy_spin_lock_release(&self->lock);
    return size;
}

/**
 * psy_audio_queue_capacity:
 *
 * Returns: The capacity of the audio queue, should always be a power of 2;
 * Stability: private
 */
gint
psy_audio_queue_capacity(PsyAudioQueue *self)
{
    g_return_val_if_fail(self, -1);
    return self->capacity;
}

/**
 * psy_audio_queue_pop_samples:(skip)
 * @self: the instance of the queue
 * @num_samples: the number for samples to pop from the queue
 * @samples:(out): The output array of samples allocated by the user.
 *                 It should be at least big enough to contain num_samples
 *
 * Pops samples from the queue, this should push num_samples samples from the
 * queue This is a locking operation, but it should be done very quickly.
 *
 * Returns: PSY_QUEUE_OK when successfull, PSY_QUEUE_EMPTY when not enough
 *          samples have been pushed onto the queue.
 * Stability: private
 */
PsyQueueStatus
psy_audio_queue_pop_samples(PsyAudioQueue *self,
                            gint           num_samples,
                            gfloat        *samples)
{
    PsyQueueStatus ret = PSY_QUEUE_OK;

    psy_spin_lock_lock(&self->lock);
    if (num_samples > psy_audio_queue_size_priv(self)) {
        ret = PSY_QUEUE_EMPTY;
        goto failure;
    }

    gfloat *outptr = samples;
    for (gint i = 0; i < num_samples; i++) {
        *outptr++ = self->samples[self->read_index++];
        self->read_index &= self->imask;
    }

    self->size -= num_samples;

failure:

    psy_spin_lock_release(&self->lock);

    return ret;
}

/**
 * psy_audio_queue_push_samples:(skip)
 * @self: the instance of the queue
 * @num_samples: the number for samples to pop from the queue
 * @samples:(in): The input array of samples from the user.
 *              It should be at least big enough to contain num_samples
 *
 * Pushes samples @num_samples onto the queue. This is a locking operation, but
 * it should be done very quickly. As it might be bad for the audio callback
 * to block on this function.
 *
 * Returns: PSY_QUEUE_OK when successful, PSY_QUEUE_FULL when not enough
 *          room for the samples is available
 * Stability: private
 */
PsyQueueStatus
psy_audio_queue_push_samples(PsyAudioQueue *self,
                             gint           num_samples,
                             const gfloat  *samples)
{
    PsyQueueStatus ret = PSY_QUEUE_OK;

    psy_spin_lock_lock(&self->lock);

    if (psy_audio_queue_size_priv(self) + num_samples > self->capacity) {
        ret = PSY_QUEUE_FULL;
        goto failure;
    }

    const gfloat *in_ptr = samples;
    for (gint i = 0; i < num_samples; i++) {
        self->samples[self->write_index++] = *in_ptr++;
        self->write_index &= self->imask;
    }

    self->size += num_samples;

failure:

    psy_spin_lock_release(&self->lock);

    return ret;
}
