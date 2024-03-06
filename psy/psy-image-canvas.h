
#pragma once

#include <glib-object.h>

#include "psy-canvas.h"

G_BEGIN_DECLS

#define PSY_TYPE_IMAGE_CANVAS psy_image_canvas_get_type()
G_MODULE_EXPORT
G_DECLARE_DERIVABLE_TYPE(
    PsyImageCanvas, psy_image_canvas, PSY, IMAGE_CANVAS, PsyCanvas)

/**
 * PsyImageCanvasClass:
 * @parent_class: The parent class
 * @iterate: A function that runs one iteration of drawing.
 */
typedef struct _PsyImageCanvasClass {
    PsyCanvasClass parent_class;

    /*< public >*/

    void (*iterate)(PsyImageCanvas *self);

    /*< private >*/

    gpointer padding[12];
} PsyImageCanvasClass;

G_MODULE_EXPORT PsyImageCanvas *
psy_image_canvas_new(gint width, gint height);

G_MODULE_EXPORT void
psy_image_canvas_free(PsyImageCanvas *self);

G_MODULE_EXPORT void
psy_image_canvas_iterate(PsyImageCanvas *self);

G_MODULE_EXPORT void
psy_image_canvas_set_time(PsyImageCanvas *self, PsyTimePoint *tp);

G_MODULE_EXPORT PsyTimePoint *
psy_image_canvas_get_time(PsyImageCanvas *self);

G_END_DECLS
