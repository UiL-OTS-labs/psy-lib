/**
 * SideStep:
 *
 * # SideSteps are steps used for its side effects
 *
 * Sometimes when using [class@SteppingStones] you just want to have
 * a step where you want to jump to, or to test something without running
 * a complete trial. This is where [class@SideStep] comes in. Steps
 * like the are auto leaving, this means when the step is activated, it
 * will leave it self, with the same timestamp it was activated with.
 *
 * so when you have an experiment like this:
 *
 * ```
 * SteppingStones: experiment
 *   |
 *   |-- Trial: Elaborate Instruction
 *   |
 *   |-- SideStep: added with key "post-instruction"
 *   |
 *   |-- Trial: instruction "Respond as quickly as possible"
 *   |
 *   |-- Loop:
 *       |
 *       | Trial:PracticeTrial
 *
 *   |-- SideStep: test if loop of trials had suffcient performance
 *   |             if not, jump back to step with key "post-instruction"
 *   .
 *   . the rest of the experiment.
 *   .
 * ```
 *
 * So in the experiment above, you first have an elaborate instruction,
 * The you step in the "post-instruction" SideStep, which will leave it self
 * and the short instruction is displayed. Then a loop of practice trials
 * is presented and when that is done, we can test in the 2nd sidestep
 * whether we jump back to the first side step and this way, the paricipant
 * only needs to read the short instruction.
 *
 * This is a bit of a toy example, because you can also do this from the
 * `Loop`s [signal@Step.leave] signal and jump straight back to the short
 * instruction trial.
 */

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
