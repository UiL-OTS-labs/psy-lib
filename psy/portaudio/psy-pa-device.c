
#include "psy-config.h"

#include <math.h>
#include <portaudio.h>
#if defined HAVE_PA_LINUX_ALSA_H
    #include <pa_linux_alsa.h>
#endif
#include <stdio.h>

#include "enum-types.h"
#include "psy-audio-mixer.h"
#include "psy-clock.h"
#include "psy-enums.h"
#include "psy-pa-device.h"

typedef struct {
    GMutex lock;
    gint64 num_frames;
    PaTime current_stream_time;
    PaTime time_input;
    PaTime time_output;
} LastFrameInfo;

/**
 * PsyPADevice:
 *
 * PsyPADevice is a device that uses portaudio to implement a PsyAudioDevice.
 */
typedef struct _PsyPADevice {
    PsyAudioDevice       parent;
    LastFrameInfo        last_frame;
    PaStream            *stream;
    PsyClock            *clk;
    PsyDuration         *clock_offset; // pa_time - psy_clock_now()
    gboolean             pa_initialized;
    PsyAudioDeviceInfo **dev_infos;
    guint                num_infos;
} PsyPADevice;

G_DEFINE_FINAL_TYPE(PsyPADevice, psy_pa_device, PSY_TYPE_AUDIO_DEVICE)

typedef enum { PROP_NULL, NUM_PROPERTIES } PsyPADeviceProperty;

//
// static GParamSpec *pa_device_properties[NUM_PROPERTIES];

#ifdef __linux__
PaHostApiTypeId g_supported_apis[] = {paALSA};
#elif WIN32
PaHostApiTypeId g_supported_apis[] = {paASIO, paWASAPI};
#else
    #error "Currently unsupported platform"
#endif

/* *************** private methods ************** */

/**
 * pa_audio_callback:(skip)
 *
 * The audio callback for the portaudio backend
 *
 * stability:private
 */
static int
pa_audio_callback(const void                     *input,
                  void                           *output,
                  unsigned long                   frame_count,
                  const PaStreamCallbackTimeInfo *timeInfo,
                  PaStreamCallbackFlags           statusFlags,
                  void                           *audio_device)
{
    (void) input;
    (void) timeInfo;
    (void) statusFlags;

    //    g_info("frame_count %lu, stream_time = %lf, output time = %lf",
    //           frame_count,
    //           timeInfo->currentTime,
    //           timeInfo->outputBufferDacTime);

    gboolean locked = FALSE;

    PsyPADevice   *self = audio_device;
    guint          num_out_channels;
    PsyAudioMixer *mixer;

    locked = g_mutex_trylock(&self->last_frame.lock);
    if (locked) {
        self->last_frame.current_stream_time = timeInfo->currentTime;
        self->last_frame.time_input          = timeInfo->inputBufferAdcTime;
        self->last_frame.time_output         = timeInfo->outputBufferDacTime;
        self->last_frame.num_frames
            = psy_audio_device_get_current_frame_count(PSY_AUDIO_DEVICE(self));
        g_mutex_unlock(&self->last_frame.lock);
    }

    num_out_channels
        = psy_audio_device_get_num_output_channels(PSY_AUDIO_DEVICE(self));

    guint num_out_floats = num_out_channels * frame_count;
    mixer                = psy_audio_device_get_mixer(PSY_AUDIO_DEVICE(self));

    // TODO Read from input here.

    // Save's our ears when there is something wrong reading from output mixer.
    memset(output, 0, num_out_floats * sizeof(float));

    if (output != NULL && frame_count > 0) {
        guint num_read
            = psy_audio_mixer_read_frames(mixer, frame_count, output);
        if (G_UNLIKELY(num_read != num_out_floats)) {
            g_critical(
                "%s, Unable to read: %u samples (got %u) from the output mixer",
                __func__,
                num_out_floats,
                num_read);
        }
    }

    if (G_UNLIKELY(frame_count >= G_MAXINT)) {
        g_critical("unexpected large frame_count %ld", frame_count);
        return paAbort;
    }

    psy_audio_device_update_frame_count(PSY_AUDIO_DEVICE(self),
                                        (gint) frame_count);

    return paContinue;
}

/**
 * pa_clear_last_time_info:
 * @self: An instance of PsyPADevice
 *
 * Clears the info of the frames.
 */
static void
pa_clear_last_frame_info(PsyPADevice *self)
{
    self->last_frame.current_stream_time = 0;
    self->last_frame.num_frames          = 0;
    self->last_frame.time_input          = 0;
    self->last_frame.current_stream_time = 0;
}

/**
 * pa_time_to_psy_timepoint:
 * @time: A stream time from Pa_GetStreamTime().
 *
 * This function converts a PaTime to a PsyTimePoint. The returned timepoint
 * has probably an offset to an timepoint returned by [method@Clock.now]
 * hence it should be transformed to a time comparable with the time
 * reported by [method@Clock.now].
 *
 * Stability: private
 * Returns: An instance of [class@PsyTimePoint], the timepoint returned is an
 *          timepoint that is a timepoint with the origin of in Port Audio.
 */
static PsyTimePoint *
pa_time_to_psy_timepoint(PaTime time)
{
    PsyTimePoint *tp_null        = psy_time_point_new();
    PsyDuration  *pa_running_dur = psy_duration_new(time);

    PsyTimePoint *tp_psy = psy_time_point_add(tp_null, pa_running_dur);

    g_object_unref(tp_null);
    g_object_unref(pa_running_dur);
    return tp_psy;
}

/**
 * pa_time_calculate_clock_offset:
 * @self: an instance of PsyPADevice*
 * @tp_pa: An timepoint obtained from pa_time_to_psy_timepoint
 *
 * Calculates the offset between the PsyClock and Pa_GetStreamTime()
 * and stores the offset in @self.
 *
 * Stability: Private
 */
static void
pa_time_calculate_clock_offset(PsyPADevice *self, PsyTimePoint *tp_pa)
{
    PsyTimePoint *tp_now = psy_clock_now(self->clk);
    g_clear_object(&self->clock_offset);
    self->clock_offset = psy_time_point_subtract(tp_now, tp_pa);
    g_debug("The clock offset is roughly %lf s",
            psy_duration_get_seconds(self->clock_offset));
    g_object_unref(tp_now);
}

/**
 * pa_transform_pa_time_to_psy_time:
 *
 * Transfers the portaudio time to psylib's time
 *
 * Stability: Private
 * Returns:(transfer full): The pa timepoint to an timepoint comparable with
 *    [class@Clock]'s timepoints.
 */
static PsyTimePoint *
pa_transform_pa_time_to_psy_time(PsyPADevice *self, PsyTimePoint *tp_pa)
{
    return psy_time_point_add(tp_pa, self->clock_offset);
}

/**
 * pa_is_pcm_device:
 *
 * This function tries to determine whether it is a pcm device, e.g. ALSA
 * virtual devices are ignored.
 *
 * TODO check for windows devices whether they are PCM devices
 * Stability: Private
 */
static gboolean
pa_is_pcm_device(const PaDeviceInfo *info)
{
    gboolean ret = TRUE;
    if (info->hostApi == Pa_HostApiTypeIdToHostApiIndex(paALSA)) {

        const char *re_string = "(hw:\\d+,\\d+)";

        if (!g_regex_match_simple(re_string, info->name, 0, 0))
            ret = FALSE;
    }

    return ret;
}

/**
 * pa_device_uses_preferred_host_api:
 *
 * Checks whether the host api is supported:
 * linux: ALSA
 * windows: (WASAPI, ASIO) // TODO determine the preferrable host api('s)
 * MAC: (coreaudio) // TODO
 */
static gboolean
pa_device_uses_preferred_host_api(const PaDeviceInfo *info)
{
    const size_t num_preferred
        = sizeof(g_supported_apis) / sizeof(g_supported_apis[0]);

    for (size_t i = 0; i < num_preferred; i++) {
        if (info->hostApi
            == Pa_HostApiTypeIdToHostApiIndex(g_supported_apis[i])) {
            return TRUE;
        }
    }
    return FALSE;
}

/**
 * pa_get_sample_rates:
 * @info: a PaDeviceInfo for a specific device
 * @idx: the PaDeviceIndex for a specific device should match info.
 * @sample_rates:(out) (transfer full) (array length=num_sample_rates): the
 *              output is returned here.
 * @num_sample_rates:(out): the length of the output
 *
 * Collect the supported sample rates for one specific device.
 */
static void
pa_get_sample_rates(const PaDeviceInfo  *info,
                    PaDeviceIndex        idx,
                    PsyAudioSampleRate **sample_rates,
                    gsize               *num_sample_rates)
{
    PsyAudioSampleRate applicable[] = {
        PSY_AUDIO_SAMPLE_RATE_22050,
        PSY_AUDIO_SAMPLE_RATE_24000,
        PSY_AUDIO_SAMPLE_RATE_32000,
        PSY_AUDIO_SAMPLE_RATE_44100,
        PSY_AUDIO_SAMPLE_RATE_48000,
        PSY_AUDIO_SAMPLE_RATE_88200,
        PSY_AUDIO_SAMPLE_RATE_96000,
        PSY_AUDIO_SAMPLE_RATE_192000,
    };

    PsyAudioSampleRate  found[sizeof(applicable) / sizeof(applicable[0])];
    PsyAudioSampleRate *ret       = NULL;
    guint               num_found = 0;

    PaStreamParameters *inp, *outp;

    PaStreamParameters in_stream;
    in_stream.channelCount              = info->maxInputChannels;
    in_stream.device                    = idx;
    in_stream.sampleFormat              = paFloat32;
    in_stream.suggestedLatency          = 0;
    in_stream.hostApiSpecificStreamInfo = NULL;

    PaStreamParameters out_stream;
    out_stream.channelCount              = info->maxOutputChannels;
    out_stream.device                    = idx;
    out_stream.sampleFormat              = paFloat32;
    out_stream.suggestedLatency          = 0;
    out_stream.hostApiSpecificStreamInfo = NULL;

    inp  = in_stream.channelCount > 0 ? &in_stream : NULL;
    outp = out_stream.channelCount > 0 ? &out_stream : NULL;

    // collect and count applicable sample rates
    for (guint i = 0; i < sizeof(applicable) / sizeof(applicable[0]); i++) {
        if (Pa_IsFormatSupported(inp, outp, applicable[i])
            == paFormatIsSupported) {
            found[num_found++] = applicable[i];
        }
    }

    // fill output array
    if (num_found > 0) {
        ret = malloc(sizeof(PsyAudioSampleRate) * num_found);
        for (guint i = 0; i < num_found; i++)
            ret[i] = found[i];
    }

    *num_sample_rates = num_found;
    *sample_rates     = ret;
}

static void
pa_device_enumerate_devices(PsyAudioDevice       *self,
                            PsyAudioDeviceInfo ***infos,
                            guint                *n_infos)
{
    PsyPADevice *pa_self = PSY_PA_DEVICE(self);

    // If we don't have a cache build it.
    if (!pa_self->dev_infos) {
        guint                num_devices = 0;
        PsyAudioDeviceInfo **cache_infos = NULL;

        for (PaDeviceIndex i = 0; i < Pa_GetDeviceCount(); i++) {
            const PaDeviceInfo *info = Pa_GetDeviceInfo(i);
            if (pa_device_uses_preferred_host_api(info)) {
                num_devices++;
            }
        }

        if (num_devices > 0) {

            cache_infos = g_malloc(sizeof(PsyAudioDeviceInfo *) * num_devices);

            num_devices = 0; // reuse variable

            for (PaDeviceIndex i = 0; i < Pa_GetDeviceCount(); i++) {
                PsyAudioSampleRate *sample_rates     = NULL;
                gsize               num_sample_rates = 0;
                const PaDeviceInfo *info             = Pa_GetDeviceInfo(i);

                if (!pa_device_uses_preferred_host_api(info))
                    continue;

                if (!pa_is_pcm_device(info))
                    continue;

                const PaHostApiInfo *host_api_info
                    = Pa_GetHostApiInfo(info->hostApi);
                pa_get_sample_rates(info, i, &sample_rates, &num_sample_rates);
                cache_infos[num_devices]
                    = psy_audio_device_info_new(num_devices,
                                                g_strdup("Portaudio"),
                                                g_strdup(host_api_info->name),
                                                g_strdup(info->name),
                                                info->maxInputChannels,
                                                info->maxOutputChannels,
                                                sample_rates,
                                                num_sample_rates,
                                                i);

                num_devices++;
            }
        }

        pa_self->num_infos = num_devices;
        pa_self->dev_infos = cache_infos;
    }

    // Create a copy from cache
    PsyAudioDeviceInfo **ret_infos
        = g_malloc(sizeof(PsyAudioDeviceInfo *) * pa_self->num_infos);
    for (guint i = 0; i < pa_self->num_infos; i++)
        ret_infos[i] = psy_audio_device_info_copy(pa_self->dev_infos[i]);

    // Return values by reference
    *infos   = ret_infos;
    *n_infos = pa_self->num_infos;
}

/**
 * pa_determine_device:
 * @self: An instance of PsyPADevice.
 * @info:(out): An instance of [struct@PsyAudioDeviceInfo] that matches
 *              the requirements this device.
 *
 * Returns the PaDeviceIndex of the first enumerated device for which the
 * name of the audio device matches the PsyDeviceInfo->device_name.
 *
 * If no name was specified to this audio device it returns the first device,
 * that matches the Requirements regarding number of in- and output channels,
 * sample rate etc.
 *
 * Returns: the PaDeviceNumber that matched or -1 on error.
 */
static gint
pa_determine_device(PsyPADevice         *self,
                    PsyAudioDeviceInfo **info,
                    GError             **error)
{
    PaDeviceIndex devnum = -1;
    const gchar  *name   = psy_audio_device_get_name(PSY_AUDIO_DEVICE(self));

    PsyAudioDeviceInfo **infos = NULL;
    guint                num_infos;
    psy_audio_device_enumerate_devices(
        PSY_AUDIO_DEVICE(self), &infos, &num_infos);

    if (num_infos == 0)
        return -1;

    if (name && g_strcmp0(name, "") != 0) {
        for (guint i = 0; i < num_infos; i++) {
            if (g_strcmp0(name, infos[i]->device_name) == 0) {
                devnum = (gint) infos[i]->private_index;
                *info  = psy_audio_device_info_copy(infos[i]);
                break;
            }
        }
        if (devnum < 0) {
            g_set_error(error,
                        PSY_AUDIO_DEVICE_ERROR,
                        PSY_AUDIO_DEVICE_ERROR_OPEN_NAME,
                        "No such portaudio device: '%s'",
                        name);
        }
    }
    else {
        guint ninput
            = psy_audio_device_get_num_input_channels(PSY_AUDIO_DEVICE(self));
        guint noutput
            = psy_audio_device_get_num_output_channels(PSY_AUDIO_DEVICE(self));
        PsyAudioSampleRate sr
            = psy_audio_device_get_sample_rate(PSY_AUDIO_DEVICE(self));

        for (guint i = 0; i < num_infos; i++) {
            if (ninput > infos[i]->max_inputs)
                continue;
            if (noutput > infos[i]->max_outputs)
                continue;
            if (!psy_audio_device_info_contains_sr(infos[i], sr))
                continue;

            // We've found a match.
            devnum = (gint) i;
            *info  = psy_audio_device_info_copy(infos[i]);
            break;
        }
        if (devnum < 0)
            g_set_error(error,
                        PSY_AUDIO_DEVICE_ERROR,
                        PSY_AUDIO_DEVICE_ERROR_OPEN_NO_MATCH,
                        "Unable to open no device can open %d input channels, "
                        "%d output channels with a sample_rate of %d",
                        ninput,
                        noutput,
                        sr);
    }

    // cleanup enumeration
    for (guint i = 0; i < num_infos; i++) {
        psy_audio_device_info_free(infos[i]);
    }
    g_free(infos);

    return devnum;
}

/* *********** virtual methods ***************** */

// static void
// psy_pa_device_set_property(GObject      *object,
//                                    guint         prop_id,
//                                    const GValue *value,
//                                    GParamSpec   *pspec)
//{
//     PsyPADevice *self = PSY_PA_DEVICE(object);
//     (void) self;
//     (void) value;
//
//     switch ((PsyPADeviceProperty) prop_id) {
//     default:
//         G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
//     }
// }
//
// static void
// psy_pa_device_get_property(GObject    *object,
//                                    guint       prop_id,
//                                    GValue     *value,
//                                    GParamSpec *pspec)
//{
//     PsyPADevice        *self = PSY_PA_DEVICE(object);
//     PsyPADevicePrivate *priv
//         = psy_pa_device_get_instance_private(self);
//     (void) value;
//     (void) priv;
//
//     switch ((PsyPADeviceProperty) prop_id) {
//     default:
//         G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
//     }
// }

static void
psy_pa_device_init(PsyPADevice *self)
{
    gint error           = Pa_Initialize();
    self->pa_initialized = error == paNoError;
    self->clk            = psy_clock_new();
    g_mutex_init(&self->last_frame.lock);
    if (error != paNoError) {
        g_critical("Unable to init portaudio: %s", Pa_GetErrorText(error));
    }
}

static void
psy_pa_device_dispose(GObject *object)
{
    PsyPADevice *pa_self = PSY_PA_DEVICE(object);

    g_clear_object(&pa_self->clock_offset);

    G_OBJECT_CLASS(psy_pa_device_parent_class)->dispose(object);
}

static void
psy_pa_device_finalize(GObject *object)
{
    PsyPADevice *self = PSY_PA_DEVICE(object);

    if (self->pa_initialized) {
        Pa_Terminate();
    }

    if (self->dev_infos) {
        for (guint i = 0; i < self->num_infos; i++) {
            psy_audio_device_info_free(self->dev_infos[i]);
        }
        g_free(self->dev_infos);
    }

    g_mutex_clear(&self->last_frame.lock);

    G_OBJECT_CLASS(psy_pa_device_parent_class)->finalize(object);
}

static void
pa_device_open(PsyAudioDevice *self, GError **error)
{
    gint                pa_err;
    PsyPADevice        *pa_self = PSY_PA_DEVICE(self);
    PsyAudioDeviceInfo *dev_info;

    PaStreamParameters input_params  = {0};
    PaStreamParameters output_params = {0};

    gint device_num = pa_determine_device(pa_self, &dev_info, error);
    if (device_num < 0)
        return;

    const PaDeviceInfo *pa_info = Pa_GetDeviceInfo(device_num);

    PaStreamParameters *p_in_param, *p_out_param;

    input_params.channelCount
        = (int) psy_audio_device_get_num_input_channels(self);
    input_params.device                    = device_num;
    input_params.sampleFormat              = paFloat32;
    input_params.suggestedLatency          = pa_info->defaultLowInputLatency;
    input_params.hostApiSpecificStreamInfo = NULL;

    output_params.channelCount
        = (int) psy_audio_device_get_num_output_channels(self);
    output_params.device                    = device_num;
    output_params.sampleFormat              = paFloat32;
    output_params.suggestedLatency          = pa_info->defaultLowOutputLatency;
    output_params.hostApiSpecificStreamInfo = NULL;

    p_in_param  = input_params.channelCount > 0 ? &input_params : NULL;
    p_out_param = output_params.channelCount > 0 ? &output_params : NULL;

    pa_err = Pa_OpenStream(&pa_self->stream,
                           p_in_param,
                           p_out_param,
                           psy_audio_device_get_sample_rate(self),
                           paFramesPerBufferUnspecified,
                           paPrimeOutputBuffersUsingStreamCallback,
                           &pa_audio_callback,
                           self);

    if (pa_err != paNoError) {
        g_set_error(error,
                    PSY_AUDIO_DEVICE_ERROR,
                    PSY_AUDIO_DEVICE_ERROR_OPEN,
                    "Portaudio is unable to open a stream: %s",
                    Pa_GetErrorText(pa_err));
        return;
    }
#if defined HAVE_PA_LINUX_ALSA_H
    if (g_strcmp0(dev_info->host_api, "ALSA")) {
        // enable "realtime" scheduling for alsa
        PaAlsa_EnableRealtimeScheduling(pa_self->stream, 1);
    }
#endif

    if (g_strcmp0(psy_audio_device_get_name(self), pa_info->name) != 0) {
        psy_audio_device_set_name(self, dev_info->device_name);
    }

    psy_audio_device_info_free(dev_info);

    PSY_AUDIO_DEVICE_CLASS(psy_pa_device_parent_class)->open(self, error);
}

static void
pa_device_start(PsyAudioDevice *self, GError **error)
{
    PsyPADevice *pa_self = PSY_PA_DEVICE(self);

    PaError err = Pa_StartStream(pa_self->stream);
    if (err != paNoError) {
        g_set_error(error,
                    PSY_AUDIO_DEVICE_ERROR,
                    PSY_AUDIO_DEVICE_ERROR_FAILED,
                    "Unable to start portaudio stream: %s",
                    Pa_GetErrorText(err));
        return;
    }

    pa_clear_last_frame_info(pa_self);

    PaTime        stream_time = Pa_GetStreamTime(pa_self->stream);
    PsyTimePoint *pa_time     = pa_time_to_psy_timepoint(stream_time);

    pa_time_calculate_clock_offset(pa_self, pa_time);

    g_object_unref(pa_time);

    PSY_AUDIO_DEVICE_CLASS(psy_pa_device_parent_class)->start(self, error);
}

static void
pa_device_stop(PsyAudioDevice *self)
{
    PsyPADevice *pa_self = PSY_PA_DEVICE(self);

    if (psy_audio_device_get_started(self)) {
        PaError err = Pa_StopStream(pa_self->stream);
        if (err != paNoError) {
            g_error("Unable to stop portaudio stream: %s",
                    Pa_GetErrorText(err));
            return;
        }

        pa_clear_last_frame_info(pa_self);
    }

    PSY_AUDIO_DEVICE_CLASS(psy_pa_device_parent_class)->stop(self);
}

static void
pa_device_close(PsyAudioDevice *self)
{
    PsyPADevice *pa_self = PSY_PA_DEVICE(self);

    // This function is sometimes called recursively while closing the audio
    // device.
    if (pa_self->stream != NULL) {

        PaError error = Pa_CloseStream(pa_self->stream);
        if (error != paNoError)
            g_critical("Unable to close stream: %s", Pa_GetErrorText(error));
        else
            pa_self->stream = NULL;
    }

    PSY_AUDIO_DEVICE_CLASS(psy_pa_device_parent_class)->close(self);
}

static const gchar *
pa_device_get_default_name(PsyAudioDevice *self)
{
    PsyPADevice *pa_self = PSY_PA_DEVICE(self);

    if (!pa_self->dev_infos) {
        PsyAudioDeviceInfo **infos = NULL;
        guint                num_infos;

        psy_audio_device_enumerate_devices(self, &infos, &num_infos);

        for (gsize i = 0; i < num_infos; i++)
            psy_audio_device_info_free(infos[i]);
        g_free(infos);
    }

    if (!pa_self->dev_infos)
        return NULL;

    return pa_self->dev_infos[0]->device_name;
}

static PsyDuration *
pa_device_get_output_latency(PsyAudioDevice *self)
{
    g_return_val_if_fail(psy_audio_device_get_is_open(self), NULL);
    PsyPADevice *pa_self = PSY_PA_DEVICE(self);
    g_return_val_if_fail(pa_self->stream, NULL);

    PaTime pa_time = Pa_GetStreamInfo(pa_self->stream)->outputLatency;

    PsyDuration *latency = psy_duration_new(pa_time);
    return latency;
}

static gboolean
pa_device_get_last_known_frame(PsyAudioDevice *self,
                               gint64         *nth_frame,
                               PsyTimePoint  **tp_in,
                               PsyTimePoint  **tp_out)
{
    PsyPADevice *pa_self = PSY_PA_DEVICE(self);

    *nth_frame = pa_self->last_frame.num_frames;

    if (tp_in) {
        PsyTimePoint *tp_pa_in
            = pa_time_to_psy_timepoint(pa_self->last_frame.time_input);
        *tp_in = pa_transform_pa_time_to_psy_time(pa_self, tp_pa_in);
        g_object_unref(tp_pa_in);
    }

    if (tp_out) {
        PsyTimePoint *tp_pa_out
            = pa_time_to_psy_timepoint(pa_self->last_frame.time_output);
        *tp_out = pa_transform_pa_time_to_psy_time(pa_self, tp_pa_out);
        g_object_unref(tp_pa_out);
    }

    return TRUE;
}

static void
psy_pa_device_class_init(PsyPADeviceClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

    //    gobject_class->set_property = psy_pa_device_set_property;
    //    gobject_class->get_property = psy_pa_device_get_property;
    gobject_class->finalize = psy_pa_device_finalize;
    gobject_class->dispose  = psy_pa_device_dispose;

    PsyAudioDeviceClass *audio_klass = PSY_AUDIO_DEVICE_CLASS(klass);

    audio_klass->open                 = pa_device_open;
    audio_klass->close                = pa_device_close;
    audio_klass->start                = pa_device_start;
    audio_klass->stop                 = pa_device_stop;
    audio_klass->get_default_name     = pa_device_get_default_name;
    audio_klass->enumerate_devices    = pa_device_enumerate_devices;
    audio_klass->get_output_latency   = pa_device_get_output_latency;
    audio_klass->get_last_known_frame = pa_device_get_last_known_frame;

    // We just use what the base class knows.
    //    audio_klass->set_name        = pa_device_set_name;
    //    audio_klass->set_sample_rate = pa_device_set_sample_rate;

    // We currently do not have properties
    //    g_object_class_install_properties(
    //        gobject_class, NUM_PROPERTIES, pa_device_properties);
}

/* ************ public functions ******************** */

/**
 * psy_pa_device_new:(constructor)
 *
 * Constructs an jack audio device.
 * This object will try to connect to or instantiate a jack server in order
 * to obtain the playback and capture devices.
 *
 * Returns: a instance of [class@PsyPADevice]
 */
PsyAudioDevice *
psy_pa_device_new(void)
{
    return g_object_new(PSY_TYPE_PA_DEVICE, NULL);
}
