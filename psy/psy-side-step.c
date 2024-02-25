
#include "psy-side-step.h"

struct _PsySideStep {
    PsyStep parent;
};

G_DEFINE_TYPE(PsySideStep, psy_side_step, PSY_TYPE_STEP)

static void
side_step_activate(PsyStep *self, PsyTimePoint *tp)
{
    // Chain up to parent.
    PSY_STEP_CLASS(psy_side_step_parent_class)->activate(self, tp);
    psy_step_leave(self, tp);
}

static void
psy_side_step_class_init(PsySideStepClass *klass)
{
    (void) klass;
    PsyStepClass *step_class = PSY_STEP_CLASS(klass);

    step_class->activate = side_step_activate;
}

static void
psy_side_step_init(PsySideStep *trial)
{
    (void) trial;
}

/**
 * psy_side_step_new:(constructor)
 *
 * Create a new side_step, to be freed with g_object_unref or
 * [method@SideStep.free]
 *
 * Returns: a new [class@SideStep] instance
 */
PsySideStep *
psy_side_step_new(void)
{
    return g_object_new(PSY_TYPE_SIDE_STEP, NULL);
}

/**
 * psy_side_step_free:(skip)
 *
 * Frees a side_step previously allocated with psy_side_step_new.
 */
void
psy_side_step_free(PsySideStep *self)
{
    g_return_if_fail(PSY_IS_SIDE_STEP(self));
    g_object_unref(self);
}
