
#include <portaudio.h>
#include <stdio.h>

#include "enum-types.h"
#include "psy-clock.h"
#include "psy-config.h"
#include "psy-enums.h"
#include "psy-pa-device.h"

/**
 * PsyPADevice:
 *
 * PsyPADevice is a device that uses portaudio to implement a PsyAudioDevice.
 */

typedef struct _PsyPADevice {
    PsyAudioDevice       parent;
    PsyClock            *psy_clock;
    PaStream            *stream;
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
                  unsigned long                   frameCount,
                  const PaStreamCallbackTimeInfo *timeInfo,
                  PaStreamCallbackFlags           statusFlags,
                  void                           *audio_device)
{
    (void) input;
    (void) output;
    (void) frameCount;
    (void) timeInfo;
    (void) statusFlags;
    PsyPADevice *self = audio_device;

    // TODO no system calls in audio callback and no mallocs..
    PsyTimePoint *tp = psy_clock_now(self->psy_clock);

    // TODO this might block
    if (G_UNLIKELY(!psy_audio_device_get_started(PSY_AUDIO_DEVICE(self)))) {
        psy_audio_device_set_started(PSY_AUDIO_DEVICE(self), tp);
    }
    // Read first, because the input might be desired for the output.

    // TODO freeing isn't bounded.
    g_object_unref(tp);

    return 0;
}

/**
 * pa_is_pcm_device:
 *
 * This function tries to determine whether it is a pcm device, e.g. ALSA
 * virtual devices are ignored.
 *
 * TODO check for windows devices whether they are PCM devices
 */
gboolean
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
gboolean
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
void
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

/**
 * pa_enumerate_devices:
 *
 */
void
pa_enumerate_devices(PsyAudioDevice       *self,
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
                                                num_sample_rates);

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
 * pa_determine_device_num:
 *
 * Uses the first available device
 * for alsa, wasapi on linux and windows respectively
 *
 * TODO Allow to use other that the first device.
 */
static int
pa_determine_device_num(PsyPADevice *self)
{
    int             devnum = -1;
    PaHostApiTypeId hostapi_typeid;
#if defined(__linux__)
    hostapi_typeid = paALSA;
#elif defined(WIN32)
    hostapi_typeid = paWASAPI;
#else
    #error "Currently unsupported platform"
#endif

    for (gint i = 0; i < Pa_GetDeviceCount(); i++) {
        const PaDeviceInfo *dev_info = Pa_GetDeviceInfo(i);
        if (dev_info->hostApi
            == Pa_HostApiTypeIdToHostApiIndex(hostapi_typeid)) {
            devnum = i;
            break;
        }
    }

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
    if (error != paNoError) {
        g_critical("Unable to init portaudio: %s", Pa_GetErrorText(error));
    }
    self->psy_clock = psy_clock_new();
}

static void
psy_pa_device_dispose(GObject *object)
{
    PsyPADevice *self = PSY_PA_DEVICE(object);
    (void) self;

    g_clear_object(&self->psy_clock);

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

    G_OBJECT_CLASS(psy_pa_device_parent_class)->finalize(object);
}

static void
pa_device_open(PsyAudioDevice *self, GError **error)
{
    gint         pa_err;
    PsyPADevice *pa_self = PSY_PA_DEVICE(self);

    PaStreamParameters input_params  = {0};
    PaStreamParameters output_params = {0};

    gint device_num = pa_determine_device_num(pa_self);

    PaStreamParameters *p_in_param, *p_out_param;

    input_params.channelCount = psy_audio_device_get_num_input_channels(self);
    input_params.device       = device_num;
    input_params.sampleFormat = paFloat32;
    input_params.suggestedLatency = 0;

    output_params.channelCount = psy_audio_device_get_num_output_channels(self);
    output_params.device       = device_num;
    output_params.sampleFormat = paFloat32;
    output_params.suggestedLatency = 0;

    p_in_param  = input_params.channelCount > 0 ? &input_params : NULL;
    p_out_param = output_params.channelCount > 0 ? &output_params : NULL;

    pa_err = Pa_OpenStream(&pa_self->stream,
                           p_in_param,
                           p_out_param,
                           psy_audio_device_get_sample_rate(self),
                           paFramesPerBufferUnspecified,
                           paNoFlag,
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

    PSY_AUDIO_DEVICE_CLASS(psy_pa_device_parent_class)->open(self, error);
}

static void
pa_device_start(PsyAudioDevice *self, GError **error)
{
    PsyPADevice *pa_self = PSY_PA_DEVICE(self);

    PSY_AUDIO_DEVICE_CLASS(psy_pa_device_parent_class)->start(self, error);
}

static void
pa_device_stop(PsyAudioDevice *self)
{
    PsyPADevice *pa_self = PSY_PA_DEVICE(self);
    PSY_AUDIO_DEVICE_CLASS(psy_pa_device_parent_class)->stop(self);
}

static void
pa_device_close(PsyAudioDevice *self)
{
    PsyPADevice *pa_self = PSY_PA_DEVICE(self);
    PSY_AUDIO_DEVICE_CLASS(psy_pa_device_parent_class)->close(self);
}

static const gchar *
pa_device_get_default_name(PsyAudioDevice *self)
{
    (void) self;
    // NOTE This is only correct when using ALSA as jack backend.
    return "hw:0";
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

    audio_klass->open              = pa_device_open;
    audio_klass->close             = pa_device_close;
    audio_klass->start             = pa_device_start;
    audio_klass->stop              = pa_device_stop;
    audio_klass->get_default_name  = pa_device_get_default_name;
    audio_klass->enumerate_devices = pa_enumerate_devices;

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
