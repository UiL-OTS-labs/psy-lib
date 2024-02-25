
#ifndef PSY_SIDE_STEP_H
#define PSY_SIDE_STEP_H

#include "psy-step.h"

G_BEGIN_DECLS

#define PSY_TYPE_SIDE_STEP psy_side_step_get_type()
G_DECLARE_FINAL_TYPE(PsySideStep, psy_side_step, PSY, SIDE_STEP, PsyStep)

G_MODULE_EXPORT PsySideStep *
psy_side_step_new(void);

G_MODULE_EXPORT void
psy_side_step_free(PsySideStep *self);

G_END_DECLS

#endif
