
#pragma once

#include <glib-object.h>
#include "psy-time-point.h"
#include "psy-artist.h"
#include "psy-visual-stimulus.h"
#include "psy-program.h"
#include "psy-enums.h"
#include "psy-drawing-context.h"

G_BEGIN_DECLS


#define PSY_TYPE_WINDOW psy_window_get_type()
G_DECLARE_DERIVABLE_TYPE(PsyWindow, psy_window, PSY, WINDOW, GObject)

/**
 * PsyWindowClass:
 * @parent_class: The parent class
 * @set_monitor: Set the window at monitor x.
 * @get_monitor: Retrieve the number of the monitor
 * @get_width: Get the window width in pixels
 * @get_height: Get the window height in pixels
 * @draw: clears the window, than draws the stimuli.
 * @clear: A function to clear the picture to the background color
 * @draw_stimuli: A function that draws the stimuli.
 * @set_monitor_size_mm: a function used by the child to set the physical size
 *                       of the window that matches the size of the monitor in mm.
 * @get_monitor_size_mm: get the physical size of the window.
 * @set_frame_dur:set the duration of the frame
 * @get_frame_dur:get the duration of the frame
 * @schedule_stimulus:This function is called when a new stimulus is scheduled
 *                    for this window.
 * @create_projection_matrix: create an projection matrix that is suitable
 *                            for the current value of 
 */
typedef struct _PsyWindowClass {
    GObjectClass parent_class;

    /*< public >*/

    void (*set_monitor)(PsyWindow* self, gint nth_monitor);
    gint (*get_monitor)(PsyWindow* self);

    void (*resize) (PsyWindow* self, gint width, gint height);

    void (*set_width)(PsyWindow* window, gint width);
    void (*set_height)(PsyWindow* window, gint width);

    void (*draw) (PsyWindow* self, guint64 frame_num, PsyTimePoint* frame_time);
    void (*clear)(PsyWindow* self);
    void (*draw_stimuli)(PsyWindow* self, guint64 frame_num, PsyTimePoint* tp);
    void (*draw_stimulus)(PsyWindow* self, PsyVisualStimulus* stimulus);

    void (*set_monitor_size_mm)(PsyWindow* monitor,
                                gint width_mm,
                                gint height_mm);

    PsyDuration* (*get_frame_dur) (PsyWindow* window);
    void (*set_frame_dur) (PsyWindow* window, PsyDuration* dur);

    PsyArtist* (*create_artist)(PsyWindow* self, PsyVisualStimulus* stimulus);
    void (*schedule_stimulus) (PsyWindow* self, PsyVisualStimulus* stimulus);
    void (*remove_stimulus) (PsyWindow* self, PsyVisualStimulus* stimulus);

    PsyMatrix4* (*create_projection_matrix)(PsyWindow* self);
    void (*set_projection_matrix)(PsyWindow* self, PsyMatrix4* projection);
    void (*upload_projection_matrices)(PsyWindow* window);
    
    /*< private >*/

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

G_MODULE_EXPORT gint
psy_window_get_width(PsyWindow* window);

G_MODULE_EXPORT gint
psy_window_get_height(PsyWindow* window);

G_MODULE_EXPORT void
psy_window_set_width_mm(PsyWindow* window, gint width_mm);

G_MODULE_EXPORT void
psy_window_set_height_mm(PsyWindow* window, gint height_mm);

G_MODULE_EXPORT void
psy_window_schedule_stimulus(PsyWindow* window, PsyVisualStimulus* stimulus);

G_MODULE_EXPORT PsyDuration*
psy_window_get_frame_dur(PsyWindow* window);

G_MODULE_EXPORT void
psy_window_remove_stimulus(PsyWindow* window, PsyVisualStimulus* stimulus);

G_MODULE_EXPORT void
psy_window_set_projection_style(PsyWindow* window, gint projection_style);

G_MODULE_EXPORT gint
psy_window_get_projection_style(PsyWindow* window);

G_MODULE_EXPORT PsyMatrix4*
psy_window_get_projection(PsyWindow* window);

G_MODULE_EXPORT void 
psy_window_set_context(PsyWindow* window, PsyDrawingContext* context);

G_MODULE_EXPORT PsyDrawingContext*
psy_window_get_context(PsyWindow* window);


G_END_DECLS

