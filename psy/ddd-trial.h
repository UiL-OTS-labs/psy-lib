
#ifndef DDD_TRIAL_H
#define DDD_TRIAL_H

#include "ddd-step.h"

G_BEGIN_DECLS

#define DDD_TYPE_TRIAL ddd_trial_get_type()
G_DECLARE_FINAL_TYPE(DddTrial, ddd_trial, DDD, TRIAL, DddStep)

DddTrial*
ddd_trial_new();

G_END_DECLS

#endif
