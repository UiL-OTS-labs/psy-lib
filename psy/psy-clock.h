
#pragma once

#include "psy-time-point.h"

G_BEGIN_DECLS

#define PSY_TYPE_CLOCK psy_clock_get_type()
G_DECLARE_FINAL_TYPE(PsyClock, psy_clock, PSY, CLOCK, GObject)

G_MODULE_EXPORT PsyClock*
psy_clock_new(void);

G_MODULE_EXPORT PsyTimePoint*
psy_clock_now(PsyClock* self);

G_MODULE_EXPORT gint64
psy_clock_get_zero_time(void);

G_END_DECLS
