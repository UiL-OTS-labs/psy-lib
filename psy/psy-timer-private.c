

#include "psy-timer-private.h"
#include "psy-clock.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <timeapi.h>
#endif

/* *********** globals *************** */

static GMutex g_thread_mutex;

static PsyTimerThread *g_timer_thread;

// if start count = 1 start thread, if start count = 0
static gint g_start_count;

/* ************** forward declarations ************** */

// The thread
static gpointer timer_thread(gpointer);

static void
psy_timer_thread_join(PsyTimerThread *self);

typedef enum TimerThreadMessage {
    MSG_STOP,      // Stops the mainloop
    MSG_TIMER_ADD, // Adds a timer to the thread
    MSG_TIMER_DEL, // Deletes a timer from the thread
} TimerThreadMessage;

typedef struct ThreadData {
    TimerThreadMessage msg;
    PsyTimerThread    *self;  // not owned.
    PsyTimer          *timer; // not owned.
} ThreadData;

typedef struct TimePointTimerPair {
    PsyTimePoint *tp;    // not owned
    PsyTimer     *timer; // not owned
} TimePointTimerPair;

/* ************** utility functions ***************** */

static gint
compare_pairs(gconstpointer pair1, gconstpointer pair2)
{
    const TimePointTimerPair *p1 = pair1;
    const TimePointTimerPair *p2 = pair2;

    int cmp = psy_compare_time_point(p1->tp, p2->tp);
    if (cmp != 0)
        return cmp;

    if (p1->timer < p2->timer)
        return -1;
    else if (p1->timer > p2->timer)
        return 1;
    else
        return 0;
}

static ThreadData *
thread_data_new(TimerThreadMessage msg, PsyTimerThread *self, PsyTimer *timer)
{
    g_warn_if_fail(self != NULL);

    ThreadData *data = g_malloc(sizeof(ThreadData));
    data->msg        = msg;
    data->self       = self;
    data->timer      = timer;

    return data;
}

static void
thread_data_free(ThreadData *data)
{
    g_free(data);
}

/* ************** private functions ***************** */

typedef struct _PsyTimerThread {
    GObject       parent;
    GMainContext *context;
    GMainLoop    *loop;
    GArray       *pairs;
    GThread      *thread;
    PsyClock     *clock;
    PsyDuration  *busy_loop_dur;
} PsyTimerThread;

G_DEFINE_TYPE(PsyTimerThread, psy_timer_thread, G_TYPE_OBJECT)

static void
psy_timer_thread_init(PsyTimerThread *self)
{
    self->context       = g_main_context_new();
    self->pairs         = g_array_new(FALSE, TRUE, sizeof(TimePointTimerPair));
    self->thread        = g_thread_new("TimerThread", timer_thread, self);
    self->clock         = psy_clock_new();
    self->busy_loop_dur = psy_duration_new_ms(2);
#ifdef _WIN32
    int ret = timeBeginPeriod(1);
    g_assert(ret == TIMERR_NOERROR);
    if (ret == TIMERR_NOCANDO) {
        g_critical("Unable to set timeBeginPeriod(1): "
                   "timers might have a low resolution.");
    }
#endif
}

static void
psy_timer_thread_dispose(GObject *self)
{
    PsyTimerThread *tt_self = PSY_TIMER_THREAD(self);

    psy_timer_thread_join(tt_self);

    g_main_context_unref(tt_self->context);
    tt_self->context = NULL;

    g_clear_object(&tt_self->clock);

    G_OBJECT_CLASS(psy_timer_thread_parent_class)->dispose(self);
}

static void
psy_timer_thread_finalize(GObject *self)
{
    PsyTimerThread *tt_self = PSY_TIMER_THREAD(self);
    g_array_free(tt_self->pairs, TRUE);

    psy_duration_free(tt_self->busy_loop_dur);
#ifdef _WIN32
    timeEndPeriod(1);
#endif

    G_OBJECT_CLASS(psy_timer_thread_parent_class)->finalize(self);
}

static gboolean
psy_timer_thread_stop(ThreadData *data)
{
    PsyTimerThread *self = data->self;

    g_main_loop_quit(self->loop);

    return G_SOURCE_REMOVE;
}

static gboolean
psy_timer_thread_add_timer(ThreadData *data)
{
    PsyTimerThread *self = data->self;

    PsyTimePoint *tp = psy_timer_get_fire_time(data->timer);

    TimePointTimerPair pair
        = {.timer = data->timer, .tp = psy_time_point_copy(tp)};

    g_array_append_val(self->pairs, pair);

    g_array_sort(self->pairs, compare_pairs);

    return G_SOURCE_REMOVE;
}

static gboolean
psy_timer_thread_del_timer(ThreadData *data)
{
    PsyTimerThread *self = data->self;

    guint    index = -1;
    gboolean found = FALSE;

    for (guint i = 0; i < self->pairs->len; i++) {
        TimePointTimerPair *pairs = (TimePointTimerPair *) self->pairs->data;
        if (pairs[i].timer == data->timer) {
            found = TRUE;
            index = i;
            psy_time_point_free(pairs[i].tp);
            break;
        }
    }

    if (found) {
        g_array_remove_index(self->pairs, index);
    }

    return G_SOURCE_REMOVE;
}

void
psy_timer_thread_class_init(PsyTimerThreadClass *klass)
{
    GObjectClass *obj_class = G_OBJECT_CLASS(klass);

    obj_class->dispose  = psy_timer_thread_dispose;
    obj_class->finalize = psy_timer_thread_finalize;
}

static void
psy_timer_thread_run_busy_loop(PsyTimerThread *self, TimePointTimerPair pair)
{
    PsyTimePoint *now = NULL;
    while (1) {
        now = psy_clock_now(self->clock);
        if (psy_time_point_greater_equal(now, pair.tp)) {
            psy_timer_fire(pair.timer, pair.tp);
            psy_timer_cancel(pair.timer);
            psy_time_point_free(now);
            break;
        }
        psy_time_point_free(now);
        g_thread_yield();
    }
}

static gboolean
psy_timer_thread_check_timers(gpointer data)
{
    PsyTimerThread *self = data;
    PsyTimePoint   *now  = psy_clock_now(self->clock);

    while (self->pairs->len > 0) {

        TimePointTimerPair *pair
            = &g_array_index(self->pairs, TimePointTimerPair, 0);

        PsyDuration *dur = psy_time_point_subtract(pair->tp, now);

        if (psy_duration_less_equal(dur, self->busy_loop_dur)) {
            psy_timer_thread_run_busy_loop(self, *pair);
            psy_duration_free(dur);
        }
        else {
            psy_duration_free(dur);
            break;
        }
    }
    psy_time_point_free(now);

    return G_SOURCE_CONTINUE;
}

// the thread
static gpointer
timer_thread(gpointer data)
{
    PsyTimerThread *self = data;
    g_main_context_push_thread_default(self->context);

    self->loop = g_main_loop_new(self->context, FALSE);

    GSource *timeout = g_timeout_source_new(1);
    g_source_set_callback(timeout, psy_timer_thread_check_timers, self, NULL);
    g_source_attach(timeout, self->context);
    g_source_unref(timeout);

    g_main_loop_run(self->loop);
    g_main_loop_unref(self->loop);

    g_main_context_pop_thread_default(self->context);

    return data;
}

static void
psy_timer_thread_join(PsyTimerThread *self)
{
    if (!self->thread)
        return;

    // Send stop message
    ThreadData *data = thread_data_new(MSG_STOP, self, NULL);

    g_main_context_invoke_full(self->context,
                               G_PRIORITY_DEFAULT,
                               G_SOURCE_FUNC(psy_timer_thread_stop),
                               data,
                               (GDestroyNotify) thread_data_free);

    g_thread_join(self->thread);
    self->thread = NULL;
}

static GMainContext *
psy_timer_thread_get_context(PsyTimerThread *self)
{
    g_return_val_if_fail(PSY_IS_TIMER_THREAD(self), NULL);

    return self->context;
}

// Call this function only if g_thread_mutex is locked
static void
timer_thread_start(void)
{
    g_assert(g_start_count >= 0);
    g_start_count++;
    if (g_start_count == 1) {
        g_info("Starting timer thread");
        g_timer_thread = g_object_new(PSY_TYPE_TIMER_THREAD, NULL);
    }
}

// Call this function only if g_thread_mutex is locked
static void
timer_thread_stop(void)
{
    g_start_count--;
    g_assert(g_start_count >= 0);

    if (g_start_count == 0) {
        g_info("Stopping timer thread");

        psy_timer_thread_join(g_timer_thread);
        g_clear_object(&g_timer_thread);
    }
}

/* **** non static functions *** */

void
timer_private_start_timer_thread(void)
{
    g_mutex_lock(&g_thread_mutex);
    timer_thread_start();
    g_mutex_unlock(&g_thread_mutex);
}

void
timer_private_stop_timer_thread(void)
{
    g_mutex_lock(&g_thread_mutex);
    timer_thread_stop();
    g_mutex_unlock(&g_thread_mutex);
}

/**
 * timer_private_set_timer:
 * @timer:(transfer none): a timer that needs to be signalled
 *
 * Add a timer to the timer thread to have it signalled in time when
 * possible
 *
 * Stability:Private
 */
void
timer_private_set_timer(PsyTimer *timer)
{
    ThreadData *data = thread_data_new(MSG_TIMER_ADD, g_timer_thread, timer);
    g_main_context_invoke_full(psy_timer_thread_get_context(g_timer_thread),
                               G_PRIORITY_DEFAULT,
                               G_SOURCE_FUNC(psy_timer_thread_add_timer),
                               data,
                               (GDestroyNotify) thread_data_free);
}

/**
 * timer_private_cancel_timer:
 * @timer:(transfer none): This timer is looked up and removed if it is
 * contained in the timer thread.
 *
 * Removes a timer from the thread
 *
 * Stability:Private
 */
void
timer_private_cancel_timer(PsyTimer *timer)
{
    ThreadData *data = thread_data_new(MSG_TIMER_DEL, g_timer_thread, timer);

    g_main_context_invoke_full(psy_timer_thread_get_context(g_timer_thread),
                               G_PRIORITY_DEFAULT,
                               G_SOURCE_FUNC(psy_timer_thread_del_timer),
                               data,
                               (GDestroyNotify) thread_data_free);
}
