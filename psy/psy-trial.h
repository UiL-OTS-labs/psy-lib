
#ifndef PSY_TRIAL_H
#define PSY_TRIAL_H

#include "psy-step.h"

G_BEGIN_DECLS

#define PSY_TYPE_TRIAL psy_trial_get_type()

G_MODULE_EXPORT
G_DECLARE_FINAL_TYPE(PsyTrial, psy_trial, PSY, TRIAL, PsyStep)

G_MODULE_EXPORT PsyTrial *
psy_trial_new(void);

G_MODULE_EXPORT void
psy_trial_free(PsyTrial *trial);

G_END_DECLS

#endif
