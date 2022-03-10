
#pragma once

#include "ddd-time-point.h"

G_BEGIN_DECLS

#define DDD_TYPE_CLOCK ddd_clock_get_type()
G_DECLARE_FINAL_TYPE(DddClock, ddd_clock, DDD, CLOCK, GObject)

G_MODULE_EXPORT DddClock*
ddd_clock_new();

G_MODULE_EXPORT DddTimePoint*
ddd_clock_now(DddClock* clock);

G_END_DECLS
