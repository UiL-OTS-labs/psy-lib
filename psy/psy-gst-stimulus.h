
#pragma once

#include <gst/app/gstappsink.h>
#include <gst/gst.h>

#include "psy-auditory-stimulus.h"

G_BEGIN_DECLS

#define PSY_TYPE_GST_STIMULUS psy_gst_stimulus_get_type()
G_MODULE_EXPORT
G_DECLARE_DERIVABLE_TYPE(
    PsyGstStimulus, psy_gst_stimulus, PSY, GST_STIMULUS, PsyAuditoryStimulus)

/**
 * PsyGstStimulusClass:
 * @parent: the parent of PsyGstClass
 * @create_gst_pipeline: the abstract method that should create a GstPipeline
 *                       via a deriving class. It should set the pipeline to
 *                       the instance of [class@GstStimulus]. So the derived
 *                       class is create the timeline, whereas PsyGstStimulus
 *                       is going to manage it.
 * @destroy_gst_pipeline: This destroys the pipeline in contrast to constructing
 *                        it.
 */

typedef struct _PsyGstStimulusClass {
    PsyAuditoryStimulusClass parent;

    void (*create_gst_pipeline)(PsyGstStimulus *self);
    void (*destroy_gst_pipeline)(PsyGstStimulus *self);

    gpointer reseved[12];
} PsyGstStimulusClass;

G_MODULE_EXPORT void
psy_gst_stimulus_set_running(PsyGstStimulus *self, gboolean running);

G_MODULE_EXPORT gboolean
psy_gst_stimulus_get_running(PsyGstStimulus *self);

G_END_DECLS
