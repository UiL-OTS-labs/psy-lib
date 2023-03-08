
#pragma once

#include "psy-visual-stimulus.h"

G_BEGIN_DECLS

#define PSY_TYPE_RECTANGLE psy_rectangle_get_type()
G_DECLARE_DERIVABLE_TYPE(
    PsyRectangle, psy_rectangle, PSY, RECTANGLE, PsyVisualStimulus)

/**
 * PsyRectangleClass:
 * @set_width: Sets the width of the rectangle.
 *             (may reflect about the y-axis the image when negative)
 * @set_height: Sets the height of the rectangle.
 *             (may reflect the image about the x-axis when negative)
 *
 * The PsyRectangleClass is a (Base) class for rectangular shapes
 */
typedef struct _PsyRectangleClass {
    PsyVisualStimulusClass parent;

    void (*set_width)(PsyRectangle *self, gfloat width);
    void (*set_height)(PsyRectangle *self, gfloat height);

    gpointer reserved[16];
} PsyRectangleClass;

G_MODULE_EXPORT PsyRectangle *
psy_rectangle_new(PsyCanvas *canvas);

G_MODULE_EXPORT PsyRectangle *
psy_rectangle_new_full(
    PsyCanvas *canvas, gfloat x, gfloat y, gfloat width, gfloat height);

G_MODULE_EXPORT void
psy_rectangle_set_width(PsyRectangle *self, gfloat width);

G_MODULE_EXPORT gfloat
psy_rectangle_get_width(PsyRectangle *self);

G_MODULE_EXPORT void
psy_rectangle_set_height(PsyRectangle *self, gfloat height);

G_MODULE_EXPORT gfloat
psy_rectangle_get_height(PsyRectangle *self);

G_MODULE_EXPORT void
psy_rectangle_set_size(PsyRectangle *self, gfloat width, gfloat height);

G_END_DECLS
