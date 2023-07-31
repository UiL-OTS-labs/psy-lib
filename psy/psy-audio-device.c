

#include "psy-audio-device.h"
#include "enum-types.h"
#include "psy-config.h"
#include "psy-enums.h"

//#if defined HAVE_ALSA
//    #include "alsa/psy-alsa-audio-device.h"
//#endif

#if defined HAVE_JACK2
    #include "jack/psy-jack-audio-device.h"
#endif

/**
 * PsyAudioDevice:
 *
 * A PsyAudioDevice is a device that represents an PCI(e), USB, Firewire
 * on the mother board, etc. type of an audio interface.
 */

// clang-format off
G_DEFINE_QUARK(psy-audio-device-error-quark,
               psy_audio_device_error)

// clang-format on

typedef struct _PsyAudioDevicePrivate {
    char              *name;
    PsyAudioSampleRate sample_rate;
    GMainContext      *main_context;
    gboolean           is_open;
    gboolean           started;
} PsyAudioDevicePrivate;

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE(PsyAudioDevice,
                                    psy_audio_device,
                                    G_TYPE_OBJECT)

typedef enum {
    PROP_NULL,
    PROP_NAME,
    PROP_IS_OPEN,
    PROP_SAMPLE_RATE,
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
    case PROP_SAMPLE_RATE:
        g_value_set_enum(value, psy_audio_device_get_sample_rate(self));
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
    (void) priv;

    g_free(priv->name);

    G_OBJECT_CLASS(psy_audio_device_parent_class)->finalize(object);
}

static void
audio_device_open(PsyAudioDevice *self, GError **error)
{
    (void) error; // Error's might be raised in derived classes (backends).
    PsyAudioDevicePrivate *priv = psy_audio_device_get_instance_private(self);
    priv->is_open               = TRUE;
    g_info("Opened PsyAudioDevice %s", psy_audio_device_get_name(self));
}

static void
audio_device_close(PsyAudioDevice *self)
{
    PsyAudioDevicePrivate *priv = psy_audio_device_get_instance_private(self);
    priv->is_open               = FALSE;
    g_info("Closed PsyAudioDevice %s", psy_audio_device_get_name(self));
}

static void
audio_device_set_name(PsyAudioDevice *self, const gchar *name)
{
    PsyAudioDevicePrivate *priv = psy_audio_device_get_instance_private(self);
    g_clear_pointer(&priv->name, g_free);
    priv->name = g_strdup(name);
}

static void
audio_device_set_sample_rate(PsyAudioDevice *self, guint sample_rate)
{
    PsyAudioDevicePrivate *priv = psy_audio_device_get_instance_private(self);
    priv->sample_rate           = sample_rate;
}

static gboolean
audio_device_emit_started(AudioStartedMsg *msg)
{
    PsyAudioDevicePrivate *priv
        = psy_audio_device_get_instance_private(msg->audio_device);

    PsyDuration  *dur;
    PsyTimePoint *tp_null = psy_time_point_new();

    dur = psy_time_point_subtract(msg->tp_started, tp_null);

    g_object_unref(dur);
    g_object_unref(tp_null);

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

    klass->open            = audio_device_open;
    klass->close           = audio_device_close;
    klass->set_name        = audio_device_set_name;
    klass->set_sample_rate = audio_device_set_sample_rate;

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
                              G_PARAM_READABLE);

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
#if defined HAVE_JACK2
    return psy_jack_audio_device_new();
#elif defined HAVE_ALSA
    return psy_alsa_audio_device_new();
#else
    return NULL;
#endif
}

// TODO
// /**
//  * psy_audio_device_get_playback:
//  * @self: An instance of [class@AudioDevice] that will provide a
//  * [class@AudioPlayback]
//  *
//  * Requests the backend of this audiodevice to provide a
//  * [class@AudioPlayback] that is compatible with this context.
//  *
//  * Returns:(transfer none): An instance of [class@Texture]
//  */
// PsyTexture *
// psy_audio_device_create_playback(PsyAudioDevice *self)
// {
//     g_return_val_if_fail(PSY_IS_AUDIO_DEVICE(self), NULL);
//
//     PsyAudioDeviceClass *cls = PSY_AUDIO_DEVICE_GET_CLASS(self);
//
//     g_return_val_if_fail(cls->create_playback, NULL);
//     return cls->create_playback(self);
// }

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

void
psy_audio_device_open(PsyAudioDevice *self, GError **error)
{
    g_return_if_fail(PSY_IS_AUDIO_DEVICE(self));

    if (psy_audio_device_get_is_open(self))
        return;

    PsyAudioDeviceClass *cls = PSY_AUDIO_DEVICE_GET_CLASS(self);

    g_return_if_fail(cls->open);

    cls->open(self, error);
}

void
psy_audio_device_close(PsyAudioDevice *self)
{
    g_return_if_fail(PSY_IS_AUDIO_DEVICE(self));

    if (psy_audio_device_get_is_open(self) == FALSE)
        return;

    PsyAudioDeviceClass *cls = PSY_AUDIO_DEVICE_GET_CLASS(self);

    g_return_if_fail(cls->close);

    cls->close(self);
}

gboolean
psy_audio_device_get_is_open(PsyAudioDevice *self)
{
    g_return_val_if_fail(PSY_IS_AUDIO_DEVICE(self), FALSE);

    PsyAudioDevicePrivate *priv = psy_audio_device_get_instance_private(self);

    return priv->is_open;
}

PsyAudioSampleRate
psy_audio_device_get_sample_rate(PsyAudioDevice *self)
{
    g_return_val_if_fail(PSY_IS_AUDIO_DEVICE(self), 0);

    PsyAudioDevicePrivate *priv = psy_audio_device_get_instance_private(self);

    return priv->sample_rate;
}

gboolean
psy_audio_device_set_sample_rate(PsyAudioDevice    *self,
                                 PsyAudioSampleRate sample_rate,
                                 GError           **error)
{
    g_return_val_if_fail(PSY_IS_AUDIO_DEVICE(self), FALSE);
    g_return_val_if_fail(error == NULL || *error == NULL, FALSE);
    PsyAudioDevicePrivate *priv = psy_audio_device_get_instance_private(self);

    if (psy_audio_device_get_is_open(self)) {
        g_set_error(error,
                    PSY_AUDIO_DEVICE_ERROR,
                    PSY_AUDIO_DEVICE_ERROR_OPEN,
                    "Unable to change sample rate when the device is open.\n");
        return FALSE;
    }

    priv->sample_rate = sample_rate;
    return TRUE;
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
 * psy_audio_device_get_started:
 *
 * Determine whether the sound device is started. The device is considered
 * started when when the audio callback is running.
 *
 * Returns: TRUE when the device has started FALSE otherwise.
 */
gboolean
psy_audio_device_get_started(PsyAudioDevice *self)
{
    PsyAudioDevicePrivate *priv = psy_audio_device_get_instance_private(self);
    g_return_val_if_fail(PSY_IS_AUDIO_DEVICE(self), FALSE);

    return priv->started;
}

void
psy_audio_device_set_started(PsyAudioDevice *self, PsyTimePoint *tp)
{
    PsyAudioDevicePrivate *priv = psy_audio_device_get_instance_private(self);

    g_return_if_fail(PSY_IS_AUDIO_DEVICE(self) && PSY_IS_TIME_POINT(tp));

    AudioStartedMsg *msg = g_new(AudioStartedMsg, 1);

    msg->tp_started   = g_object_ref(tp);
    msg->audio_device = g_object_ref(self);

    g_main_context_invoke_full(priv->main_context,
                               G_PRIORITY_DEFAULT,
                               G_SOURCE_FUNC(audio_device_emit_started),
                               msg,
                               audio_started_msg_free);
}
