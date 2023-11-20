
#include "enum-types.h"

#include "psy-audio-device.h"
#include "psy-audio-output-mixer.h"
#include "psy-config.h"
#include "psy-enums.h"

//#if defined HAVE_ALSA
//    #include "alsa/psy-alsa-audio-device.h"
//#endif

#if defined HAVE_JACK2
    #include "jack/psy-jack-audio-device.h"
#endif

#if defined HAVE_PORTAUDIO
    #include "portaudio/psy-pa-device.h"
#endif

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"

G_DEFINE_BOXED_TYPE(PsyAudioDeviceInfo,
                    psy_audio_device_info,
                    &psy_audio_device_info_copy,
                    &psy_audio_device_info_free);

#pragma GCC diagnostic pop

/**
 * psy_audio_device_info_new:(constructor)(skip)
 * @device_num: The device number of this info
 * @psy_api:(transfer full): The api that psylib is using for this device info
 * @host_api:(transfer full): The api that @psy_api is using for this device
 *                            info this may be the same e.g. portaudio might use
 *                            ALSA, but ALSA just uses itself (or no
 *                            intermediate info).
 * @device_name:(transfer full): The name of this device.
 * @max_inputs: the number of input channels this device has
 * @max_outputs: the number of output channels this device has
 * @sample_rates:(transfer full)(array length=num_sample_rates):
 *                            The sample rates this devices supports
 * @num_sample_rates: The length of @sample_rates.
 *
 * Construct a new boxed instance of [struct@PsyAudioDeviceInfo]
 */
PsyAudioDeviceInfo *
psy_audio_device_info_new(gint                device_num,
                          gchar              *psy_api,
                          gchar              *host_api,
                          gchar              *device_name,
                          guint               max_inputs,
                          guint               max_outputs,
                          PsyAudioSampleRate *sample_rates,
                          guint               num_sample_rates,
                          guint               private_index)
{
    PsyAudioDeviceInfo *new = g_malloc(sizeof(PsyAudioDeviceInfo));

    new->device_num       = device_num;
    new->psy_api          = psy_api;
    new->host_api         = host_api;
    new->device_name      = device_name;
    new->max_inputs       = max_inputs;
    new->max_outputs      = max_outputs;
    new->sample_rates     = sample_rates;
    new->num_sample_rates = num_sample_rates;
    new->private_index    = private_index;

    return new;
}

/**
 * psy_audio_device_info_get_sample_rates:
 * @self: an instance of [struct@PsyAudioDeviceInfo],
 * @sample_rates:(out)(transfer none)(array length=num_sample_rates):
 * @num_sample_rates:(out): The number of supported sample rates
 *
 * Get the supported sample rates for this device
 */
void
psy_audio_device_info_get_sample_rates(PsyAudioDeviceInfo  *self,
                                       PsyAudioSampleRate **sample_rates,
                                       guint               *num_sample_rates)
{
    g_return_if_fail(self);
    g_return_if_fail(sample_rates);
    g_return_if_fail(num_sample_rates);

    *sample_rates     = self->sample_rates;
    *num_sample_rates = self->num_sample_rates;
}

/**
 * psy_audio_device_info_copy:
 * @self: An instance of [struct@AudioDeviceInfo] to copy
 *
 * Copy a PsyAudioDeviceInfo
 *
 * Returns:(transfer full): a deep copy of @self
 */
PsyAudioDeviceInfo *
psy_audio_device_info_copy(PsyAudioDeviceInfo *self)
{
    PsyAudioDeviceInfo *new = g_malloc(sizeof(PsyAudioDeviceInfo));

    new->device_num = self->device_num;

    new->psy_api     = g_strdup(self->psy_api);
    new->host_api    = g_strdup(self->host_api);
    new->device_name = g_strdup(self->device_name);

    new->max_inputs  = self->max_inputs;
    new->max_outputs = self->max_outputs;

    new->sample_rates     = g_malloc(sizeof(PSY_AUDIO_SAMPLE_RATE_48000)
                                 * self->num_sample_rates);
    new->num_sample_rates = self->num_sample_rates;
    for (guint i = 0; i < self->num_sample_rates; i++)
        new->sample_rates[i] = self->sample_rates[i];

    new->private_index = self->private_index;

    return new;
}

/**
 * psy_audio_device_info_as_string:
 * @self: an instance of [struct@PsyAudioDeviceInfo] to stringify
 *
 * Creates a string representation of the device info
 *
 * Returns:(transfer full): a string with a representation of the device, the
 * returned value should be freed with g_free.
 */
gchar *
psy_audio_device_info_as_string(PsyAudioDeviceInfo *self)
{
    GString *str_buf = g_string_sized_new(1024);

    g_string_append_printf(str_buf,
                           "PsyAudioDeviceInfo for device: %s\n"
                           "\tdevice_num = %d\n"
                           "\tpsy_api = %s\n"
                           "\thost_api = %s\n"
                           "\tmax_inputs = %d\n"
                           "\tmax_outputs = %d\n"
                           "\tsupported sample rates = [",
                           self->device_name,
                           self->device_num,
                           self->psy_api,
                           self->host_api,
                           self->max_inputs,
                           self->max_outputs);

    for (guint i = 0; i < self->num_sample_rates; i++) {
        if (i < self->num_sample_rates - 1)
            g_string_append_printf(str_buf, "%d, ", self->sample_rates[i]);
        else
            g_string_append_printf(str_buf, "%d", self->sample_rates[i]);
    }

    g_string_append_printf(str_buf, "]");

#if GLIB_CHECK_VERSION(2, 76, 0)
    return g_string_free_and_steal(str_buf);
#else
    return g_string_free(str_buf, FALSE);
#endif
}

/**
 * psy_audio_device_info_contains_sr:
 * @self: An instance of [struct@PsyAudioDeviceInfo]
 * @sr: An value of [enum@PsyAudioSampleRate]
 *
 * Checks whether this device supports the samplerate @sr
 *
 * Returns: #TRUE when the sample rate is supported #FALSE otherwise
 */
gboolean
psy_audio_device_info_contains_sr(PsyAudioDeviceInfo *self,
                                  PsyAudioSampleRate  sr)
{
    g_return_val_if_fail(self, FALSE);
    for (guint i = 0; i < self->num_sample_rates; i++) {
        if (self->sample_rates[i] == sr)
            return TRUE;
    }
    return FALSE;
}

void
psy_audio_device_info_free(PsyAudioDeviceInfo *self)
{
    g_free(self->psy_api);
    g_free(self->host_api);
    g_free(self->device_name);
    g_free(self->sample_rates);

    g_free(self);
}

/**
 * PsyAudioDevice:
 *
 * A PsyAudioDevice is a device that represents an PCI(e), USB, Firewire
 * on the mother board, etc. type of an audio interface.
 *
 * In order to open it, one should first set the right parameters of the
 * device. Once they are set you should can try to open it to see whether
 * the parameters are accepted and possible for the device.
 *
 * Once you have opened the device you should be able to start it.
 * Notice that enumerating, opening, starting and closing a device are expensive
 * operations. Hence, you want to do this prior to starting your experiment.
 */

// clang-format off
G_DEFINE_QUARK(psy-audio-device-error-quark,
               psy_audio_device_error)

// clang-format on

typedef struct _PsyAudioDevicePrivate {
    gchar               *name;
    PsyAudioSampleRate   sample_rate;
    GMainContext        *main_context;
    PsyAudioOutputMixer *output_mixer;
    guint                num_inputs;
    guint                num_outputs;
    guint                num_sample_callback;
    gboolean             is_open;
    gboolean             started;
} PsyAudioDevicePrivate;

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE(PsyAudioDevice,
                                    psy_audio_device,
                                    G_TYPE_OBJECT)

typedef enum {
    PROP_NULL,
    PROP_NAME,
    PROP_IS_OPEN,
    PROP_STARTED,
    PROP_SAMPLE_RATE,
    PROP_NUM_INPUTS,
    PROP_NUM_OUTPUTS,
    PROP_NUM_SAMPLES_CALLBACK,
    NUM_PROPERTIES
} PsyAudioDeviceProperty;

typedef enum { STARTED, NUM_SIGNALS } PsyAudioDeviceSignals;

static GParamSpec *audio_device_properties[NUM_PROPERTIES];
static guint       audio_device_signals[NUM_SIGNALS];

/* *** helpers to signal stuff from audio callback to main thread *** */

typedef struct AudioStartedMsg {
    PsyTimePoint   *tp_started;
    PsyAudioDevice *audio_device;
} AudioStartedMsg;

static void
audio_started_msg_free(gpointer msg)
{
    AudioStartedMsg *message = msg;
    g_object_unref(message->tp_started);
    g_object_unref(message->audio_device);
    g_free(msg);
}

/* ********** virtual(/private) functions ***************** */

static void
psy_audio_device_set_property(GObject      *object,
                              guint         prop_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
    PsyAudioDevice *self = PSY_AUDIO_DEVICE(object);
    (void) self;
    (void) value;

    switch ((PsyAudioDeviceProperty) prop_id) {
    case PROP_NAME:
        psy_audio_device_set_name(self, g_value_get_string(value));
        break;
    case PROP_SAMPLE_RATE:
        psy_audio_device_set_sample_rate(self, g_value_get_enum(value));
        break;
    case PROP_NUM_INPUTS:
        psy_audio_device_set_num_input_channels(self, g_value_get_uint(value));
        break;
    case PROP_NUM_OUTPUTS:
        psy_audio_device_set_num_output_channels(self, g_value_get_uint(value));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

static void
psy_audio_device_get_property(GObject    *object,
                              guint       prop_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
    PsyAudioDevice *self = PSY_AUDIO_DEVICE(object);

    switch ((PsyAudioDeviceProperty) prop_id) {
    case PROP_NAME:
        g_value_set_string(value, psy_audio_device_get_name(self));
        break;
    case PROP_IS_OPEN:
        g_value_set_boolean(value, psy_audio_device_get_is_open(self));
        break;
    case PROP_STARTED:
        g_value_set_boolean(value, psy_audio_device_get_started(self));
        break;
    case PROP_SAMPLE_RATE:
        g_value_set_enum(value, psy_audio_device_get_sample_rate(self));
        break;
    case PROP_NUM_INPUTS:
        g_value_set_uint(value, psy_audio_device_get_num_input_channels(self));
        break;
    case PROP_NUM_OUTPUTS:
        g_value_set_uint(value, psy_audio_device_get_num_output_channels(self));
        break;
    case PROP_NUM_SAMPLES_CALLBACK:
        g_value_set_uint(value,
                         psy_audio_device_get_num_samples_callback(self));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

static void
psy_audio_device_init(PsyAudioDevice *self)
{
    PsyAudioDevicePrivate *priv = psy_audio_device_get_instance_private(self);

    priv->is_open      = FALSE;
    priv->name         = g_strdup("");
    priv->sample_rate  = PSY_AUDIO_SAMPLE_RATE_48000;
    priv->main_context = g_main_context_ref_thread_default();
}

static void
psy_audio_device_dispose(GObject *object)
{
    PsyAudioDevice        *self = PSY_AUDIO_DEVICE(object);
    PsyAudioDevicePrivate *priv = psy_audio_device_get_instance_private(self);

    if (priv->is_open) {
        psy_audio_device_stop(self);
        psy_audio_device_close(self);
    }
    g_assert(!priv->is_open);

    g_main_context_unref(priv->main_context);
    priv->main_context = NULL;

    G_OBJECT_CLASS(psy_audio_device_parent_class)->dispose(object);
}

static void
psy_audio_device_finalize(GObject *object)
{
    PsyAudioDevice        *self = PSY_AUDIO_DEVICE(object);
    PsyAudioDevicePrivate *priv = psy_audio_device_get_instance_private(self);

    g_free(priv->name);

    G_OBJECT_CLASS(psy_audio_device_parent_class)->finalize(object);
}

static void
audio_device_open(PsyAudioDevice *self, GError **error)
{
    (void) error; // Error's might be raised in derived classes (backends).
    PsyAudioDevicePrivate *priv = psy_audio_device_get_instance_private(self);

    PsyAudioOutputMixer *mixer = psy_audio_output_mixer_new(self);
    priv->output_mixer         = mixer;

    priv->is_open = TRUE;
    g_info("Opened PsyAudioDevice %s", psy_audio_device_get_name(self));

    psy_audio_device_start(self, error);
}

static void
audio_device_close(PsyAudioDevice *self)
{
    PsyAudioDevicePrivate *priv = psy_audio_device_get_instance_private(self);
    priv->is_open               = FALSE;
    g_info("Closed PsyAudioDevice %s", psy_audio_device_get_name(self));
}

static void
audio_device_start(PsyAudioDevice *self, GError **error)
{
    (void) error; // Error's might be raised in derived classes (backends).
    PsyAudioDevicePrivate *priv = psy_audio_device_get_instance_private(self);
    priv->started               = TRUE;
    g_info("Started PsyAudioDevice %s", psy_audio_device_get_name(self));
}

static void
audio_device_stop(PsyAudioDevice *self)
{
    PsyAudioDevicePrivate *priv = psy_audio_device_get_instance_private(self);
    priv->started               = FALSE;
    g_info("Stopped PsyAudioDevice %s", psy_audio_device_get_name(self));
}

static gboolean
audio_device_emit_started(AudioStartedMsg *msg)
{
    g_signal_emit(
        msg->audio_device, audio_device_signals[STARTED], 0, msg->tp_started);

    return FALSE;
}

static void
psy_audio_device_class_init(PsyAudioDeviceClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

    gobject_class->set_property = psy_audio_device_set_property;
    gobject_class->get_property = psy_audio_device_get_property;
    gobject_class->finalize     = psy_audio_device_finalize;
    gobject_class->dispose      = psy_audio_device_dispose;

    klass->open  = audio_device_open;
    klass->close = audio_device_close;
    klass->start = audio_device_start;
    klass->stop  = audio_device_stop;

    /**
     * PsyAudioDevice:name:
     *
     * You may get the name of the device here. After the device has been opened
     * a new name may be set here.
     */
    audio_device_properties[PROP_NAME]
        = g_param_spec_string("name",
                              "Name",
                              "The name of the device to open",
                              NULL,
                              G_PARAM_READWRITE);

    /**
     * PsyAudioDevice:is-open:
     *
     * You may use this property to see whether the device is open.
     */
    audio_device_properties[PROP_IS_OPEN]
        = g_param_spec_boolean("is-open",
                               "IsOpen",
                               "Whether or not the device is open",
                               FALSE,
                               G_PARAM_READABLE);

    /**
     * PsyAudioDevice:started:
     *
     * You may use this property to see whether the audio callback is running.
     */
    audio_device_properties[PROP_STARTED]
        = g_param_spec_boolean("started",
                               "started",
                               "Whether or not the audio callback is running",
                               FALSE,
                               G_PARAM_READABLE);

    /**
     * PsyAudioDevice:sample-rate:
     *
     * You may use this to get the sample rate that is used for the audio
     * device.
     */
    audio_device_properties[PROP_SAMPLE_RATE]
        = g_param_spec_enum("sample-rate",
                            "SampleRate",
                            "The sample rate of this device",
                            psy_audio_sample_rate_get_type(),
                            PSY_AUDIO_SAMPLE_RATE_48000,
                            G_PARAM_READWRITE);

    /**
     * PsyAudioDevice:num-input-channels
     *
     * The desired number of input channels, this property should be set before
     * opening the device, if it is open you'll have to close and stop the
     * device first.
     */
    audio_device_properties[PROP_NUM_INPUTS]
        = g_param_spec_uint("num-input-channels",
                            "NumInputChannels",
                            "The number of input channels",
                            0,
                            G_MAXUINT,
                            0,
                            G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

    /**
     * PsyAudioDevice:num-output-channels
     *
     * The desired number of output channels, this property should be set before
     * opening the device, if it is open you'll have to close and stop the
     * device first.
     */
    audio_device_properties[PROP_NUM_OUTPUTS]
        = g_param_spec_uint("num-output-channels",
                            "NumOutputChannels",
                            "The number of output channels",
                            0,
                            G_MAXUINT,
                            2,
                            G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

    /**
     * PsyAudioDevice:num-samples-per-callback
     *
     * Obtain the number of samples that are processed each time the callback
     * is called. This means each channel of audio processes this number
     * of samples, hence you might need the number of channels to process
     * which the total numbers of samples.
     */
    audio_device_properties[PROP_NUM_SAMPLES_CALLBACK] = g_param_spec_uint(
        "samples-per-callback",
        "SamplesPerCallback",
        "The number of samples per channel that are processed",
        0,
        G_MAXUINT,
        0,
        G_PARAM_READABLE);

    g_object_class_install_properties(
        gobject_class, NUM_PROPERTIES, audio_device_properties);

    /**
     * PsyAudioDevice::started
     * @self: The object on which this signal is emitted.
     * @timestamp: The timestamp when the first samples are transmitted to the
     *             AudioDevice.
     *
     * This signal may be used to determine when the AudioDevice is started.
     * This signal is emitted when the first time the audio callback is run.
     */
    audio_device_signals[STARTED]
        = g_signal_new("started",
                       G_TYPE_FROM_CLASS(gobject_class),
                       G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE,
                       0,    // TODO set a class handler
                       NULL, // accumulator
                       NULL, // accumulator data
                       NULL, // marshaller
                       G_TYPE_NONE,
                       1,
                       PSY_TYPE_TIME_POINT);
}

/* ************ public functions ******************** */

/**
 * psy_audio_device_new:(constructor)
 *
 * Constructs an audio device. This constructor will check which backends
 * are available on this machine and if so, it will try to get the best one
 * possible.
 * The returned device will implement a PsyAudioDevice for one specific
 * backend.
 *
 * Returns: A platform specific instance of [class@PsyAudioDevice] that will
 *          implement this class.
 */
PsyAudioDevice *
psy_audio_device_new(void)
{
#if defined HAVE_PORTAUDIO
    return psy_pa_device_new();
#elif defined HAVE_ALSA
    return psy_alsa_audio_device_new();
#elif defined HAVE_JACK2
    return psy_jack_audio_device_new();
#else
    return NULL;
#endif
}

gboolean
psy_audio_device_set_name(PsyAudioDevice *self, const gchar *name)
{
    g_return_val_if_fail(PSY_IS_AUDIO_DEVICE(self) && name != NULL, FALSE);
    PsyAudioDevicePrivate *priv = psy_audio_device_get_instance_private(self);

    if (psy_audio_device_get_is_open(self)) {
        g_warning(
            "It only makes sense to set the name when the device isn't open.");
        return FALSE;
    }

    g_clear_pointer(&priv->name, g_free);

    priv->name = g_strdup(name);
    return TRUE;
}

const gchar *
psy_audio_device_get_name(PsyAudioDevice *self)
{
    g_return_val_if_fail(PSY_IS_AUDIO_DEVICE(self), NULL);
    PsyAudioDevicePrivate *priv = psy_audio_device_get_instance_private(self);

    return priv->name;
}

const gchar *
psy_audio_device_get_default_name(PsyAudioDevice *self)
{
    g_return_val_if_fail(PSY_IS_AUDIO_DEVICE(self), NULL);

    PsyAudioDeviceClass *cls = PSY_AUDIO_DEVICE_GET_CLASS(self);

    g_return_val_if_fail(cls->get_default_name, NULL);

    return cls->get_default_name(self);
}

/**
 * psy_audio_device_open:
 * @self: An instance of [class@PsyAudioDevice]
 *
 * Opens a PsyAudioDevice. Before trying to open the device, it is recommended
 * to set the parameters to use for opening the device. Relevant parameters
 * are [property@PsyAudioDevice:num-input-channels],
 * [property@PsyAudioDevice:num-output-channels],
 * [property@PsyAudioDevice:sample-rate],
 * [property@PsyAudioDevice:name]
 *
 * If you set the name of the device, this is the device psylib will try to
 * open and will try to apply the other parameters to this device. If you
 * leave the name empty ("") or NULL, the first device that matches the
 * other properties above will be opened and the name will be set on the
 * device.
 *
 * So if you want to open a specific device, set the
 * [property@PsyAudioDevice:name] before opening the device. You can find the
 * applicable devices using [method@Psy.AudioDevice.enumerate_devices], this
 * will return a list of [struct@PsyAudioDeviceInfo] which contains the
 * appropriate device_name.
 *
 * If you have opened the devices successfully it will also be started,
 * and the audio will stream immediately.
 */
void
psy_audio_device_open(PsyAudioDevice *self, GError **error)
{
    g_return_if_fail(PSY_IS_AUDIO_DEVICE(self));
    g_return_if_fail(!error || *error == NULL);

    if (psy_audio_device_get_is_open(self))
        return;

    PsyAudioDeviceClass *cls = PSY_AUDIO_DEVICE_GET_CLASS(self);

    g_return_if_fail(cls->open);

    cls->open(self, error);
}

/**
 * psy_audio_device_close:
 * @self: an instance of [class@AudioDevice]
 *
 * Closes the audio device.
 */
void
psy_audio_device_close(PsyAudioDevice *self)
{
    g_return_if_fail(PSY_IS_AUDIO_DEVICE(self));

    if (psy_audio_device_get_started(self))
        psy_audio_device_stop(self);
    if (psy_audio_device_get_is_open(self) == FALSE)
        return;

    PsyAudioDeviceClass *cls = PSY_AUDIO_DEVICE_GET_CLASS(self);

    g_return_if_fail(cls->close);

    cls->close(self);
}

/**
 * psy_audio_device_start:
 * @self: an instance of [class@AudioDevice]
 * @error: errors may be returned here
 *
 * Generally you should not need to use this function as it is already
 * called when the device is successfully opened.
 * When this function is called the AudioCallback function will be called from
 * some background thread, hence, the audio will start streaming and as such
 * The device should be prepared to handle audio when this function is called.
 */
void
psy_audio_device_start(PsyAudioDevice *self, GError **error)
{
    g_return_if_fail(PSY_IS_AUDIO_DEVICE(self));
    g_return_if_fail(!error || *error == NULL);

    PsyAudioDeviceClass *cls = PSY_AUDIO_DEVICE_GET_CLASS(self);
    g_return_if_fail(cls->start);

    cls->start(self, error);
}

/**
 * psy_audio_device_stop:
 * @self: an instance of [class@AudioDevice]
 *
 * Generally you should not need to use this function as it is already
 * called when the device is about to close. The audio callback should not
 * be called from this moment on.
 */
void
psy_audio_device_stop(PsyAudioDevice *self)
{
    g_return_if_fail(PSY_IS_AUDIO_DEVICE(self));

    PsyAudioDeviceClass *cls = PSY_AUDIO_DEVICE_GET_CLASS(self);
    g_return_if_fail(cls->stop);

    cls->stop(self);
}

/**
 * psy_audio_device_get_is_open:
 *
 *
 * Returns: TRUE if the device is open, false otherwise.
 */
gboolean
psy_audio_device_get_is_open(PsyAudioDevice *self)
{
    g_return_val_if_fail(PSY_IS_AUDIO_DEVICE(self), FALSE);

    PsyAudioDevicePrivate *priv = psy_audio_device_get_instance_private(self);

    return priv->is_open;
}

/**
 * psy_audio_device_get_sample_rate:
 *
 * Get the sample rate that this device should use or is using.
 */
PsyAudioSampleRate
psy_audio_device_get_sample_rate(PsyAudioDevice *self)
{
    g_return_val_if_fail(PSY_IS_AUDIO_DEVICE(self), 0);

    PsyAudioDevicePrivate *priv = psy_audio_device_get_instance_private(self);

    return priv->sample_rate;
}

/**
 * psy_audio_device_set_sample_rate:
 * @self: an instance of [class@PsyAudioDevice]
 * @sample_rate: The desired sample rate for opening the audio device
 *
 * Set the desired sample rate of the audio device. This sample rate will
 * be used to configure the audio device when opening the device. Hence,
 * it makes only sense to change it when the device isn't open yet.
 *
 * Returns: TRUE if the desired sample rate is set, FALSE otherwise
 */
gboolean
psy_audio_device_set_sample_rate(PsyAudioDevice    *self,
                                 PsyAudioSampleRate sample_rate)
{
    g_return_val_if_fail(PSY_IS_AUDIO_DEVICE(self), FALSE);
    PsyAudioDevicePrivate *priv = psy_audio_device_get_instance_private(self);

    if (psy_audio_device_get_is_open(self)) {
        g_warning("Unable to change the sample rate when the device is open.");
        return FALSE;
    }

    priv->sample_rate = sample_rate;
    return TRUE;
}

/**
 * psy_audio_device_set_num_input_channels:
 * @self: an instance of [class@AudioDevice]
 * @n_channels: the desired number of input channels
 *
 * Set the desired number of input channels for opening the audio device. This
 * property must be set prior to opening the device.
 *
 * Returns: TRUE if the desired number of channels is set, FALSE otherwise.
 */
gboolean
psy_audio_device_set_num_input_channels(PsyAudioDevice *self, guint n_channels)
{
    g_return_val_if_fail(PSY_IS_AUDIO_DEVICE(self), FALSE);
    PsyAudioDevicePrivate *priv = psy_audio_device_get_instance_private(self);

    if (psy_audio_device_get_is_open(self)) {
        g_warning(
            "Unable to change num-input-channels when the device is open.");
        return FALSE;
    }
    priv->num_inputs = n_channels;
    return TRUE;
}

/**
 * psy_audio_device_get_num_input_channels:
 * @self: An instance of [class@PsyAudioDevice]
 *
 * Returns the number of input channels that are desired for opening the device
 * if the device has opened, this may reflect the number of channels that
 * are actually open.
 *
 * Returns: The desired/actual number of input channels.
 */
guint
psy_audio_device_get_num_input_channels(PsyAudioDevice *self)
{
    g_return_val_if_fail(PSY_IS_AUDIO_DEVICE(self), -1);

    PsyAudioDevicePrivate *priv = psy_audio_device_get_instance_private(self);

    return priv->num_inputs;
}

/**
 * psy_audio_device_set_num_output_channels:
 * @self: an instance of [class@AudioDevice]
 * @n_channels: the desired number of output channels
 *
 * Set the desired number of output channels for opening the audio device. This
 * property must be set prior to opening the device.
 *
 * Returns: TRUE if the desired number of channels is set, FALSE otherwise.
 */
gboolean
psy_audio_device_set_num_output_channels(PsyAudioDevice *self, guint n_channels)
{
    g_return_val_if_fail(PSY_IS_AUDIO_DEVICE(self), FALSE);
    PsyAudioDevicePrivate *priv = psy_audio_device_get_instance_private(self);

    if (psy_audio_device_get_is_open(self)) {
        g_warning(
            "Unable to change num-output-channels when the device is open.");
        return FALSE;
    }
    priv->num_outputs = n_channels;
    return TRUE;
}

/**
 * psy_audio_device_get_num_output_channels:
 * @self: An instance of [class@PsyAudioDevice]
 *
 * Returns the number of output channels that are desired for opening the
 * device. If the device has opened, this may reflect the number of channels
 * that are actually open.
 *
 * Returns: The desired/actual number of input channels.
 */
guint
psy_audio_device_get_num_output_channels(PsyAudioDevice *self)
{
    g_return_val_if_fail(PSY_IS_AUDIO_DEVICE(self), -1);

    PsyAudioDevicePrivate *priv = psy_audio_device_get_instance_private(self);

    return priv->num_outputs;
}

/**
 * psy_audio_device_get_frame_dur:
 * @self: an instance of [class@AudioDevice].
 *
 * Returns the duration of one sample/frame for the audio device. As the sample
 * rate set might not be a sample rate supported by the hardware, the sample
 * rate might change to a supported rate.
 * So the duration of a frame can be queried once the device is open.
 *
 * Returns:(transfer full): The [class@Psy.Duration]of a single sample.
 */
PsyDuration *
psy_audio_device_get_frame_dur(PsyAudioDevice *self)
{
    g_return_val_if_fail(PSY_IS_AUDIO_DEVICE(self), NULL);
    g_return_val_if_fail(psy_audio_device_get_is_open(self), NULL);

    gdouble dur_flt = 1.0 / psy_audio_device_get_sample_rate(self);

    return psy_duration_new(dur_flt);
}

/**
 * psy_audio_device_get_num_samples_callback:
 *
 * Get the number of samples that are processed in each iteration of the
 * audio.
 */
guint
psy_audio_device_get_num_samples_callback(PsyAudioDevice *self)
{
    g_return_val_if_fail(PSY_IS_AUDIO_DEVICE(self), -1);
    PsyAudioDevicePrivate *priv = psy_audio_device_get_instance_private(self);

    return priv->num_sample_callback;
}

/**
 * psy_audio_device_get_started:
 *
 * Returns: TRUE if the device is started
 */
gboolean
psy_audio_device_get_started(PsyAudioDevice *self)
{
    PsyAudioDevicePrivate *priv = psy_audio_device_get_instance_private(self);
    g_return_val_if_fail(PSY_IS_AUDIO_DEVICE(self), FALSE);

    return priv->started;
}

/**
 * psy_audio_device_set_started:
 *
 * stability:private
 */
void
psy_audio_device_set_started(PsyAudioDevice *self, PsyTimePoint *tp)
{
    PsyAudioDevicePrivate *priv = psy_audio_device_get_instance_private(self);

    g_return_if_fail(PSY_IS_AUDIO_DEVICE(self) && PSY_IS_TIME_POINT(tp));

    AudioStartedMsg *msg = g_new(AudioStartedMsg, 1);

    msg->tp_started   = g_object_ref(tp);
    msg->audio_device = g_object_ref(self);

    // TODO REMOVE as it is potentially blocking, hence blocking the
    // audio callback.
    g_main_context_invoke_full(priv->main_context,
                               G_PRIORITY_DEFAULT,
                               G_SOURCE_FUNC(audio_device_emit_started),
                               msg,
                               audio_started_msg_free);
}

/**
 * psy_audio_device_set_num_samples_callback:
 *
 * This function is for internal use. It allows to cache the number of samples
 * that will be handled with each iteration of the audio callback. This allows
 * the audio mixers to be prepared to buffer in and output samples.
 *
 * stability: private
 */
void
psy_audio_device_set_num_samples_callback(PsyAudioDevice *self,
                                          guint           num_samples)
{
    g_return_if_fail(PSY_IS_AUDIO_DEVICE(self));

    PsyAudioDevicePrivate *priv = psy_audio_device_get_instance_private(self);
    priv->num_sample_callback   = num_samples;
}

/**
 * psy_audio_device_enumerate_devices:
 * @self: an instance of [class@PsyAudioDevice].
 * @infos:(out callee-allocates)(array length=n_infos): An array with
 *        Device infos.
 * @n_infos:(out): the number of PsyDeviceInfo's returned in infos
 *
 * Obtain the endpoint for this audio device, the output gives info about
 * the endpoints that are available and which you can use for opening the
 * device.
 */
void
psy_audio_device_enumerate_devices(PsyAudioDevice       *self,
                                   PsyAudioDeviceInfo ***infos,
                                   guint                *n_infos)
{
    g_return_if_fail(PSY_IS_AUDIO_DEVICE(self));
    g_return_if_fail(infos != NULL && *infos == NULL);
    g_return_if_fail(n_infos != NULL);

    PsyAudioDeviceClass *cls = PSY_AUDIO_DEVICE_GET_CLASS(self);
    g_return_if_fail(cls->enumerate_devices);

    cls->enumerate_devices(self, infos, n_infos);
}

/**
 * psy_audio_device_get_output_mixer:(skip)
 * @self: an instance of [class@AudioDevice]
 *
 * Get the mixer of the audiodevice. This is a method private to psylib
 * If the device isn't open this function should return NULL.
 *
 * Stability:private:
 * Returns:(transfer none)(nullable): The audio mixer for this AudioDevice
 */
PsyAudioOutputMixer *
psy_audio_device_get_output_mixer(PsyAudioDevice *self)
{
    g_return_val_if_fail(PSY_IS_AUDIO_DEVICE(self), NULL);
    PsyAudioDevicePrivate *priv = psy_audio_device_get_instance_private(self);

    return priv->output_mixer;
}
