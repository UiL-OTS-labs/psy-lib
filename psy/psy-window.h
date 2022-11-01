
#pragma once

#include <glib-object.h>
#include "psy-time-point.h"
#include "psy-program.h"

G_BEGIN_DECLS

/*
 * Forward declarations as psy-visual-stimulus.h also includes this file.
 */
struct _PsyVisualStimulus;
typedef struct _PsyVisualStimulus PsyVisualStimulus;

/**
 * PsyWindowProjectionStyle:
 * @PSY_WINDOW_PROJECTION_STYLE_C: 0.0, 0.0 is in the upper left corner of the
 *                                 window. With positive y coordinates going down.
 * @PSY_WINDOW_PROJECTION_STYLE_CENTER: 0.0, 0.0 is in the center of the window
 *                            with positive y coordinates is going up.
 * @PSY_WINDOW_PROJECTION_STYLE_PIXELS: Create a projection matrix that makes the
 *                            screen as wide,tall as the dimensions of the
 *                            number of pixels. Stimuli may than also be
 *                            specified in pixels.
 * @PSY_WINDOW_PROJECTION_STYLE_METER:  Create a projection matrix based on the number
 *                            meters the window is, the sizes of stimuli can be
 *                            specified in meters.
 * @PSY_WINDOW_PROJECTION_STYLE_MILLIMETER: Create a projection matrix based on the
 *                            number meters the window is tall. Stimuli
 *                            may also be presented in millimeters.
 * @PSY_WINDOW_PROJECTION_STYLE_VISUAL_DEGREES: Create a projection matrix based on 
 *                            the number of visual degrees the window is tall.
 *                            Stimuli should be specified in visual degrees.
 *                            NOT IMPLEMENTED YET.
 *
 * Instances of `PsyWindow` use an orthographic projection by default. We
 * focus on 2 Dimensional stimuli. This enum can be used to set the 
 * projection-style property of a PsyWindow. By default the projection is
 * set up with the point[0.0, 0.0] in the center of the screen, regardless
 * of the units used.
 *
 * The PsyWindow is able to tell the size in meters of the window when it's
 * fullscreen and also the "size" in pixels. It doesn't know how far the
 * subject is sitting from the screen, so this need to be specified when using
 * #PsyProjectionStyleVisualDegrees.
 *
 * When using these flags one should always specify exactly one of:
 *
 *  - PSY_WINDOW_PROJECTION_STYLE_C
 *  - PSY_WINDOW_PROJECTION_STYLE_CENTER
 *
 * and exactly one of 
 *
 *  - PSY_WINDOW_PROJECTION_STYLE_PIXELS
 *  - PSY_WINDOW_PROJECTION_STYLE_METER
 *  - PSY_WINDOW_PROJECTION_STYLE_MILLIMETER
 *  - PSY_WINDOW_PROJECTION_STYLE_VISUAL_DEGREES
 */
typedef enum _PsyWindowProjectionStyle {
    PSY_WINDOW_PROJECTION_STYLE_C               = 1 << 0,
    PSY_WINDOW_PROJECTION_STYLE_CENTER          = 1 << 1,
    PSY_WINDOW_PROJECTION_STYLE_PIXELS          = 1 << 2,
    PSY_WINDOW_PROJECTION_STYLE_METER           = 1 << 3,
    PSY_WINDOW_PROJECTION_STYLE_MILLIMETER      = 1 << 4,
    PSY_WINDOW_PROJECTION_STYLE_VISUAL_DEGREES  = 1 << 5
} PsyWindowProjectionStyle;

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

    void (*schedule_stimulus) (PsyWindow* self, PsyVisualStimulus* stimulus);
    void (*remove_stimulus) (PsyWindow* self, PsyVisualStimulus* stimulus);

    PsyProgram* (*get_shader_program)(PsyWindow* window, PsyProgramType type);

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

G_MODULE_EXPORT PsyProgram*
psy_window_get_shader_program(PsyWindow* window, PsyProgramType type);

G_MODULE_EXPORT void
psy_window_set_projection_style(PsyWindow* window, gint projection_style);

G_MODULE_EXPORT gint
psy_window_get_projection_style(PsyWindow* window);

G_MODULE_EXPORT PsyMatrix4*
psy_window_get_projection(PsyWindow* window);


G_END_DECLS

