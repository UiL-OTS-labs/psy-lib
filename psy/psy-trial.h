
#ifndef PSY_TRIAL_H
#define PSY_TRIAL_H

#include "psy-step.h"

G_BEGIN_DECLS

#define PSY_TYPE_TRIAL psy_trial_get_type()
G_DECLARE_FINAL_TYPE(PsyTrial, psy_trial, PSY, TRIAL, PsyStep)

PsyTrial *
psy_trial_new();

G_END_DECLS

#endif
