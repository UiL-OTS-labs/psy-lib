
#include <gst/app/gstappsink.h>
#include <gst/gst.h>

#include "enum-types.h"
#include "psy-audio-device.h"
#include "psy-wave.h"

/**
 * PsyWave:
 *
 * PsyWave are instance that generate WaveForm. This is the class to use
 * when you would like to present a special generated wave form.
 * Psylib is able to generate pure tones using [property@Wave:wave-form] you
 * may set it to example #PSY_WAVE_FORM_SINE or
 * #PSY_WAVE_FORM_WHITE_UNIFORM_NOISE
 */

typedef struct _PsyWave {
    PsyGstStimulus parent;
    PsyWaveForm    wave_form;
    gdouble        volume;
    gdouble        freq;
} PsyWave;

G_DEFINE_FINAL_TYPE(PsyWave, psy_wave, PSY_TYPE_GST_STIMULUS)

typedef enum {
    PROP_NULL,
    PROP_WAVE_FORM,
    PROP_VOLUME,
    PROP_FREQ,
    NUM_PROPS
} PsyWaveProperty;

// GObject stuff

static GParamSpec *wave_properties[NUM_PROPS];

static void
wave_set_property(GObject      *object,
                  guint         property_id,
                  const GValue *value,
                  GParamSpec   *pspec)
{
    PsyWave *self = PSY_WAVE(object);

    switch ((PsyWaveProperty) property_id) {
    case PROP_WAVE_FORM:
        psy_wave_set_form(self, g_value_get_enum(value));
        break;
    case PROP_VOLUME:
        psy_wave_set_volume(self, g_value_get_double(value));
        break;
    case PROP_FREQ:
        psy_wave_set_freq(self, g_value_get_double(value));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

static void
wave_get_property(GObject    *object,
                  guint       property_id,
                  GValue     *value,
                  GParamSpec *pspec)
{
    PsyWave *self = PSY_WAVE(object);

    switch ((PsyWaveProperty) property_id) {
    case PROP_WAVE_FORM:
        g_value_set_enum(value, self->wave_form);
        break;
    case PROP_VOLUME:
        g_value_set_double(value, self->volume);
        break;
    case PROP_FREQ:
        g_value_set_double(value, self->freq);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

// PsyAuditoryStimulus stuff

static gboolean
wave_get_flexible_num_channels(void)
{
    return TRUE;
}

// PsyGstStimulus stuff

/**
 * calculate_num_buffers:
 * @samples_per_buffer: the number of samples "decoded"/generated per buffer.
 * @num_desired_samples:
 *      The total number of samples that should be generated. If negative,
 *      -1 is returned:
 *
 * Calculates the minimal number of buffer to reach the duration of the
 * stimulus. So in most cases, the number of buffers will produce a number
 * of samples that exceeds the total number of buffers. So the last
 * presented buffer should should be smaller than the preceding buffers
 * to get to the desired number of samples
 *
 * Returns the number of buffers desired or -1 for an infinite number.
 */
static gint64
calculate_num_buffers(gint64 samples_per_buffer, gint64 num_desired_samples)
{
    if (num_desired_samples < 0)
        return -1;

    gint num_buffers = num_desired_samples / samples_per_buffer;
    if (num_buffers * samples_per_buffer < num_desired_samples)
        return num_buffers + 1;
    else
        return num_buffers;
}

static void
wave_set_source_properties(PsyWave *self, GstElement *source)
{
    double freq      = self->freq;
    int    wave_form = self->wave_form;
    double volume    = self->volume;

    // clang-format off
    g_object_set(source,
                 "volume", volume,
                 "wave", wave_form,
                 "freq", freq,
                 NULL);
    // clang-format on
}

static void
wave_create_gst_pipeline(PsyGstStimulus *self)
{
    PsyWave *wave_self          = PSY_WAVE(self);
    guint    samples_per_buffer = 1024;

    PsyAudioDevice *device
        = psy_auditory_stimulus_get_audio_device(PSY_AUDITORY_STIMULUS(self));
    if (!device) {
        g_critical("The PsyAuditoryStimulus isn't attached to an audio device");
        return;
    }
    gint  sample_rate = psy_audio_device_get_sample_rate(device);
    guint channels
        = psy_auditory_stimulus_get_num_channels(PSY_AUDITORY_STIMULUS(self));

    g_return_if_fail(channels <= G_MAXINT);

    if (channels < 1 || channels > G_MAXINT) {
        g_critical(
            "The PsyAuditoryStimulus num channels %d is to large or incorrect",
            channels);
        return;
    }
    gint ichannels = (gint) channels;

    // clang-format off
    GstCaps *caps = gst_caps_new_simple("audio/x-raw",
                                        "format", G_TYPE_STRING, "F32LE",
                                        "layout", G_TYPE_STRING, "interleaved",
                                        "rate", G_TYPE_INT, sample_rate,
                                        "channels", G_TYPE_INT, ichannels,
                                        NULL);
    // clang-format on

    GstElement *pipeline = gst_pipeline_new("wave-pipeline");

    GstElement *source = gst_element_factory_make("audiotestsrc", "source");
    GstElement *sink   = gst_element_factory_make("appsink", "sink");

    wave_set_source_properties(wave_self, source);

#ifndef NDEBUG
    g_assert(g_object_is_floating(sink));
#endif

    if (!pipeline || !source || !sink) { // Oops unable to create all elements
        g_critical("Unable to create gst elements for a PsyWave");
        g_clear_object(&pipeline);
        g_clear_object(&source);
        g_clear_object(&sink);
        return;
    }

    GstAppSink *appsink = GST_APP_SINK(sink); // just a cast.
    gst_app_sink_set_caps(appsink, caps);

    // This should sink the reference.
    gst_bin_add_many(GST_BIN(pipeline), sink, source, NULL);

    if (gst_element_link(source, sink) != TRUE) {
        g_critical("Unable to link source to sink");
        gst_object_unref(pipeline);
        return;
    }

#ifndef NDEBUG
    GObject *gpipeline = G_OBJECT(pipeline);
    GObject *gappsink  = G_OBJECT(pipeline);

    g_assert(gpipeline->ref_count == 1);
    g_assert(gappsink->ref_count == 1);
#else
#endif

    gint64 num_frames
        = psy_auditory_stimulus_get_num_frames(PSY_AUDITORY_STIMULUS(self));
    if (num_frames < 0) {
        g_critical("The stimulus has a negative number of frames: %ld",
                   num_frames);
        return;
    }

    // clang-format off
    g_object_set(self,
            "pipeline", pipeline,
            "app-sink", appsink,
            NULL);
    // clang-format on

    gint64 num_buffers = calculate_num_buffers(samples_per_buffer, num_frames);

    if (num_buffers > G_MAXINT) {
        num_buffers = G_MAXINT;
    }

    // clang-format off
    g_object_set(source,
            "volume", PSY_WAVE(self)->volume,
            "samplesperbuffer", samples_per_buffer,
            "num-buffers", (int)num_buffers,
            NULL);
    // clang-format on

#ifndef NDEBUG
    // only PsyGstStimulus should hold a reference
    g_assert(gpipeline->ref_count == 1);
    // The pipeline should hold only owns a reference
    g_assert(gappsink->ref_count == 1);
#else
#endif

    PSY_GST_STIMULUS_CLASS(psy_wave_parent_class)->create_gst_pipeline(self);
}

// Instance stuff

static void
psy_wave_init(PsyWave *self)
{
    self->freq      = 440.0;
    self->volume    = .5;
    self->wave_form = PSY_WAVE_FORM_SINE;
}

static void
psy_wave_class_init(PsyWaveClass *klass)
{
    GObjectClass *obj_class = G_OBJECT_CLASS(klass);

    obj_class->set_property = wave_set_property;
    obj_class->get_property = wave_get_property;

    PsyAuditoryStimulusClass *as_class  = PSY_AUDITORY_STIMULUS_CLASS(klass);
    as_class->get_flexible_num_channels = wave_get_flexible_num_channels;

    PsyGstStimulusClass *gst_class = PSY_GST_STIMULUS_CLASS(klass);
    gst_class->create_gst_pipeline = wave_create_gst_pipeline;

    /**
     * PsyWave:wave-form:
     *
     * The type of wave that is being generated. There are pure tones
     * available but also types of noise. See [enum@PsyWaveForm]
     */
    wave_properties[PROP_WAVE_FORM]
        = g_param_spec_enum("wave-form",
                            "WaveForm",
                            "The type of wave form that is being generated",
                            PSY_TYPE_WAVE_FORM,
                            PSY_WAVE_FORM_SINE,
                            G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

    /**
     * PsyWave:volume:
     *
     * This value reflects the volume of the generated audio. A volume of
     * 1.0 is full volume without clipping for a Sine wave. For the
     * gaussian white noise form, the volume is used a the standard
     * deviviation so the a volume of 0.1 has a standard deviation of 0.1.
     * So in case of the GAUSSIAN noise, you probalbly don't want to set the
     * volume to 1.0.
     */
    wave_properties[PROP_VOLUME]
        = g_param_spec_double("volume",
                              "Volume",
                              "The volume at which the waveform is generated",
                              0,
                              1.0,
                              0.5,
                              G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

    /**
     * PsyWave:freq:
     *
     * This value reflects the frequency of the generated audio. So
     * It's only relevant for the SINE wave-form. You' can set very
     * high frequencies, but not that a frequecy higher that the
     * [method.PsyAudioDevice.get_sample_rate()/2 is not possible, you'll
     * run into artifacts.
     */
    wave_properties[PROP_FREQ]
        = g_param_spec_double("freq",
                              "frequency",
                              "The frequency of the SINE wave",
                              0,
                              G_MAXDOUBLE,
                              440.0,
                              G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

    g_object_class_install_properties(obj_class, NUM_PROPS, wave_properties);
}

// Public functions

/**
 * psy_wave_new:(constructor):
 * @device: The instance of PsyAudioDevice on which to play this stimulus
 *
 * Construct an instance of [class@Wave] with default parameters. This
 * will play an pure tone with a volume of .5 with a frequency of
 * 440 Hz.
 *
 * Returns: A new object with default parameters free with g_object_unref or
 *          [method@Wave.free].
 */
PsyWave *
psy_wave_new(PsyAudioDevice *device)
{
    PsyWave *wave = g_object_new(PSY_TYPE_WAVE, "audio-device", device, NULL);
    return wave;
}

/**
 * psy_wave_new_volume:(constructor)
 * @device: The audio device on which to play this stimulus
 * @volume: the desired volume of the waveform should be 0.0 <= volume
 * <= 1.0
 *
 * Construct an instance of [class@Wave] with default parameters. This
 * will play an pure tone with a volume of .5 with a frequency of
 * 440 Hz.
 *
 * Returns: A new instance with representing a tone with specified volume
 * with g_object_unref or [method@Wave.free].
 */
PsyWave *
psy_wave_new_volume(PsyAudioDevice *device, gdouble volume)
{
    // clang-format off
    PsyWave *wave = g_object_new(PSY_TYPE_WAVE,
            "audio-device", device,
            "volume", volume,
            NULL);
    // clang-format on
    return wave;
}

/**
 * psy_wave_tone_new:(constructor)
 * @device: The audio device on which to play this stimulus
 * @hz: the desired frequency, should be 0 < freq <= sample_rate / 2
 * @volume: the desired volume of the waveform should be 0.0 <= volume
 * <= 1.0
 *
 * Construct an instance of [class@Wave] with a sine wave. This
 * will play an pure tone with a specified volume with the specified
 * frequency You should take care that it should have a frequency that can
 * be played with an audio device, typically, audio devices have troubles
 * with low frequency e.g. < 100 Hz and cannot possibly represent a tone wit
 * a frequency higher that half the sampling rate.
 *
 * Returns: A new instance with representing a tone with specified volume
 * with g_object_unref or [method@Wave.free]
 */
PsyWave *
psy_wave_tone_new(PsyAudioDevice *device, gdouble hz, gdouble volume)
{
    // clang-format off
    PsyWave *wave = g_object_new(
            PSY_TYPE_WAVE,
            "audio-device", device,
            "wave-form", PSY_WAVE_FORM_SINE,
            "freq", hz,
            "volume", volume,
            NULL);
    // clang-format on

    return wave;
}

/**
 * psy_wave_free: (skip)
 *
 * frees instances of [class@Wave]
 */
void
psy_wave_free(PsyWave *self)
{
    g_return_if_fail(PSY_IS_WAVE(self));
    g_object_unref(self);
}

/**
 * psy_wave_get_volume:
 * @self: an instance of [class@Wave]
 *
 * Returns: the volume of this wave form
 */
gdouble
psy_wave_get_volume(PsyWave *self)
{
    g_return_val_if_fail(PSY_IS_WAVE(self), 0);
    return self->volume;
}

/**
 * psy_wave_set_volume:
 * @self: an instance of [class@Wave]
 * @volume: The desired volume for this wave form valid range is
 *          0 <= volume <= 1.0.
 *
 * Sets the volume of this wave form. The volume should be 0 <= volume
 * <= 1.0.
 */
void
psy_wave_set_volume(PsyWave *self, gdouble volume)
{
    g_return_if_fail(PSY_IS_WAVE(self));

    g_warn_if_fail(volume >= 0 && volume <= 1.0);

    self->volume = CLAMP(volume, 0.0, 1.0);
}

/**
 * psy_wave_get_freq:
 * @self: an instance of [class@Wave]
 *
 * Returns: the frequency of this wave form
 */
gdouble
psy_wave_get_freq(PsyWave *self)
{
    g_return_val_if_fail(PSY_IS_WAVE(self), 0);
    return self->freq;
}

/**
 * psy_wave_set_freq:
 * @self: an instance of [class@Wave]
 * @freq: a value between 0 and sampe_rate/2.0
 *
 * Sets the freq of this wave form. The freq should be 0 <= freq <=
 * [method@PsyAudioDevice.get_sample_rate()/2.
 */
void
psy_wave_set_freq(PsyWave *self, gdouble freq)
{
    g_return_if_fail(PSY_IS_WAVE(self));

    gdouble         max_sr;
    PsyAudioDevice *device
        = psy_auditory_stimulus_get_audio_device(PSY_AUDITORY_STIMULUS(self));

    g_return_if_fail(device != NULL);

    max_sr = psy_audio_device_get_sample_rate(device);

    g_warn_if_fail(freq >= 0 && freq <= max_sr);

    self->freq = CLAMP(freq, 0.0, max_sr);
}

/**
 * psy_wave_get_form:
 * @self: an instance of [class@Wave]
 *
 * Returns: the waveform type of audio wave of this instance
 */
PsyWaveForm
psy_wave_get_form(PsyWave *self)
{
    g_return_val_if_fail(PSY_IS_WAVE(self), PSY_WAVE_FORM_SINE);
    return self->wave_form;
}

/**
 * psy_wave_set_form:
 * @self: an instance of [class@Wave]
 *
 * Sets the form of this wave. The form set here is used to shape the form
 * that will be put to the AudioOutputDevice that is linked to this
 * stimulus. See [enum@PsyWaveForm] for more details about the optional wave
 * forms psylib knows about.
 */
void
psy_wave_set_form(PsyWave *self, PsyWaveForm form)
{
    g_return_if_fail(PSY_IS_WAVE(self));

    self->wave_form = form;
}
