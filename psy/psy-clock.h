
#pragma once

#include "psy-time-point.h"

G_BEGIN_DECLS

#define PSY_TYPE_CLOCK psy_clock_get_type()
G_DECLARE_FINAL_TYPE(PsyClock, psy_clock, PSY, CLOCK, GObject)

G_MODULE_EXPORT PsyClock*
psy_clock_new();

G_MODULE_EXPORT PsyTimePoint*
psy_clock_now(PsyClock* clock);

G_END_DECLS
