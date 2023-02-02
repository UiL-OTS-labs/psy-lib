
#pragma once

#include "psy-visual-stimulus.h"

G_BEGIN_DECLS

#define PSY_TYPE_CROSS psy_cross_get_type()
G_DECLARE_DERIVABLE_TYPE(PsyCross, psy_cross, PSY, CROSS, PsyVisualStimulus)

typedef struct _PsyCrossClass {
    PsyVisualStimulusClass parent;
} PsyCrossClass;

G_MODULE_EXPORT PsyCross *
psy_cross_new(PsyWindow *window);

G_MODULE_EXPORT PsyCross *
psy_cross_new_full(
    PsyWindow *window, gfloat x, gfloat y, gfloat length, gfloat line_width);

G_MODULE_EXPORT void
psy_cross_set_line_length_x(PsyCross *cross, gfloat length);

G_MODULE_EXPORT gfloat
psy_cross_get_line_length_x(PsyCross *cross);

G_MODULE_EXPORT void
psy_cross_set_line_length_y(PsyCross *cross, gfloat length);

G_MODULE_EXPORT gfloat
psy_cross_get_line_length_y(PsyCross *cross);

G_MODULE_EXPORT void
psy_cross_set_line_width_x(PsyCross *cross, gfloat width);

G_MODULE_EXPORT gfloat
psy_cross_get_line_width_x(PsyCross *cross);

G_MODULE_EXPORT void
psy_cross_set_line_width_y(PsyCross *cross, gfloat width);

G_MODULE_EXPORT gfloat
psy_cross_get_line_width_y(PsyCross *cross);

G_END_DECLS
