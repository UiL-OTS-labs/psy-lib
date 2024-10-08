
#ifndef PSY_TIMER_PRIVATE_H
#define PSY_TIMER_PRIVATE_H

#include "psy-timer.h"

G_BEGIN_DECLS

#define PSY_TYPE_TIMER_THREAD psy_timer_thread_get_type()

G_DECLARE_FINAL_TYPE(
    PsyTimerThread, psy_timer_thread, PSY, TIMER_THREAD, GObject)

void
timer_private_start_timer_thread(void);

void
timer_private_stop_timer_thread(void);

void
timer_private_set_timer(PsyTimer *timer);

void
timer_private_cancel_timer(PsyTimer *timer);

void
psy_timer_fire(PsyTimer *self, PsyTimePoint *tp);

G_END_DECLS

#endif
