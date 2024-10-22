
#include "psy-timer-private.h"
#include "psy-clock.h"
#include "psy-config.h"

#ifdef _WIN32
    #include <windows.h>
#endif

/* *********** globals *************** */

static PsyTimerThread *g_timer_thread;
static int             init_warning = 0;

/* ************** forward declarations ************** */

// The thread
static gpointer timer_thread(gpointer);

static void
psy_timer_thread_join(PsyTimerThread *self);

typedef enum TimerThreadMessage {
    MSG_STOP,           // Stop the thread
    MSG_TIMER_ADD,      // Add a timer to the thread
    MSG_TIMER_DEL,      // Deletes a timer from the thread
    MSG_TIMER_CANCELED, // Acknowledge the deletion of the timer
} TimerThreadMessage;

typedef struct ThreadData {
    TimerThreadMessage msg;
    PsyTimerThread    *self;  // not owned.
    PsyTimer          *timer; // not owned.
} ThreadData;

/* ************** utility functions ***************** */

static gint
compare_timer_time_stamps_values(gconstpointer t1, gconstpointer t2)
{
    const PsyTimePoint *tp1 = psy_timer_get_fire_time(PSY_TIMER((gpointer) t1));
    const PsyTimePoint *tp2 = psy_timer_get_fire_time(PSY_TIMER((gpointer) t2));

    return psy_compare_time_point(tp1, tp2);
}

#if !GLIB_CHECK_VERSION(2, 76, 0)
static gint
compare_timer_time_stamps(gconstpointer tpp1, gconstpointer tpp2)
{
    return compare_timer_time_stamps_values(*(PsyTimer **) tpp1,
                                            *(PsyTimer **) tpp2);
}
#endif

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
    GObject parent;

    GAsyncQueue *queue;
    gboolean     running;

    GPtrArray   *timers;
    GThread     *thread;
    PsyClock    *clock;
    PsyDuration *busy_loop_dur;
} PsyTimerThread;

G_DEFINE_TYPE(PsyTimerThread, psy_timer_thread, G_TYPE_OBJECT)

static void
timer_thread_handle_message(PsyTimerThread *self, ThreadData *data);

static void
psy_timer_thread_init(PsyTimerThread *self)
{
    self->queue         = g_async_queue_new();
    self->timers        = g_ptr_array_new_full(64, NULL);
    self->clock         = psy_clock_new();
    self->busy_loop_dur = psy_duration_new_ms(2);
    self->running       = TRUE;
    self->thread        = g_thread_new("TimerThread", timer_thread, self);

#ifdef _WIN32
    // On windows a Sleep(1) should sleep for 1 millisecond. In practice, this
    // can take a bit longer due to OS scheduling, the 1 ms is a minimal amount.
    // The scheduler might finish the current "quantum" for this process. Which
    // can easily be +/- 15 ms. timeBeginPeriod sets the sleep precision a bit
    // higher, at the expense of extra power use.
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

    g_clear_object(&tt_self->clock);

    g_async_queue_unref(tt_self->queue);

    G_OBJECT_CLASS(psy_timer_thread_parent_class)->dispose(self);
}

static void
psy_timer_thread_finalize(GObject *self)
{
    PsyTimerThread *tt_self = PSY_TIMER_THREAD(self);
    g_ptr_array_free(tt_self->timers, TRUE);

    psy_duration_free(tt_self->busy_loop_dur);
#ifdef _WIN32
    timeEndPeriod(1);
#endif

    G_OBJECT_CLASS(psy_timer_thread_parent_class)->finalize(self);
}

static gboolean
psy_timer_thread_add_timer(PsyTimerThread *self, PsyTimer *timer)
{
    g_ptr_array_add(self->timers, timer);

#if GLIB_CHECK_VERSION(2, 76, 0)
    g_ptr_array_sort_values(self->timers, compare_timer_time_stamps_values);
#else
    g_ptr_array_sort(self->timers, compare_timer_time_stamps);
#endif

    return TRUE;
}

static gboolean
psy_timer_thread_del_timer(PsyTimerThread *self, PsyTimer *timer)
{
    GAsyncQueue *reply_queue = psy_timer_get_queue(timer);

    gboolean ret = g_ptr_array_remove(self->timers, timer);
    if (!ret) {
        g_critical("Unable to remove timer %p", (gpointer) timer);
    }

    ThreadData *msg = thread_data_new(MSG_TIMER_CANCELED, self, timer);

    g_async_queue_push(reply_queue, msg);

    return TRUE;
}

void
psy_timer_thread_class_init(PsyTimerThreadClass *klass)
{
    GObjectClass *obj_class = G_OBJECT_CLASS(klass);

    obj_class->dispose  = psy_timer_thread_dispose;
    obj_class->finalize = psy_timer_thread_finalize;
}

/**
 * psy_timer_thread_fire_timers:
 * @data a pointer to self.
 *
 * A loop function that checks whether one or multiple timers
 * are ready to fire. If so, the timers are fired.
 *
 * If there are no timers with a fire time  before now + self->busy_loop_dur
 * the loop is terminated.
 *
 * stability:private
 */
static void
psy_timer_thread_fire_timers(PsyTimerThread *self)
{
    while (self->timers->len > 0 && self->running) {
        PsyTimePoint *now = psy_clock_now(self->clock);
        PsyTimePoint *now_plus_busy_dur
            = psy_time_point_add(now, self->busy_loop_dur);

        PsyTimer *first = self->timers->pdata[0];
        g_assert(PSY_IS_TIMER(first));
        PsyTimePoint *tp = psy_timer_get_fire_time(first);

        if (psy_time_point_greater_equal(now, tp)) {
            psy_timer_fire(first, psy_timer_get_fire_time(first));
            g_ptr_array_remove_index(self->timers, 0);

            psy_time_point_free(now);
            psy_time_point_free(now_plus_busy_dur);
            break;
        }

        ThreadData *msg = g_async_queue_try_pop(self->queue);
        if (msg) {
            timer_thread_handle_message(self, msg);
        }

        psy_time_point_free(now);
        psy_time_point_free(now_plus_busy_dur);
    }
}

/**
 * psy_timer_thread_check_timers:
 *
 * This function checks whether there are timers about to be ready to fire
 *
 * Returns: TRUE if a timer is ready within now and now + self->busy_dur
 *
 * stability:private
 */
static gboolean
psy_timer_thread_check_timers(PsyTimerThread *self)
{
    gboolean ret = FALSE;

    if (self->timers->len > 0) {
        PsyTimePoint *now = psy_clock_now(self->clock);

        PsyTimer     *first = self->timers->pdata[0];
        PsyTimePoint *tp    = psy_timer_get_fire_time(first);
        g_assert(tp != NULL);
        PsyDuration *dur = psy_time_point_subtract(tp, now);

        if (psy_duration_less_equal(dur, self->busy_loop_dur)) {
            ret = TRUE;
        }
        psy_duration_free(dur);
        psy_time_point_free(now);
    }

    return ret;
}

static void
timer_thread_handle_message(PsyTimerThread *self, ThreadData *data)
{
    g_assert(PSY_IS_TIMER_THREAD(self));
    g_assert(data != NULL);

    switch (data->msg) {
    case MSG_STOP:
        self->running = FALSE;
        break;
    case MSG_TIMER_ADD:
        psy_timer_thread_add_timer(self, data->timer);
        break;
    case MSG_TIMER_DEL:
        psy_timer_thread_del_timer(self, data->timer);
        break;
    default:
        g_assert_not_reached();
    }

    thread_data_free(data); // clean up message
}

// the thread
static gpointer
timer_thread(gpointer data)
{
    PsyTimerThread *self = data;

    while (self->running) {
        ThreadData *data = g_async_queue_timeout_pop(self->queue, 1000);
        if (data) {
            timer_thread_handle_message(self, data);
        }

        while (psy_timer_thread_check_timers(self)) {
            // Timers are almost ready for dispatch
            psy_timer_thread_fire_timers(self);
        }
    }

    return data;
}

static void
psy_timer_thread_join(PsyTimerThread *self)
{
    if (!self->thread)
        return;

    // Send stop message
    ThreadData *data = thread_data_new(MSG_STOP, self, NULL);

    g_async_queue_push(self->queue, data);

    g_thread_join(self->thread);
    self->thread = NULL;
}

/* **** non static functions *** */

void
timer_private_start_timer_thread(void)
{
    g_info("Starting timer thread");
    g_timer_thread = g_object_new(PSY_TYPE_TIMER_THREAD, NULL);
}

void
timer_private_stop_timer_thread(void)
{
    psy_timer_thread_join(g_timer_thread);
    g_clear_object(&g_timer_thread);
    init_warning = 0;
}

/**
 * timer_private_add_timer:
 * @timer:(transfer none): a timer that needs to be signaled
 *
 * Add a timer to the timer thread to have it signaled in time when
 * possible
 *
 * Stability:Private
 */
void
timer_private_add_timer(PsyTimer *timer)
{
    if (G_UNLIKELY(!PSY_IS_TIMER_THREAD(g_timer_thread))) {
        if (!init_warning) {
            g_warning("The timer thread isn't running, did you init: %s",
                      PSY_PROJECT_NAME);
            init_warning = 1;
        }
        return;
    }

    ThreadData *data = thread_data_new(MSG_TIMER_ADD, g_timer_thread, timer);
    g_async_queue_push(g_timer_thread->queue, data);
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
    if (G_UNLIKELY(!PSY_IS_TIMER_THREAD(g_timer_thread))) {
        if (!init_warning) {
            g_warning("The timer thread isn't running, did you init: %s",
                      PSY_PROJECT_NAME);
            init_warning = 1;
        }
        return;
    }

    ThreadData *data = thread_data_new(MSG_TIMER_DEL, g_timer_thread, timer);
    g_async_queue_push(g_timer_thread->queue, data);

    GAsyncQueue  *timer_queue = psy_timer_get_queue(timer);
    const guint64 one_ms      = 1000; // 1000 µs
    ThreadData   *result      = g_async_queue_timeout_pop(timer_queue, one_ms);

    if (G_UNLIKELY(result == NULL || result->msg != MSG_TIMER_CANCELED)) {
        g_critical("Didn't receive an timer cancel acknowledgment.");
    }
}
