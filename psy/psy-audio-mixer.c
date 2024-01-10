

#include "psy-config.h"

#include "enum-types.h"

#include "psy-audio-device.h"
#include "psy-audio-mixer.h"
#include "psy-audio-utils.h"
#include "psy-duration.h"
#include "psy-enums.h"
#include "psy-queue.h"

#define NUM_BUF_SAMPLES (65536)

/**
 * PsyAudioMixer:
 *
 * The AudioMixer is considered to be a private to Psylib in practice the
 * AudioDevice will create one instance of this class for the in and output.
 *
 * A PsyAudioMixer is a mixer that mixes the scheduled PsyAuditoryStimuli
 * together. It maintains an buffer with audio samples, Psylib must make sure
 * that this audio buffer is ready to produce samples for the audio callback.
 * the settings of the PsyAudioMixer should always match the settings
 * for the PsyAudioDevice.
 *
 * This class is used by PsyAudioDevices to buffer audio in such a way
 * that the audio callback can retrieve samples from this buffer very quickly.
 *
 * The class has a pure virtual function process_audio that should be
 * implemented by deriving classes. The function is installed as a
 * callback function that is called every roughly every ms. In the case of
 * an output mixer, the callback should ensure there is enough data to be read
 * by the audio callback. In case of an input, there should be enough space
 * in the input buffer so that the audio callback can write all its samples.
 *
 * Stability: private
 */

#define DEFAULT_NUM_STIM_CACHE 16

typedef struct _PsyAudioMixerPrivate {

    PsyAudioDevice *device;

    PsyAudioQueue *in_queue;  // The audio callback writes to input queue.
                              // So the process callback empties this.
    PsyAudioQueue *out_queue; // The audio callback reads from output queue.
                              // So the process callback fills this.
    PsyDuration *buf_dur;     // The buffer duration of the mixer.

    gint64 num_out_frames;
    gint64 num_in_frames;

    GMainContext *context;
    GPtrArray    *stimuli;
    guint         process_callback_id;
} PsyAudioMixerPrivate;

G_DEFINE_TYPE_WITH_PRIVATE(PsyAudioMixer, psy_audio_mixer, G_TYPE_OBJECT)

typedef enum {
    PROP_NULL,
    PROP_AUDIO_DEVICE,
    PROP_SAMPLE_RATE,
    PROP_BUFFER_DURATION,
    PROP_NUM_IN_CHANNELS,
    PROP_NUM_OUT_CHANNELS,
    NUM_PROPERTIES
} PsyAudioMixerProperty;

// typedef enum { STARTED, NUM_SIGNALS } PsyAudioMixerSignals;

static GParamSpec *audio_mixer_properties[NUM_PROPERTIES];

// static guint       audio_mixer_signals[NUM_SIGNALS];

/* ********** virtual(/private) functions ***************** */

static void
psy_audio_mixer_set_property(GObject      *object,
                             guint         prop_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
    PsyAudioMixer        *self = PSY_AUDIO_MIXER(object);
    PsyAudioMixerPrivate *priv = psy_audio_mixer_get_instance_private(self);

    switch ((PsyAudioMixerProperty) prop_id) {
    case PROP_AUDIO_DEVICE:
        psy_audio_mixer_set_audio_device(self, g_value_get_object(value));
        break;
    case PROP_BUFFER_DURATION:
        priv->buf_dur = g_value_get_object(value);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

static void
psy_audio_mixer_get_property(GObject    *object,
                             guint       prop_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
    PsyAudioMixer *self = PSY_AUDIO_MIXER(object);
    // PsyAudioMixerPrivate *priv = psy_audio_mixer_get_instance_private(self);

    switch ((PsyAudioMixerProperty) prop_id) {
    case PROP_AUDIO_DEVICE:
        g_value_set_object(value, psy_audio_mixer_get_audio_device(self));
        break;
    case PROP_SAMPLE_RATE:
        g_value_set_enum(value, psy_audio_mixer_get_sample_rate(self));
        break;
    case PROP_BUFFER_DURATION:
        g_value_set_object(value, psy_audio_mixer_get_buffer_duration(self));
        break;
    case PROP_NUM_IN_CHANNELS:
        g_value_set_uint(value, psy_audio_mixer_get_num_in_channels(self));
        break;
    case PROP_NUM_OUT_CHANNELS:
        g_value_set_uint(value, psy_audio_mixer_get_num_out_channels(self));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

static int
audio_mixer_call_process(gpointer data)
{
    g_assert(PSY_IS_AUDIO_MIXER(data));
    PsyAudioMixer *self = data;

    psy_audio_mixer_process_audio(self);

    return G_SOURCE_CONTINUE;
}

static void
psy_audio_mixer_init(PsyAudioMixer *self)
{
    PsyAudioMixerPrivate *priv = psy_audio_mixer_get_instance_private(self);
    priv->device               = NULL;
    priv->stimuli
        = g_ptr_array_new_full(DEFAULT_NUM_STIM_CACHE, g_object_unref);

    priv->buf_dur = psy_duration_new(.020);

    priv->process_callback_id = g_timeout_add_full(
        G_PRIORITY_HIGH, 1, audio_mixer_call_process, self, NULL);
}

static void
audio_mixer_constructed(GObject *self)
{
    G_OBJECT_CLASS(psy_audio_mixer_parent_class)->constructed(self);

    PsyAudioMixerPrivate *priv
        = psy_audio_mixer_get_instance_private(PSY_AUDIO_MIXER(self));

    guint n_in_chan
        = psy_audio_mixer_get_num_in_channels(PSY_AUDIO_MIXER(self));
    guint n_out_chan
        = psy_audio_mixer_get_num_out_channels(PSY_AUDIO_MIXER(self));

    gint64 num_frames = psy_duration_to_num_audio_frames(
        priv->buf_dur, psy_audio_device_get_sample_rate(priv->device));

    gint64 num_in_samples  = (gint64) num_frames * n_in_chan;
    gint64 num_out_samples = (gint64) num_frames * n_out_chan;

    g_return_if_fail(num_in_samples < G_MAXUINT && num_out_samples < G_MAXUINT);

    priv->in_queue  = psy_audio_queue_new(num_in_samples);
    priv->out_queue = psy_audio_queue_new(num_out_samples);

    // fill output queue, otherwise audio callback can't fetch data and will log
    // errors.
    psy_audio_mixer_process_audio(PSY_AUDIO_MIXER(self));
}

static void
psy_audio_mixer_dispose(GObject *object)
{
    PsyAudioMixer *self = PSY_AUDIO_MIXER(object);

    PsyAudioMixerPrivate *priv = psy_audio_mixer_get_instance_private(self);

    g_clear_object(&priv->device);

    if (priv->stimuli) {
        g_ptr_array_unref(priv->stimuli);
        priv->stimuli = NULL;
    }

    if (priv->process_callback_id != 0) {
        g_source_remove(priv->process_callback_id);
        priv->process_callback_id = 0;
    }

    G_OBJECT_CLASS(psy_audio_mixer_parent_class)->dispose(object);
}

static void
psy_audio_mixer_finalize(GObject *object)
{
    PsyAudioMixer        *self = PSY_AUDIO_MIXER(object);
    PsyAudioMixerPrivate *priv = psy_audio_mixer_get_instance_private(self);

    psy_audio_queue_free(priv->in_queue);
    psy_audio_queue_free(priv->out_queue);
    priv->in_queue  = NULL;
    priv->out_queue = NULL;

    G_OBJECT_CLASS(psy_audio_mixer_parent_class)->finalize(object);
}

static gboolean
stimulus_overlaps_window(gint64 window_start,
                         gint64 window_stop,
                         gint64 stim_start,
                         gint64 stim_stop)
{
    g_assert(window_start < window_stop);
    g_assert(stim_start < stim_stop);

    if (stim_stop < window_start) // stimulus should be discarded it's finished
        return FALSE;
    if (stim_start >= window_stop) // stimulus is scheduled in the future.
        return FALSE;

    return TRUE;
}

static void
audio_mixer_process_output_frames(PsyAudioMixer *self, gint64 num_frames)
{
    gfloat samples[NUM_BUF_SAMPLES] = {0};
    gfloat temp[NUM_BUF_SAMPLES];

    g_assert(num_frames >= 0);

    PsyAudioMixerPrivate *priv = psy_audio_mixer_get_instance_private(self);

    gint64 window_start, window_stop;
    guint  num_out_channels
        = psy_audio_device_get_num_output_channels(priv->device);
    const gint64 num_samples = num_out_channels * num_frames;

    if (num_frames == 0)
        return;

    if (G_UNLIKELY(num_samples > G_MAXUINT || num_samples < 0)) {
        g_critical("Unexpected number of samples");
        g_assert_not_reached();
        return;
    }

    if (G_UNLIKELY(num_samples > NUM_BUF_SAMPLES)) {
        g_critical(
            "More samples to process than %s can handle silencing output",
            G_OBJECT_CLASS_NAME(G_OBJECT_GET_CLASS(self)));
        gfloat zero = 0.0f;
        for (gint64 f = 0; f < num_frames; f++) {
            for (gint64 c = 0; c < num_out_channels; c++) {
                psy_audio_queue_push_samples(priv->out_queue, 1, &zero);
            }
        }
        return;
    }

    window_start = priv->num_out_frames;
    window_stop  = priv->num_out_frames + num_frames;

    for (guint i = 0; i < priv->stimuli->len; i++) {

        PsyAuditoryStimulus *stim = priv->stimuli->pdata[i];

        gint64 stim_start, stim_dur, stim_stop;

        stim_start = psy_auditory_stimulus_get_start_frame(stim);
        stim_dur   = psy_auditory_stimulus_get_num_frames(stim);
        stim_stop  = stim_start + stim_dur;

        // Check whether we need to do some work for this stimulus
        if (!stimulus_overlaps_window(
                window_start, window_stop, stim_start, stim_stop))
            continue;

        PsyAudioChannelMap *channel_map
            = psy_auditory_stimulus_get_channel_map(stim);

        memset(temp, 0, sizeof(temp));

        if (!channel_map) {
            g_critical("%s: Encountered PsyAuditoryStimulus without "
                       "channel_map",
                       __func__);
            continue;
        }

        gint64 stim_index_frame_start = MAX(0, stim_start - window_start);
        gint64 stim_index_sample_start
            = stim_index_frame_start * num_out_channels;
        gint64 num_stim_frames = num_frames - stim_index_frame_start;

        gint64 num_frames_read = psy_auditory_stimulus_read(
            stim, num_stim_frames, &temp[stim_index_sample_start]);

        guint num_mappings = psy_audio_channel_map_get_size(channel_map);
        for (guint map = 0; map < num_mappings; map++) {
            PsyAudioChannelMapping *mapping
                = psy_audio_channel_map_get_mapping(channel_map, map);

            const gfloat *source
                = &temp[stim_index_sample_start] + mapping->mapped_source;
            gfloat *mapped_channel
                = &samples[stim_index_sample_start] + mapping->sink_channel;

            for (; source < source + num_frames_read * num_out_channels;
                 source += num_out_channels,
                 mapped_channel += num_out_channels) {
                *mapped_channel += *source;
            }

            psy_audio_channel_mapping_free(mapping);
        }

        psy_audio_channel_map_free(channel_map);
    }

    psy_audio_queue_push_samples(priv->out_queue, num_samples, &samples[0]);
    priv->num_out_frames += num_frames;
}

static void
audio_mixer_process_audio(PsyAudioMixer *self)
{
    PsyAudioMixerPrivate *priv = psy_audio_mixer_get_instance_private(self);

    gint64 num_samples_free;
    gint64 num_frames_free;

    num_samples_free = psy_audio_queue_capacity(priv->out_queue)
                       - psy_audio_queue_size(priv->out_queue);

    num_frames_free = num_samples_free
                      / psy_audio_device_get_num_output_channels(priv->device);

    if (num_frames_free > 0) {
        audio_mixer_process_output_frames(self, num_frames_free);
    }
}

static void
psy_audio_mixer_class_init(PsyAudioMixerClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

    gobject_class->set_property = psy_audio_mixer_set_property;
    gobject_class->get_property = psy_audio_mixer_get_property;
    gobject_class->finalize     = psy_audio_mixer_finalize;
    gobject_class->dispose      = psy_audio_mixer_dispose;
    gobject_class->constructed  = audio_mixer_constructed;

    klass->process_audio = audio_mixer_process_audio;

    audio_mixer_properties[PROP_AUDIO_DEVICE]
        = g_param_spec_object("audio-device",
                              "AudioDevice",
                              "The Audio device \"attached\" to this mixer",
                              PSY_TYPE_AUDIO_DEVICE,
                              G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

    audio_mixer_properties[PROP_BUFFER_DURATION] = g_param_spec_object(
        "buffer-duration",
        "BufferDuration",
        "The duration that determines the number of samples that are buffered "
        "in the mixer.",
        PSY_TYPE_DURATION,
        G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

    audio_mixer_properties[PROP_SAMPLE_RATE]
        = g_param_spec_enum("sample-rate",
                            "SampleRate",
                            "The configured sample rate",
                            PSY_TYPE_AUDIO_SAMPLE_RATE,
                            PSY_AUDIO_SAMPLE_RATE_48000,
                            G_PARAM_READABLE);

    audio_mixer_properties[PROP_NUM_IN_CHANNELS]
        = g_param_spec_uint("num-in-channels",
                            "NumInChannels",
                            "The number of input channels for the mixer.",
                            0,
                            G_MAXUINT,
                            1,
                            G_PARAM_READABLE);

    audio_mixer_properties[PROP_NUM_OUT_CHANNELS]
        = g_param_spec_uint("num-out-channels",
                            "NumOutChannels",
                            "The number of output channels for the mixer.",
                            0,
                            G_MAXUINT,
                            1,
                            G_PARAM_READABLE);

    g_object_class_install_properties(
        gobject_class, NUM_PROPERTIES, audio_mixer_properties);
}

/* ************ public functions ******************** */

/**
 * psy_audio_mixer_new:(skip)(constructor)
 * @device: An instance of [class@AudioDevice, that is connected to this mixer.
 * @buf_dur:(transfer full): An instance of [class@Duration] that is connected
 *          to this mixer.
 *
 * Configures a new in- and output audio mixer. The audio mixer will
 * have a buffer duration close to 20ms.
 *
 * Construct a new instance of [class@PsyAudioMixer]
 */
PsyAudioMixer *
psy_audio_mixer_new(PsyAudioDevice *device, PsyDuration *buf_dur)
{
    g_assert(((GObject *) buf_dur)->ref_count == 1); // it's owned by the client
    // clang-format off
    PsyAudioMixer* mixer = g_object_new(PSY_TYPE_AUDIO_MIXER,
                        "audio-device", device,
                        "buffer-duration", buf_dur,
                        NULL);
    // clang-format on
    g_assert(((GObject *) buf_dur)->ref_count
             == 1); // Now the mixer should have taken over the reference,
                    // so the caller doesn't
                    // have to free it anymore
    return mixer;
}

/**
 * psy_audio_mixer_get_buffer_duration:
 * @self: an instance of[class@AudioMixer]
 *
 *
 * returns:(transfer none): The duration of the audio buffer managed by the
 * mixer.
 */
PsyDuration *
psy_audio_mixer_get_buffer_duration(PsyAudioMixer *self)
{
    g_return_val_if_fail(PSY_IS_AUDIO_MIXER(self), NULL);
    PsyAudioMixerPrivate *priv = psy_audio_mixer_get_instance_private(self);

    return priv->buf_dur;
}

void
psy_audio_mixer_schedule_stimulus(PsyAudioMixer       *self,
                                  PsyAuditoryStimulus *stimulus)
{
    PsyAudioMixerPrivate *priv = psy_audio_mixer_get_instance_private(self);

    PsyDuration  *buffer_duration = NULL; // from audio device
    PsyDuration  *onset_dur       = NULL; // calculated needs to be freed
    PsyTimePoint *tp_start        = NULL; // from AuditoryStimulus
    PsyTimePoint *tp_sample       = NULL; // needs to be freed. Time point
                                          // of last known sample.

    gint64 nth_sample; // last presented sample with a known time

    g_return_if_fail(PSY_IS_AUDIO_MIXER(self)
                     && PSY_IS_AUDITORY_STIMULUS(stimulus));

    if (psy_auditory_stimulus_is_scheduled(stimulus)) {
        g_warning("Stimulus is already scheduled.");
        return;
    }

    if (!psy_audio_device_get_last_known_frame(
            priv->device, &nth_sample, NULL, &tp_sample)) {
        g_critical("%s: The audio device doesn't seem to be running.",
                   __func__);
        goto fail;
    }

    buffer_duration = psy_audio_device_get_buffer_duration(priv->device);
    tp_start        = psy_stimulus_get_start_time(PSY_STIMULUS(stimulus));

    onset_dur = psy_time_point_subtract(tp_start, tp_sample);

    if (psy_duration_less(onset_dur, buffer_duration)) {
        g_warning("scheduling an auditory stimulus within %lf seconds from "
                  "last frame is to soon or in the past, presenting it as "
                  "quickly as possible.",
                  psy_duration_get_seconds(onset_dur));
    }

    gint64 num_wait_samples = psy_duration_to_num_audio_frames(
        onset_dur, psy_audio_device_get_sample_rate(priv->device));

    gint64 start_frame = nth_sample + num_wait_samples;

    psy_auditory_stimulus_set_start_frame(stimulus, start_frame);

    g_ptr_array_add(priv->stimuli, g_object_ref(stimulus));

    g_info(
        "Scheduled instance of %s at %p with the audiomixer %p, ref_frame = "
        "%ld, num_wait_frames = %ld, start_frame = %ld, wait duration = %lfs",
        G_OBJECT_CLASS_NAME(G_OBJECT_GET_CLASS(stimulus)),
        (gpointer) stimulus,
        (gpointer) self,
        nth_sample,
        start_frame,
        num_wait_samples,
        psy_duration_get_seconds(onset_dur));

fail:
    if (onset_dur) {
        psy_duration_destroy(onset_dur);
    }
    if (tp_sample) {
        psy_time_point_destroy(tp_sample);
    }
}

/**
 * psy_audio_mixer_set_audio_device:(skip):
 *
 * This function should only be during the construction of an audio mixer.
 *
 * stability:private:
 */
void
psy_audio_mixer_set_audio_device(PsyAudioMixer *self, PsyAudioDevice *device)
{
    g_return_if_fail(PSY_IS_AUDIO_MIXER(self) && PSY_IS_AUDIO_DEVICE(device));

    PsyAudioMixerPrivate *priv = psy_audio_mixer_get_instance_private(self);

    g_clear_object(&priv->device);

    priv->device = g_object_ref(device);
}

/**
 * psy_audio_mixer_get_audio_device:
 *
 * Returns:(transfer none): The AudioDevice that created this mixer.
 */
PsyAudioDevice *
psy_audio_mixer_get_audio_device(PsyAudioMixer *self)
{
    g_return_val_if_fail(PSY_IS_AUDIO_MIXER(self), NULL);
    PsyAudioMixerPrivate *priv = psy_audio_mixer_get_instance_private(self);

    return priv->device;
}

guint
psy_audio_mixer_get_num_in_channels(PsyAudioMixer *self)
{
    g_return_val_if_fail(PSY_IS_AUDIO_MIXER(self), 0);

    PsyAudioMixerPrivate *priv = psy_audio_mixer_get_instance_private(self);

    g_return_val_if_fail(PSY_IS_AUDIO_DEVICE(priv->device), 0);

    return psy_audio_device_get_num_input_channels(priv->device);
}

guint
psy_audio_mixer_get_num_out_channels(PsyAudioMixer *self)
{
    g_return_val_if_fail(PSY_IS_AUDIO_MIXER(self), 0);

    PsyAudioMixerPrivate *priv = psy_audio_mixer_get_instance_private(self);

    g_return_val_if_fail(PSY_IS_AUDIO_DEVICE(priv->device), 0);

    return psy_audio_device_get_num_output_channels(priv->device);
}

guint
psy_audio_mixer_read_frames(PsyAudioMixer *self, guint num_frames, gfloat *data)
{
    PsyAudioMixerPrivate *priv = psy_audio_mixer_get_instance_private(self);
    g_return_val_if_fail(PSY_IS_AUDIO_MIXER(self), 0);
    g_return_val_if_fail(data != NULL, 0);

    guint num_out_channels = psy_audio_mixer_get_num_out_channels(self);
    guint num_samples      = num_frames * num_out_channels;

    guint num_popped
        = psy_audio_queue_pop_samples(priv->out_queue, num_samples, data);
    return num_popped;
}

PsyAudioSampleRate
psy_audio_mixer_get_sample_rate(PsyAudioMixer *self)
{
    g_return_val_if_fail(PSY_IS_AUDIO_MIXER(self), 0);

    PsyAudioMixerPrivate *priv = psy_audio_mixer_get_instance_private(self);

    g_return_val_if_fail(PSY_IS_AUDIO_DEVICE(priv->device), 0);

    return psy_audio_device_get_sample_rate(priv->device);
}

/**
 * psy_audio_mixer_process_audio:
 * @self: An instance of [class@PsyAudioMixer]
 *
 * virtual function that should be implemented in the deriving classes
 * That maintains the buffer strategy of the mixer.
 */
void
psy_audio_mixer_process_audio(PsyAudioMixer *self)
{
    g_return_if_fail(PSY_IS_AUDIO_MIXER(self));

    PsyAudioMixerClass *cls = PSY_AUDIO_MIXER_GET_CLASS(self);

    g_return_if_fail(cls->process_audio != NULL);

    cls->process_audio(self);
}
