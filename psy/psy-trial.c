
#include "psy-trial.h"

struct _PsyTrial {
    PsyStep parent;
};

G_DEFINE_TYPE(PsyTrial, psy_trial, PSY_TYPE_STEP)

static void
psy_trial_class_init(PsyTrialClass *klass)
{
    (void) klass;
}

static void
psy_trial_init(PsyTrial *trial)
{
    (void) trial;
}

PsyTrial *
psy_trial_new()
{
    return g_object_new(PSY_TYPE_TRIAL, NULL);
}
