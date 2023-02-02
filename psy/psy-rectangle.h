
#pragma once

#include "psy-visual-stimulus.h"

G_BEGIN_DECLS

#define PSY_TYPE_RECTANGLE psy_rectangle_get_type()
G_DECLARE_DERIVABLE_TYPE(
    PsyRectangle, psy_rectangle, PSY, RECTANGLE, PsyVisualStimulus)

typedef struct _PsyRectangleClass {
    PsyVisualStimulusClass parent;
} PsyRectangleClass;

G_MODULE_EXPORT PsyRectangle *
psy_rectangle_new(PsyWindow *window);

G_MODULE_EXPORT PsyRectangle *
psy_rectangle_new_full(
    PsyWindow *window, gfloat x, gfloat y, gfloat width, gfloat height);

G_MODULE_EXPORT void
psy_rectangle_set_width(PsyRectangle *rectangle, gfloat width);

G_MODULE_EXPORT gfloat
psy_rectangle_get_width(PsyRectangle *rectangle);

G_MODULE_EXPORT void
psy_rectangle_set_height(PsyRectangle *rectangle, gfloat height);

G_MODULE_EXPORT gfloat
psy_rectangle_get_height(PsyRectangle *rectangle);

G_END_DECLS
