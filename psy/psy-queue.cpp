

#include "psy-config.h"

#if defined(HAVE_BOOST_LOCKFREE_SPSC_QUEUE_HPP)

    #include "psy-queue.h"
    #include <boost/lockfree/spsc_queue.hpp>

/**
 * PsyAudioQueue:(skip)
 *
 * This queue is used to queue audio inside of psylib.
 * Stability: private
 */

struct PsyAudioQueue {
    boost::lockfree::spsc_queue<gfloat> *p_queue;
    gsize                                capacity;
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
    PsyAudioQueue *queue
        = static_cast<PsyAudioQueue *>(g_malloc(sizeof(PsyAudioQueue)));

    if (!queue)
        return queue;

    try {
        queue->p_queue = new boost::lockfree::spsc_queue<gfloat>(num_samples);
    } catch (std::bad_alloc& exception) {
        g_critical(
            "Unable to alloc boost::lockfree::spsc_queue<gfloat>(%d): %s",
            num_samples,
            exception.what());
        g_free(queue);
        queue = NULL;
    } catch (std::exception& exception) {
        g_critical(
            "Unable to alloc boost::lockfree::spsc_queue<gfloat>(%d): %s",
            num_samples,
            exception.what());
        if (queue->p_queue)
            delete queue->p_queue;
        g_free(queue);
        queue = NULL;
    }

    queue->capacity = queue->p_queue->write_available();

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
    delete self->p_queue;
    g_free(self);
}

/**
 * psy_audio_queue_size:(skip)
 *
 * Returns: the number of samples hold in the queue in total (sum of
 * samples, for each channel).
 *
 */
gsize
psy_audio_queue_size(PsyAudioQueue *self)
{
    g_return_val_if_fail(self, -1);
    return self->p_queue->read_available();
}

/**
 * psy_audio_queue_capacity:(skip)
 *
 * Returns: The capacity of the audio queue, should always be a power of 2;
 * Stability: private
 */
gsize
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
 * Pops samples from the queue, this should push num_samples samples from
 * the queue, however, the return value should tell how many samples have
 * actually been popped.
 *
 * Returns: The number of samples popped from the queue.
 *
 * Stability: private
 */
gsize
psy_audio_queue_pop_samples(PsyAudioQueue *self,
                            guint          num_samples,
                            gfloat        *samples)
{
    g_return_val_if_fail(self, -1);
    g_return_val_if_fail(samples, -1);

    return self->p_queue->pop(samples, num_samples);
}

/**
 * psy_audio_queue_push_samples:(skip)
 * @self: the instance of the queue
 * @num_samples: the number for samples to pop from the queue
 * @samples:(in): The input array of samples from the user.
 *                It should be at least big enough to contain num_samples
 *
 * Pushes samples @num_samples onto the queue. This is a locking operation,
 * but it should be done very quickly. As it might be bad for the audio
 * callback to block on this function.
 *
 * Returns: The number of samples successfully transmitted to the queue
 * Stability: private
 */
gsize
psy_audio_queue_push_samples(PsyAudioQueue *self,
                             guint          num_samples,
                             const gfloat  *samples)
{
    g_return_val_if_fail(self, -1);
    g_return_val_if_fail(samples, -1);

    return self->p_queue->push(samples, num_samples);
}

#endif
