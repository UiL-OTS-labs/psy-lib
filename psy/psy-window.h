
#ifndef PSY_WINDOW_H
#define PSY_WINDOW_H

#include <glib-object.h>
#include <psy-visual-stimulus.h>

G_BEGIN_DECLS

#define PSY_TYPE_WINDOW psy_window_get_type()
G_DECLARE_DERIVABLE_TYPE(PsyWindow, psy_window, PSY, WINDOW, GObject)

typedef struct _PsyWindowClass {
    GObjectClass parent_class;

    void (*set_monitor)(PsyWindow* self, gint nth_monitor);
    int  (*get_monitor)(PsyWindow* self);

    void (*schedule_stimulus) (PsyWindow* self, PsyVisualStimulus* stimulus);

    gpointer padding[12];
} PsyWindowClass;


G_MODULE_EXPORT gint 
psy_window_get_monitor(PsyWindow* window);

G_MODULE_EXPORT void
psy_window_set_monitor(PsyWindow* window, gint nth_monitor);

G_MODULE_EXPORT void
psy_window_set_background_color_values(PsyWindow* window, gfloat* color);

G_MODULE_EXPORT void
psy_window_get_background_color_values(PsyWindow* window, gfloat* color);


G_END_DECLS


#endif
