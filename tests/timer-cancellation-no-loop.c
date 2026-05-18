#include <assert.h>
#include <psylib.h>

static void
dont_fire(PsyTimer *timer, PsyTimePoint *tp)
{
    (void) timer;
    (void) tp;
    assert(1 == 0); // shouldn't be called as no loop is running
}

int
main(void)
{
    PsyInitializer *init = g_object_new(
        PSY_TYPE_INITIALIZER, "gstreamer", FALSE, "portaudio", FALSE, NULL);

    PsyClock     *clk    = psy_clock_new();
    PsyTimer     *timer  = psy_timer_new();
    PsyTimePoint *tp_now = psy_clock_now(clk);

    g_print("timer = %p\n", (void *) timer);

    g_signal_connect(timer, "fired", G_CALLBACK(dont_fire), NULL);
    g_object_set(timer, "fire-time", tp_now, NULL);

    g_usleep(10000); // 10ms

    psy_time_point_free(tp_now);
    psy_timer_free(timer);
    psy_clock_free(clk);
    g_object_unref(init);
}
