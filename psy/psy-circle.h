
#pragma once

#include "psy-visual-stimulus.h"

G_BEGIN_DECLS

#define PSY_TYPE_CIRCLE psy_circle_get_type()
G_DECLARE_DERIVABLE_TYPE(PsyCircle, psy_circle, PSY, CIRCLE, PsyVisualStimulus)

typedef struct _PsyCircleClass {
    PsyVisualStimulusClass parent;
} PsyCircleClass;

G_MODULE_EXPORT PsyCircle *
psy_circle_new(PsyWindow *window);

G_MODULE_EXPORT PsyCircle *
psy_circle_new_full(
    PsyWindow *window, gfloat x, gfloat y, gfloat radius, guint num_vertices);

G_MODULE_EXPORT void
psy_circle_set_radius(PsyCircle *circle, gfloat radius);

G_MODULE_EXPORT gfloat
psy_circle_get_radius(PsyCircle *circle);

G_MODULE_EXPORT void
psy_circle_set_num_vertices(PsyCircle *circle, guint num_vertices);

G_MODULE_EXPORT guint
psy_circle_get_num_vertices(PsyCircle *circle);

G_END_DECLS
