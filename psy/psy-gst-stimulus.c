
#include <gst/gst.h>

#include <gst/app/gstappsink.h>

#include "psy-gst-stimulus.h"

/**
 * PsyGstStimulus:
 *
 * A PsyGstStimulus is an abstract class. This is a base class for
 * deriving Stimulus classes that use gstreamer in order to create
 * decoded media.
 * This class will manage a GstPipeline, that should be created by
 * deriving children. The pipeline should be created by the
 * virtual create_gst_pipeline function. This class will start it and obtain
 * metadata in order to make sure the right parameters are set to the
 * PsyAuditoryStimulus it's not logical for the End user to know this.
 * E.g. The end user might be able to specify a duration for a generated
 * wave form, it not likely he/she knows the duration of an audiofile
 * and in this situation, the instance of PsyGstStimulus will set the duration.
 * This instance will handle reading from the right GstAppsink in the media
 * source primarily to obtain decoded audio that matches the sample rate of
 * the PsyAudiodevice that this instance is linked to and it makes sure
 * we'll obtain the media in 32bit floating points.
 */

typedef struct PsyGstStimulusPrivate {
    GstPipeline *pipeline;
    GstAppSink  *app_sink; // should be part of pipeline.
    gboolean     running;
} PsyGstStimulusPrivate;

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE(PsyGstStimulus,
                                    psy_gst_stimulus,
                                    PSY_TYPE_AUDITORY_STIMULUS)

typedef enum {
    PROP_NULL,
    PROP_PIPELINE,
    PROP_APP_SINK,
    PROP_RUNNING,
    NUM_PROPERTIES
} PsyGstStimulusProperty;

static GParamSpec *gst_stimulus_properties[NUM_PROPERTIES] = {NULL};

// Forward declarations
void
psy_gst_stimulus_set_pipeline(PsyGstStimulus *self, GstPipeline *pipeline);
void
psy_gst_stimulus_set_app_sink(PsyGstStimulus *self, GstAppSink *app_sink);

static void
gst_stimulus_set_property(GObject      *object,
                          guint         property_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
    PsyGstStimulus *self = PSY_GST_STIMULUS(object);

    switch ((PsyGstStimulusProperty) property_id) {
    case PROP_PIPELINE:
        psy_gst_stimulus_set_pipeline(self, g_value_get_object(value));
        break;
    case PROP_APP_SINK:
        psy_gst_stimulus_set_app_sink(self, g_value_get_object(value));
        break;
    case PROP_RUNNING:
        psy_gst_stimulus_set_running(self, g_value_get_boolean(value));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

static void
gst_stimulus_get_property(GObject    *object,
                          guint       property_id,
                          GValue     *value,
                          GParamSpec *pspec)
{
    PsyGstStimulus        *self = PSY_GST_STIMULUS(object);
    PsyGstStimulusPrivate *priv = psy_gst_stimulus_get_instance_private(self);

    switch ((PsyGstStimulusProperty) property_id) {
    case PROP_PIPELINE:
        g_value_set_object(value, priv->pipeline);
        break;
    case PROP_APP_SINK:
        g_value_set_object(value, priv->app_sink);
        break;
    case PROP_RUNNING:
        g_value_set_boolean(value, priv->running);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

static void
psy_gst_stimulus_init(PsyGstStimulus *self)
{
    PsyGstStimulusPrivate *priv = psy_gst_stimulus_get_instance_private(self);
    priv->pipeline              = NULL;
}

static void
gst_stimulus_dispose(GObject *self)
{
    PsyGstStimulusPrivate *priv
        = psy_gst_stimulus_get_instance_private(PSY_GST_STIMULUS(self));

    g_clear_object(&priv->pipeline); // This will unref the appsink.
    priv->app_sink = NULL;           // we'll mark it as NULL anyway.

    G_OBJECT_CLASS(psy_gst_stimulus_parent_class)->dispose(self);
}

static void
gst_stimulus_create_pipeline(PsyGstStimulus *self)
{
    PsyGstStimulusPrivate *priv = psy_gst_stimulus_get_instance_private(self);

    g_assert(priv->pipeline);
    g_assert(priv->app_sink);

    priv->running = (priv->pipeline != NULL && priv->app_sink != NULL);
}

static void
gst_stimulus_destroy_pipeline(PsyGstStimulus *self)
{
    PsyGstStimulusPrivate *priv = psy_gst_stimulus_get_instance_private(self);

    gst_object_unref(priv->pipeline);

    priv->app_sink = NULL;
    priv->pipeline = NULL;

    priv->running = FALSE;
}

static void
psy_gst_stimulus_class_init(PsyGstStimulusClass *klass)
{
    GObjectClass *obj_class = G_OBJECT_CLASS(klass);

    obj_class->set_property = gst_stimulus_set_property;
    obj_class->get_property = gst_stimulus_get_property;
    obj_class->dispose      = gst_stimulus_dispose;

    klass->create_gst_pipeline  = gst_stimulus_create_pipeline;
    klass->destroy_gst_pipeline = gst_stimulus_destroy_pipeline;

    /**
     * PsyGstStimulus:pipeline:(transfer full):
     *
     * This property holds the pipeline that finally yields the media
     * data/samples of a media (generated) source. The pipeline is
     * created and set by the PsyGstStimulus.create_gst_pipeline
     */
    gst_stimulus_properties[PROP_PIPELINE] = g_param_spec_object(
        "pipeline",
        "Pipeline",
        "The pipeline that yields the source of the audio",
        GST_TYPE_PIPELINE,
        G_PARAM_READWRITE);

    /**
     * PsyGstStimulus:app-sink:(transfer none):
     *
     * The app sink, this is the Gstreamer element from which GstSamples
     * can be obtains, they will contain the actual data representing the
     * raw audio. The app-sink should be inside
     * [property@PsyGstStimulus:pipeline], so this reference is only valid
     * as long as the GstPipeline is holding the appsink.
     */
    gst_stimulus_properties[PROP_APP_SINK] = g_param_spec_object(
        "app-sink",
        "Appsink",
        "The sink that will yield the audio samples for this stimulus",
        GST_TYPE_APP_SINK,
        G_PARAM_READWRITE);

    /**
     * PsyGstStimulus:running
     *
     * This boolean reflects whether the pipeline is running. When the
     * PsyGstStimulus is set to running, the pipeline will start to
     * decode/generate the audio in the background. Then it should be possible
     * to fetch the audio quickly.
     */
    gst_stimulus_properties[PROP_RUNNING]
        = g_param_spec_boolean("running",
                               "Running",
                               "This boolean is set/can be set to start the "
                               "internal pipeline that generates the audio.",
                               FALSE,
                               G_PARAM_READWRITE);

    g_object_class_install_properties(
        obj_class, NUM_PROPERTIES, gst_stimulus_properties);
}

/**
 * psy_gst_stimulus_create_pipeline:
 * @self:
 *
 * The method that will create the GstPipeline use by this instance. This
 * instance will need an pipeline in order to manage and retrieve information
 * about the media.
 *
 * Returns: TRUE when successful, FALSE otherwise.
 */
gboolean
psy_gst_stimulus_create_pipeline(PsyGstStimulus *self)
{
    g_return_val_if_fail(PSY_IS_GST_STIMULUS(self), FALSE);

    PsyGstStimulusClass *cls = PSY_GST_STIMULUS_GET_CLASS(self);

    g_return_val_if_fail(cls->create_gst_pipeline != NULL, FALSE);

    cls->create_gst_pipeline(self);
    return TRUE;
}

/**
 * psy_gst_stimulus_set_pipeline:
 * @self: an instance of [class@GstStimulus]
 * @pipeline:(transfer full): The pipeline managed by this instance of
 *                            PsyGstStimulus
 *
 * Set the pipeline of the stimulus, the pipeline should be created by a
 * derived class in the [method@PsyGstStimulus.create_gst_pipeline
 * The instance of [class@PsyGstStimulus] will manage and free the
 * pipeline on its own destruction.
 */
void
psy_gst_stimulus_set_pipeline(PsyGstStimulus *self, GstPipeline *pipeline)
{
    PsyGstStimulusPrivate *priv = psy_gst_stimulus_get_instance_private(self);

    g_return_if_fail(PSY_IS_GST_STIMULUS(self));
    g_return_if_fail(GST_IS_PIPELINE(pipeline));

    if (priv->pipeline)
        g_object_unref(priv->pipeline);
    priv->pipeline = pipeline;
}

/**
 * psy_gst_stimulus_set_app_sink:
 * @self:an instance of [class@GstStimulus]
 * @app_sink:(transfer none): An app sink that should be in the pipeline
 *
 * The appsink where psylib will read raw media from. The appsink
 * should contained by the pipeline, and hence a pipeline should be set.
 */
void
psy_gst_stimulus_set_app_sink(PsyGstStimulus *self, GstAppSink *app_sink)
{
    g_return_if_fail(PSY_IS_GST_STIMULUS(self));
    g_return_if_fail(GST_IS_APP_SINK(app_sink));

    PsyGstStimulusPrivate *priv = psy_gst_stimulus_get_instance_private(self);

    priv->app_sink = app_sink;
}

/**
 * psy_gst_stimulus_set_running:
 * @self: an instance of [class@PsyGstStimulus]
 * @running: Whether you want to set the pipeline to running or not
 *
 * An instance of [class@PsyGstStimulus] has a gstreamer pipeline when
 * it is ready to do its work. The pipeline should be running in order to
 * function properly.
 * You should set all properties that are relevant for creating the pipeline
 * before you set it to running. Some deriving classes need to know the
 * frequency of a wave form, others might need a filename in order to
 * get the audio from a file.
 * When you are done setting the required parameters, you can set the running
 * property to true and it will start the pipeline.
 */
void
psy_gst_stimulus_set_running(PsyGstStimulus *self, gboolean running)
{
    PsyGstStimulusPrivate *priv = psy_gst_stimulus_get_instance_private(self);
    g_return_if_fail(PSY_IS_GST_STIMULUS(self));

    if (running == priv->running)
        return;

    PsyGstStimulusClass *cls = PSY_GST_STIMULUS_GET_CLASS(self);

    g_return_if_fail(cls->create_gst_pipeline != NULL
                     && cls->destroy_gst_pipeline != NULL);

    if (running) {
        cls->create_gst_pipeline(self);
        // Deriving class should provide a pipeline and appsink
        if (priv->running) {
            g_assert(priv->pipeline);
            g_assert(priv->app_sink);
        }
    }
    else {
        cls->destroy_gst_pipeline(self);
    }
}

/**
 * psy_gst_stimulus_get_running:
 * @self: an instance of [class@GstStimulus]
 *
 * Check whether the stimulus is running the pipeline to generate the audio.
 */
gboolean
psy_gst_stimulus_get_running(PsyGstStimulus *self)
{
    PsyGstStimulusPrivate *priv = psy_gst_stimulus_get_instance_private(self);
    g_return_val_if_fail(PSY_IS_GST_STIMULUS(self), FALSE);

    return priv->running;
}
