
#include "ddd-trial.h"

struct _DddTrial {
    DddStep parent;
};

G_DEFINE_TYPE(DddTrial, ddd_trial, DDD_TYPE_STEP)

static void
ddd_trial_class_init(DddTrialClass* klass)
{
    (void) klass;
}

static void
ddd_trial_init(DddTrial* trial)
{
    (void) trial;
}

DddTrial*
ddd_trial_new()
{
    return g_object_new(DDD_TYPE_TRIAL, NULL);
}
