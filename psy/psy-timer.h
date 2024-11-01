
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

/**
 * psy_timer_async_cb:
 *
 * This is a callback that will be called from a thread that monitors the
 * timers. The callback should last as short as possible in order not to hinder
 * other timers.
 */
typedef void (*psy_timer_async_cb)(PsyTimePoint *tp, gpointer data);

G_MODULE_EXPORT gboolean
psy_timer_set_async_fire_cb(PsyTimer          *self,
                            psy_timer_async_cb cb,
                            gpointer           data);

/*The next functions are internal*/

void
psy_timer_fire(PsyTimer *self, PsyTimePoint *tp);

GAsyncQueue *
psy_timer_get_queue(PsyTimer *self);

G_END_DECLS

#endif
