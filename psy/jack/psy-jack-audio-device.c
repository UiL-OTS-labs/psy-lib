
#include <jack/jack.h>

#include "psy-config.h"
#include "psy-enums.h"
#include "psy-jack-audio-device.h"

/**
 * PsyJackAudioDevice:
 *
 * PsyJackAudioDevice is a device that uses the JACK server to implement a
 * PsyAudioDevice.
 */

typedef struct _PsyJackAudioDevice {
    PsyAudioDevice parent;

    jack_client_t *client;
} PsyJackAudioDevice;

G_DEFINE_FINAL_TYPE(PsyJackAudioDevice,
                    psy_jack_audio_device,
                    PSY_TYPE_AUDIO_DEVICE)

typedef enum { PROP_NULL, NUM_PROPERTIES } PsyJackAudioDeviceProperty;

//
// static GParamSpec *jack_audio_device_properties[NUM_PROPERTIES];

// static void
// psy_jack_audio_device_set_property(GObject      *object,
//                                    guint         prop_id,
//                                    const GValue *value,
//                                    GParamSpec   *pspec)
//{
//     PsyJackAudioDevice *self = PSY_JACK_AUDIO_DEVICE(object);
//     (void) self;
//     (void) value;
//
//     switch ((PsyJackAudioDeviceProperty) prop_id) {
//     default:
//         G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
//     }
// }
//
// static void
// psy_jack_audio_device_get_property(GObject    *object,
//                                    guint       prop_id,
//                                    GValue     *value,
//                                    GParamSpec *pspec)
//{
//     PsyJackAudioDevice        *self = PSY_JACK_AUDIO_DEVICE(object);
//     PsyJackAudioDevicePrivate *priv
//         = psy_jack_audio_device_get_instance_private(self);
//     (void) value;
//     (void) priv;
//
//     switch ((PsyJackAudioDeviceProperty) prop_id) {
//     default:
//         G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
//     }
// }

static void
psy_jack_audio_device_init(PsyJackAudioDevice *self)
{
    self->client = NULL;
}

static void
psy_jack_audio_device_dispose(GObject *object)
{
    PsyJackAudioDevice *self = PSY_JACK_AUDIO_DEVICE(object);
    (void) self;

    G_OBJECT_CLASS(psy_jack_audio_device_parent_class)->dispose(object);
}

static void
psy_jack_audio_device_finalize(GObject *object)
{
    PsyJackAudioDevice *self = PSY_JACK_AUDIO_DEVICE(object);
    (void) self;
    if (psy_audio_device_get_is_open(PSY_AUDIO_DEVICE(self))) {
        psy_audio_device_close(PSY_AUDIO_DEVICE(self));
    }

    G_OBJECT_CLASS(psy_jack_audio_device_parent_class)->finalize(object);
}

static void
jack_audio_device_open(PsyAudioDevice *self, GError **error)
{
    PsyJackAudioDevice *jack_self = PSY_JACK_AUDIO_DEVICE(self);

    jack_options_t options = JackNoStartServer;
    jack_status_t  status  = 0;

    jack_self->client
        = jack_client_open("psylib-client", options, &status, NULL);
    if (!jack_self->client) {
        g_set_error(error,
                    PSY_AUDIO_DEVICE_ERROR,
                    PSY_AUDIO_DEVICE_ERROR_NO_SERVER_CONNECTION,
                    "Unable to connect to a jack server, has it been started?");
        return;
    }
}

static void
jack_audio_device_close(PsyAudioDevice *self)
{
    PsyJackAudioDevice *jack_self = PSY_JACK_AUDIO_DEVICE(self);
    jack_client_close(jack_self->client);
}

static const gchar *
jack_audio_device_get_default_name(PsyAudioDevice *self)
{
    (void) self;
    return "hw:0";
}

static void
psy_jack_audio_device_class_init(PsyJackAudioDeviceClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

    //    gobject_class->set_property = psy_jack_audio_device_set_property;
    //    gobject_class->get_property = psy_jack_audio_device_get_property;
    gobject_class->finalize = psy_jack_audio_device_finalize;
    gobject_class->dispose  = psy_jack_audio_device_dispose;

    PsyAudioDeviceClass *audio_klass = PSY_AUDIO_DEVICE_CLASS(klass);

    audio_klass->open             = jack_audio_device_open;
    audio_klass->close            = jack_audio_device_close;
    audio_klass->get_default_name = jack_audio_device_get_default_name;

    // We just use what the base class knows.
    //    audio_klass->set_name        = jack_audio_device_set_name;
    //    audio_klass->set_sample_rate = jack_audio_device_set_sample_rate;

    // we currently do not have properties
    //    g_object_class_install_properties(
    //        gobject_class, NUM_PROPERTIES, jack_audio_device_properties);
}

/* ************ public functions ******************** */

/**
 * psy_jack_audio_device_new:(constructor)
 *
 * Constructs an jack audio device.
 * This object will try to connect to or instantiate a jack server in order to
 * obtain the playback and capture devices.
 *
 * Returns: a instance of [class@PsyJackAudioDevice]
 */
PsyAudioDevice *
psy_jack_audio_device_new(void)
{
    return g_object_new(PSY_TYPE_JACK_AUDIO_DEVICE, NULL);
}
