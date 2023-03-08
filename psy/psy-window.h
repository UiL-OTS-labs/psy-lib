
#pragma once

#include "psy-canvas.h"
#include "psy-enums.h"
#include <glib-object.h>

G_BEGIN_DECLS

#define PSY_TYPE_WINDOW psy_window_get_type()
G_DECLARE_DERIVABLE_TYPE(PsyWindow, psy_window, PSY, WINDOW, PsyCanvas)

/**
 * PsyWindowClass:
 * @parent_class: The parent class
 * @set_monitor: Set the window at monitor x.
 * @get_monitor: Retrieve the number of the monitor
 */
typedef struct _PsyWindowClass {
    PsyCanvasClass parent_class;

    /*< public >*/

    void (*set_monitor)(PsyWindow *self, gint nth_monitor);
    gint (*get_monitor)(PsyWindow *self);

    /*< private >*/

    gpointer padding[12];
} PsyWindowClass;

G_MODULE_EXPORT gint
psy_window_get_monitor(PsyWindow *self);

G_MODULE_EXPORT void
psy_window_set_monitor(PsyWindow *self, gint nth_monitor);

G_END_DECLS
