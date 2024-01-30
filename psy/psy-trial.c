
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

/**
 * psy_trial_new:(constructor)
 *
 * Create a new trial, to be freed with g_object_unref or [method@Trial.free]
 *
 * Returns: a new [class@Trial] instance
 */
PsyTrial *
psy_trial_new(void)
{
    return g_object_new(PSY_TYPE_TRIAL, NULL);
}

/**
 * psy_trial_free:(skip)
 *
 *
 * Frees a trial previoulsy allocated with psy_trial_new* fam of functions
 */
void
psy_trial_free(PsyTrial *self)
{
    g_return_if_fail(PSY_IS_TRIAL(self));
    g_object_unref(self);
}
