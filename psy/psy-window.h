
#pragma once

#include "psy-time-point.h"
#include <glib-object.h>

G_BEGIN_DECLS

/** Forward declarations as psy-visual-stimulus.h also includes this file.*/
struct _PsyVisualStimulus;
typedef struct _PsyVisualStimulus PsyVisualStimulus;

#define PSY_TYPE_WINDOW psy_window_get_type()
G_DECLARE_DERIVABLE_TYPE(PsyWindow, psy_window, PSY, WINDOW, GObject)


typedef struct _PsyWindowClass {
    GObjectClass parent_class;

    void (*set_monitor)(PsyWindow* self, gint nth_monitor);
    int  (*get_monitor)(PsyWindow* self);

    void (*draw) (PsyWindow* self, PsyTimePoint* frame_time);
    void (*clear)(PsyWindow* self);
    void (*draw_stimuli)(PsyWindow* self, guint64 frame_num, PsyTimePoint* tp);

    void (*set_monitor_size_mm)(PsyWindow* monitor,
                                gint width_mm,
                                gint height_mm);

    PsyDuration* (*get_frame_dur) (PsyWindow* window);
    void (*set_frame_dur) (PsyWindow* window, PsyDuration* dur);

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

G_MODULE_EXPORT void 
psy_window_get_width_height_mm(PsyWindow* window, gint* width, gint* height);

G_MODULE_EXPORT gint 
psy_window_get_width_mm(PsyWindow* window);

G_MODULE_EXPORT gint 
psy_window_get_height_mm(PsyWindow* window);

G_MODULE_EXPORT void
psy_window_set_width_mm(PsyWindow* window, gint width_mm);

G_MODULE_EXPORT void
psy_window_set_height_mm(PsyWindow* window, gint height_mm);

G_MODULE_EXPORT void
psy_window_schedule_stimulus(PsyWindow* window, PsyVisualStimulus* stimulus);

G_MODULE_EXPORT PsyDuration*
psy_window_get_frame_dur(PsyWindow* window);


G_END_DECLS

