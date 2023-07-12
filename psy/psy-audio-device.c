

#include "psy-audio-device.h"
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
    char    *name;
    guint    sample_rate;
    gboolean is_open;
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

static GParamSpec *audio_device_properties[NUM_PROPERTIES];

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
    PsyAudioDevice        *self = PSY_AUDIO_DEVICE(object);
    PsyAudioDevicePrivate *priv = psy_audio_device_get_instance_private(self);
    (void) value;
    (void) priv;

    switch ((PsyAudioDeviceProperty) prop_id) {
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

static void
psy_audio_device_init(PsyAudioDevice *self)
{
    PsyAudioDevicePrivate *priv = psy_audio_device_get_instance_private(self);
    priv->is_open               = FALSE;
    priv->name        = g_strdup(psy_audio_device_get_default_name(self));
    priv->sample_rate = PSY_AUDIO_SAMPLE_RATE_48000;
}

static void
psy_audio_device_dispose(GObject *object)
{
    PsyAudioDevice        *self = PSY_AUDIO_DEVICE(object);
    PsyAudioDevicePrivate *priv = psy_audio_device_get_instance_private(self);

    (void) priv; // currently it doesn't have other GObject's

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
        = g_param_spec_int("sample-rate",
                           "SampleRate",
                           "The sample rate of this device",
                           8000,
                           192000,
                           48000,
                           G_PARAM_READABLE);

    g_object_class_install_properties(
        gobject_class, NUM_PROPERTIES, audio_device_properties);
}

/* ************ public functions ******************** */

/**
 * psy_audio_device_new:(constructor)
 *
 * Constructs an audio device. This constructor will check which backends
 * are available on this machine and if so, it will try to get the best one
 * possible.
 * The returned device will implement an PsyAudioDevice for one specific
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
