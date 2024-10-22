
#ifndef PSY_TIMER_H
#define PSY_TIMER_H

#include <gio/gio.h>
#include <glib-object.h>

#include "psy-time-point.h"

G_BEGIN_DECLS

#define PSY_TYPE_TIMER psy_timer_get_type()

G_MODULE_EXPORT
G_DECLARE_FINAL_TYPE(PsyTimer, psy_timer, PSY, TIMER, GObject)

G_MODULE_EXPORT PsyTimer *
psy_timer_new(void);

G_MODULE_EXPORT void
psy_timer_free(PsyTimer *self);

G_MODULE_EXPORT void
psy_timer_set_fire_time(PsyTimer *self, PsyTimePoint *tp);

G_MODULE_EXPORT PsyTimePoint *
psy_timer_get_fire_time(PsyTimer *self);

G_MODULE_EXPORT void
psy_timer_cancel(PsyTimer *self);

/*The next functions are internal*/

void
psy_timer_fire(PsyTimer *self, PsyTimePoint *tp);

GAsyncQueue *
psy_timer_get_queue(PsyTimer *self);

G_END_DECLS

#endif
