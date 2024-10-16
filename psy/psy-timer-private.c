

#include "psy-timer-private.h"
#include "psy-clock.h"

#ifdef _WIN32
    #include <windows.h>
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

    GMainContext *context;
    GMainLoop    *loop;

    GPtrArray   *timers;
    GThread     *thread;
    PsyClock    *clock;
    PsyDuration *busy_loop_dur;

    guint busy_loop_id;
} PsyTimerThread;

G_DEFINE_TYPE(PsyTimerThread, psy_timer_thread, G_TYPE_OBJECT)

static void
psy_timer_thread_init(PsyTimerThread *self)
{
    self->context       = g_main_context_new();
    self->timers        = g_ptr_array_new_full(64, g_object_unref);
    self->thread        = g_thread_new("TimerThread", timer_thread, self);
    self->clock         = psy_clock_new();
    self->busy_loop_dur = psy_duration_new_ms(2);

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

    g_main_context_unref(tt_self->context);
    tt_self->context = NULL;

    g_clear_object(&tt_self->clock);

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

    PsyTimer *timer = data->timer;

    g_ptr_array_add(self->timers, g_object_ref(timer));

#if GLIB_CHECK_VERSION(2, 76, 0)
    g_ptr_array_sort_values(self->timers, compare_timer_time_stamps_values);
#else
    g_ptr_array_sort(self->timers, compare_timer_time_stamps);
#endif

    return G_SOURCE_REMOVE;
}

static gboolean
psy_timer_thread_del_timer(ThreadData *data)
{
    g_assert(data->msg == MSG_TIMER_DEL);

    PsyTimerThread *self = data->self;

    PsyTimer *t_remove = data->timer;

    g_ptr_array_remove(self->timers, t_remove);

    return G_SOURCE_REMOVE;
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
 * A callback function that checks whether one or multiple timers
 * are ready to fire. If so, the timers are fired.
 *
 * If there are no timers with a fire time  before now + self->busy_loop_dur
 * the callback is removed.
 *
 * stability:private
 */
static gboolean
psy_timer_thread_fire_timers(gpointer data)
{
    gboolean ret = G_SOURCE_CONTINUE;

    PsyTimerThread *self = data;
    PsyTimePoint   *now  = psy_clock_now(self->clock);

    while (self->timers->len > 0) {
        PsyTimer *first = self->timers->pdata[0];
        g_assert(PSY_IS_TIMER(first));
        PsyTimePoint *tp = psy_timer_get_fire_time(first);

        if (psy_time_point_greater_equal(now, tp)) {
            psy_timer_fire(first, psy_timer_get_fire_time(first));
            g_ptr_array_remove_index(self->timers, 0);
        }
        else { // There are currently no timers ready;
            psy_time_point_free(now);
            return ret;
        }
    }

    if (self->timers->len > 0) {
        PsyTimer     *first   = self->timers->pdata[0];
        PsyTimePoint *tp_fire = psy_timer_get_fire_time(first);

        PsyDuration *dur = psy_time_point_subtract(tp_fire, now);
        if (psy_duration_greater_equal(dur, self->busy_loop_dur)) {
            self->busy_loop_id = 0;
            ret                = G_SOURCE_REMOVE;
        }

        psy_duration_free(dur);
    }
    else {
        self->busy_loop_id = 0;
        ret                = G_SOURCE_REMOVE;
    }

    psy_time_point_free(now);

    return ret;
}

static void
psy_timer_thread_start_busy_loop(PsyTimerThread *self)
{
    g_debug("starting busy loop");
    GSource *busy_source = g_idle_source_new();
    g_source_set_callback(
        busy_source, psy_timer_thread_fire_timers, self, NULL);
    g_source_set_priority(busy_source, G_PRIORITY_DEFAULT_IDLE);
    self->busy_loop_id = g_source_attach(busy_source, self->context);
    g_source_unref(busy_source);
}

/**
 * psy_timer_thread_check_timers:
 *
 * This function is called every +/- 1ms, if there are any timers
 * that are to be fired within now and self->busy_dur, it will add an idle
 * function that will be polled continuously to see if a timer should fire.
 *
 * stability:private
 */
static gboolean
psy_timer_thread_check_timers(gpointer data)
{
    PsyTimerThread *self = data;

    if (self->timers->len > 0) {
        PsyTimePoint *now = psy_clock_now(self->clock);

        PsyTimer     *first = self->timers->pdata[0];
        PsyTimePoint *tp    = psy_timer_get_fire_time(first);
        g_assert(tp != NULL);

        PsyDuration *dur = psy_time_point_subtract(tp, now);

        if (psy_duration_less_equal(dur, self->busy_loop_dur)) {
            if (self->busy_loop_id == 0) {
                psy_timer_thread_start_busy_loop(self);
            }
        }
        psy_duration_free(dur);
        psy_time_point_free(now);
    }

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
