
#include <criterion/criterion.h>
#include <criterion/new/assert.h>
#include <psy-queue.h>

Test(queue, create)
{
    PsyAudioQueue *queue = psy_audio_queue_new(1234);

    gfloat some_float;

    cr_assert(ne(queue, NULL));
    cr_expect(eq(psy_audio_queue_size(queue), 0u));
    cr_expect(eq(psy_audio_queue_pop_samples(queue, 1, &some_float), 0u));
    cr_expect(eq(psy_audio_queue_capacity(queue), 1234u));

    psy_audio_queue_free(queue);
}

Test(queue, push_pop)
{
    float          input[2048];
    float          output[2048];
    PsyAudioQueue *queue = psy_audio_queue_new(1234);
    cr_assert(ne(queue, NULL));

    for (int i = 0; i < 2048; i++) {
        input[i] = 2048.0 * 1.0 / 2048;
    }

    gsize status = psy_audio_queue_push_samples(queue, 1234, input);
    cr_assert(eq(status, 1234u));

    status = psy_audio_queue_push_samples(queue, 1, &input[0]);
    cr_expect(eq(status, 0u));

    status = psy_audio_queue_pop_samples(queue, 1234, output);
    cr_assert(eq(status, 1234u));

    status = psy_audio_queue_pop_samples(queue, 1, &output[0]);
    cr_assert(eq(status, 0u));

    psy_audio_queue_free(queue);
}

Test(queue, clear)
{
    float          sample = .5;
    PsyAudioQueue *queue  = psy_audio_queue_new(16);

    psy_audio_queue_push_samples(queue, 1, &sample);

    cr_expect(eq(psy_audio_queue_size(queue), 1u));

    psy_audio_queue_push_samples(queue, 1, &sample);
    psy_audio_queue_push_samples(queue, 1, &sample);
    psy_audio_queue_push_samples(queue, 1, &sample);

    cr_expect(eq(psy_audio_queue_size(queue), 4u));

    psy_audio_queue_clear(queue);

    cr_expect(eq(psy_audio_queue_size(queue), 0u));

    psy_audio_queue_free(queue);
}

typedef struct PushPullContext {

    float *data_in;
    float *data_out;

    gsize num_samples;

    PsyAudioQueue *queue;
} PushPullContext;

static void *
push_samples(gpointer data)
{
    gsize n;

    size_t           num_send = 0;
    PushPullContext *context  = data;

    // aliases
    PsyAudioQueue *q       = context->queue;
    const float   *data_in = context->data_in;

    g_info("push thread started");
    while (num_send < context->num_samples) {
        n = psy_audio_queue_push_samples(q, 1, &data_in[num_send]);
        num_send += n;
    }
    g_info("push thread stopping");

    return NULL;
}

static void *
pull_samples(gpointer data)
{
    gsize n;

    size_t           num_received = 0;
    PushPullContext *context      = data;

    // aliases
    PsyAudioQueue *q        = context->queue;
    float         *data_out = context->data_out;

    g_info("pull thread started");

    while (num_received < context->num_samples) {
        n = psy_audio_queue_pop_samples(q, 1, &data_out[num_received]);
        num_received += n;
    }

    g_info("pull thread stopping");
    return NULL;
}

Test(queue, simultaneous_push_pull)
{
    GThread *push_thread = NULL;
    GThread *pull_thread = NULL;

    PushPullContext context = {.data_in     = NULL,
                               .data_out    = NULL,
                               .num_samples = 10000,
                               .queue       = psy_audio_queue_new(1024)};

    context.data_in  = malloc(context.num_samples * sizeof(float));
    context.data_out = malloc(context.num_samples * sizeof(float));

    for (size_t i = 0; i < context.num_samples; i++)
        context.data_in[i] = (float) i;

    memset(context.data_out, 0, context.num_samples * sizeof(float));

    g_info("starting threads");
    push_thread = g_thread_new("push_thread", push_samples, &context);
    g_assert(push_thread != NULL);
    pull_thread = g_thread_new("pull_thread", pull_samples, &context);
    g_assert(pull_thread != NULL);
    g_info("Threads started");

    g_thread_join(push_thread);
    g_thread_join(pull_thread);

    g_info("Threads are joined.");

    cr_assert(eq(memcmp(context.data_in,
                        context.data_out,
                        context.num_samples * sizeof(float)),
                 0));

    psy_audio_queue_free(context.queue);
    free(context.data_in);
    free(context.data_out);
}
