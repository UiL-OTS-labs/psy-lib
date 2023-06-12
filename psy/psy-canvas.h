
#pragma once

#include <glib-object.h>

#include "psy-color.h"
#include "psy-drawing-context.h"
#include "psy-enums.h"
#include "psy-image.h"
#include "psy-program.h"
#include "psy-time-point.h"
#include "psy-visual-stimulus.h"

G_BEGIN_DECLS

#define PSY_TYPE_CANVAS psy_canvas_get_type()
G_DECLARE_DERIVABLE_TYPE(PsyCanvas, psy_canvas, PSY, CANVAS, GObject)

/* Forward declaration of PsyArtist and PsyVisualStimulus*/
typedef struct _PsyArtist         PsyArtist;
typedef struct _PsyVisualStimulus PsyVisualStimulus;

typedef struct PsyFrameCount {
    guint64 num_frames;    // Number of frames successfully presented
    guint64 missed_frames; // Number of frames that we've missed.
    guint64 tot_frames;    // Total number of frames presented.
} PsyFrameCount;

/**
 * PsyCanvasClass:
 * @parent_class: The parent class
 * @set_width: Set the canvas width in pixels
 * @set_height: Set the canvas height in pixels
 * @draw: clears the canvas, than draws the stimuli.
 * @clear: A function to clear the picture to the background color
 * @draw_stimuli: A function that draws the stimuli.
 * @set_canvas_size_mm: a function used by the child to set the physical size
 *                      of the canvas that matches the size of the monitor in
 *                      mm.
 * @update_frame_stats: A function that needs to be implemented to update
 *                      The statistics after the frame has been drawn.
 * @get_canvas_size_mm: get the physical size of the canvas.
 * @set_frame_dur:set the duration of the frame
 * @get_frame_dur:get the duration of the frame
 * @schedule_stimulus:This function is called when a new stimulus is scheduled
 *                    for this canvas.
 * @create_projection_matrix: create an projection matrix that is suitable
 *                            for the current value of
 * @reset: Clears the canvas, remove scheduled stimuli and allow extra clearing
 *         in deriving classes(e.g. ImageCanvas will reset its time).
 */
typedef struct _PsyCanvasClass {
    GObjectClass parent_class;

    /*< public >*/

    void (*resize)(PsyCanvas *self, gint width, gint height);

    void (*set_width)(PsyCanvas *self, gint width);
    void (*set_height)(PsyCanvas *self, gint height);

    void (*draw)(PsyCanvas *self, guint64 frame_num, PsyTimePoint *frame_time);
    void (*clear)(PsyCanvas *self);
    void (*draw_stimuli)(PsyCanvas *self, guint64 frame_num, PsyTimePoint *tp);
    void (*draw_stimulus)(PsyCanvas *self, PsyVisualStimulus *stimulus);
    void (*update_frame_stats)(PsyCanvas *self, PsyFrameCount *stats);
    void (*init_default_shaders)(PsyCanvas *self, GError **error);

    //    void (*set_monitor_size_mm)(PsyCanvas *monitor,
    //                                gint       width_mm,
    //                                gint       height_mm);

    PsyDuration *(*get_frame_dur)(PsyCanvas *self);
    void (*set_frame_dur)(PsyCanvas *self, PsyDuration *dur);

    void (*schedule_stimulus)(PsyCanvas *self, PsyVisualStimulus *stimulus);
    void (*remove_stimulus)(PsyCanvas *self, PsyVisualStimulus *stimulus);

    PsyMatrix4 *(*create_projection_matrix)(PsyCanvas *self);
    void (*set_projection_matrix)(PsyCanvas *self, PsyMatrix4 *projection);
    void (*upload_projection_matrices)(PsyCanvas *self);

    PsyImage *(*get_image)(PsyCanvas *self);

    void (*reset)(PsyCanvas *self);

    /*< private >*/

    gpointer padding[12];
} PsyCanvasClass;

G_MODULE_EXPORT void
psy_canvas_set_background_color(PsyCanvas *self, PsyColor *color);

G_MODULE_EXPORT PsyColor *
psy_canvas_get_background_color(PsyCanvas *self);

G_MODULE_EXPORT void
psy_canvas_get_width_height_mm(PsyCanvas *self,
                               gint      *width_mm,
                               gint      *height_mm);

G_MODULE_EXPORT gint
psy_canvas_get_width_mm(PsyCanvas *self);

G_MODULE_EXPORT gint
psy_canvas_get_height_mm(PsyCanvas *self);

G_MODULE_EXPORT void
psy_canvas_set_width(PsyCanvas *self, gint width);

G_MODULE_EXPORT gint
psy_canvas_get_width(PsyCanvas *self);

G_MODULE_EXPORT void
psy_canvas_set_height(PsyCanvas *self, gint height);

G_MODULE_EXPORT gint
psy_canvas_get_height(PsyCanvas *self);

G_MODULE_EXPORT void
psy_canvas_set_width_mm(PsyCanvas *self, gint width_mm);

G_MODULE_EXPORT void
psy_canvas_set_height_mm(PsyCanvas *self, gint height_mm);

G_MODULE_EXPORT gint
psy_canvas_get_distance_mm(PsyCanvas *self);

G_MODULE_EXPORT void
psy_canvas_set_distance_mm(PsyCanvas *self, gint distance);

G_MODULE_EXPORT gfloat
psy_canvas_get_width_vd(PsyCanvas *self);

G_MODULE_EXPORT gfloat
psy_canvas_get_height_vd(PsyCanvas *self);

G_MODULE_EXPORT void
psy_canvas_schedule_stimulus(PsyCanvas *self, PsyVisualStimulus *stimulus);

G_MODULE_EXPORT PsyDuration *
psy_canvas_get_frame_dur(PsyCanvas *self);

G_MODULE_EXPORT void
psy_canvas_set_frame_dur(PsyCanvas *self, PsyDuration *dur);

G_MODULE_EXPORT void
psy_canvas_remove_stimulus(PsyCanvas *self, PsyVisualStimulus *stimulus);

G_MODULE_EXPORT void
psy_canvas_set_projection_style(PsyCanvas *self, gint projection_style);

G_MODULE_EXPORT gint
psy_canvas_get_projection_style(PsyCanvas *self);

G_MODULE_EXPORT PsyMatrix4 *
psy_canvas_get_projection(PsyCanvas *self);

G_MODULE_EXPORT void
psy_canvas_set_context(PsyCanvas *self, PsyDrawingContext *context);

G_MODULE_EXPORT PsyDrawingContext *
psy_canvas_get_context(PsyCanvas *self);

G_MODULE_EXPORT void
psy_canvas_swap_stimuli(PsyCanvas *self, guint i1, guint i2);

G_MODULE_EXPORT guint
psy_canvas_get_num_stimuli(PsyCanvas *self);

G_MODULE_EXPORT PsyImage *
psy_canvas_get_image(PsyCanvas *self);

G_MODULE_EXPORT gint64
psy_canvas_get_num_frames(PsyCanvas *self);

G_MODULE_EXPORT gint64
psy_canvas_get_num_frames_missed(PsyCanvas *self);

G_MODULE_EXPORT gint64
psy_canvas_get_num_frames_total(PsyCanvas *self);

G_MODULE_EXPORT void
psy_canvas_reset(PsyCanvas *self);

G_END_DECLS
