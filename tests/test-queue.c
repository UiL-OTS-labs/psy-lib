
#include <CUnit/CUnit.h>
#include <psy-queue.h>
#include <threads.h>

static void
queue_create(void)
{
    PsyAudioQueue *queue = psy_audio_queue_new(1234);

    gfloat some_float;

    CU_ASSERT_PTR_NOT_NULL_FATAL(queue);
    CU_ASSERT_EQUAL(psy_audio_queue_size(queue), 0);
    CU_ASSERT_EQUAL(psy_audio_queue_pop_samples(queue, 1, &some_float), 0);
    CU_ASSERT_EQUAL(psy_audio_queue_capacity(queue), 2048);

    psy_audio_queue_free(queue);
}

static void
queue_push_pop(void)
{
    float          input[2048];
    float          output[2048];
    PsyAudioQueue *queue = psy_audio_queue_new(1234);
    CU_ASSERT_PTR_NOT_NULL_FATAL(queue);

    for (int i = 0; i < 2048; i++) {
        input[i] = 2048.0 * 1.0 / 2048;
    }

    gsize status = psy_audio_queue_push_samples(queue, 1234, input);
    CU_ASSERT_EQUAL(status, 1234);

    status = psy_audio_queue_push_samples(queue, 1, &input[0]);
    CU_ASSERT_EQUAL(status, 0);

    status = psy_audio_queue_pop_samples(queue, 1234, output);
    CU_ASSERT_EQUAL(status, 1234);

    status = psy_audio_queue_pop_samples(queue, 1, &output[0]);
    CU_ASSERT_EQUAL(status, 0);

    psy_audio_queue_free(queue);
}

typedef struct PushPullContext {

    float *data_in;
    float *data_out;

    gsize num_samples;

    PsyAudioQueue *queue;
} PushPullContext;

static int
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
        n = psy_audio_queue_push_samples(q, 100, &data_in[num_send]);
        num_send += n;
    }
    g_info("push thread stopping");

    return 0;
}

static int
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
        n = psy_audio_queue_pop_samples(q, 100, &data_out[num_received]);
        num_received += n;
    }

    g_info("pull thread stopping");
    return 0;
}

static void
queue_simultaneous_push_pull(void)
{
    int status;

    thrd_t push_thread;
    thrd_t pull_thread;

    PushPullContext context = {.data_in     = NULL,
                               .data_out    = NULL,
                               .num_samples = 1000000,
                               .queue       = psy_audio_queue_new(1024)};

    context.data_in  = malloc(context.num_samples * sizeof(float));
    context.data_out = malloc(context.num_samples * sizeof(float));

    for (size_t i = 0; i < context.num_samples; i++)
        context.data_in[i] = i;

    memset(context.data_out, 0, context.num_samples * sizeof(float));

    g_info("starting threads");
    status = thrd_create(&push_thread, push_samples, &context);
    g_assert(status == thrd_success);
    status = thrd_create(&pull_thread, pull_samples, &context);
    g_assert(status == thrd_success);
    g_info("Threads started");

    thrd_join(push_thread, NULL);
    thrd_join(pull_thread, NULL);

    g_info("Threads are joined.");

    CU_ASSERT_TRUE(memcmp(context.data_in,
                          context.data_out,
                          context.num_samples * sizeof(float))
                   == 0);

    psy_audio_queue_free(context.queue);
    free(context.data_in);
    free(context.data_out);
}

int
add_queue_suite(void)
{
    CU_Suite *suite = CU_add_suite("queue tests", NULL, NULL);
    CU_Test  *test  = NULL;

    if (!suite)
        return 1;

    test = CU_ADD_TEST(suite, queue_create);
    if (!test)
        return 1;

    test = CU_ADD_TEST(suite, queue_push_pop);
    if (!test)
        return 1;

    test = CU_ADD_TEST(suite, queue_simultaneous_push_pull);
    if (!test)
        return 1;

    return 0;
}
