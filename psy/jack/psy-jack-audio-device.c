
#include <jack/jack.h>
#include <stdio.h>

#include "enum-types.h"
#include "psy-clock.h"
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

    // Owned by audio callback when open
    PsyClock  *psy_clock;
    GPtrArray *capture_ports;
    GPtrArray *playback_ports;

} PsyJackAudioDeviceclock;

G_DEFINE_FINAL_TYPE(PsyJackAudioDevice,
                    psy_jack_audio_device,
                    PSY_TYPE_AUDIO_DEVICE)

typedef enum { PROP_NULL, NUM_PROPERTIES } PsyJackAudioDeviceProperty;

//
// static GParamSpec *jack_audio_device_properties[NUM_PROPERTIES];

/* *************** private methods ************** */

static int
jack_audio_device_on_process(jack_nframes_t n, void *audio_device)
{
    PsyJackAudioDevice *self = audio_device;
    PsyTimePoint       *tp   = psy_clock_now(self->psy_clock);

    if (G_UNLIKELY(!psy_audio_device_get_started(PSY_AUDIO_DEVICE(self)))) {
        psy_audio_device_set_started(PSY_AUDIO_DEVICE(self), tp);
    }
    // Read first, because the input might be desired for the output.

    for (guint port = 0; port < self->capture_ports->len; port++) {
        ;
    }

    for (guint i = 0; i < self->playback_ports->len; i++) {
        jack_default_audio_sample_t *samples;
        jack_port_t                 *port = self->playback_ports->pdata[i];

        samples = jack_port_get_buffer(port, n);

        memset(samples, '0', sizeof(jack_default_audio_sample_t) * n);
    }

    g_object_unref(tp);

    return 0;
}

static void
jack_audio_device_on_shut_down(void *audio_device)
{
    PsyAudioDevice *self = audio_device;
}

static int
jack_audio_device_on_sample_rate_change(jack_nframes_t n, void *audio_device)
{
    PsyJackAudioDevice *self = audio_device;
    (void) n;

    return 0;
}

static int
jack_audio_device_on_xrun(void *audio_device)
{
    PsyJackAudioDevice *self = audio_device;

    return 0;
}

static void
jack_audio_device_on_latency(jack_latency_callback_mode_t mode,
                             void                        *audio_device)
{
    PsyJackAudioDevice *self = audio_device;

    if (mode == JackCaptureLatency) {
        // TODO
    }
    else if (mode == JackPlaybackLatency) {
        // TODO
    }
}

static void
jack_audio_device_on_error(const char *error)
{
    g_error("JackAudioDevice encountered an error: %s", error);
}

static int
jack_audio_device_register_callbacks(PsyJackAudioDevice *self)
{
    jack_client_t *client = self->client; // alias
    int            status = 0;
    status
        = jack_set_process_callback(client, jack_audio_device_on_process, self);
    if (status) {
        g_error("Unable to set process callback: %d", status);
        return status;
    }

    jack_on_shutdown(client, jack_audio_device_on_shut_down, self);

    status = jack_set_sample_rate_callback(
        client, jack_audio_device_on_sample_rate_change, self);
    if (status) {
        g_error("Unable to set sample_rate callback: %d", status);
        return status;
    }

    status = jack_set_xrun_callback(client, jack_audio_device_on_xrun, self);
    if (status) {
        g_error("Unable to set on_xrun callback: %d", status);
        return status;
    }

    status
        = jack_set_latency_callback(client, jack_audio_device_on_latency, self);
    if (status) {
        g_error("Unable to set on_latency callback: %d", status);
        return status;
    }

    jack_set_error_function(jack_audio_device_on_error);

    return status;
}

static int
jack_audio_device_get_ports(PsyJackAudioDevice *self)
{
    jack_client_t *client = self->client; // alias

    enum JackPortFlags playback_flags = JackPortIsPhysical | JackPortIsInput;
    enum JackPortFlags capture_flags  = JackPortIsPhysical | JackPortIsOutput;

    const char **capture_ports
        = jack_get_ports(client, NULL, JACK_DEFAULT_AUDIO_TYPE, capture_flags);
    const char **playback_ports
        = jack_get_ports(client, NULL, JACK_DEFAULT_AUDIO_TYPE, capture_flags);

    gchar port_name[1024];

    for (guint i = 0;; i++) {
        if (capture_ports[i] == NULL) {
            break;
        }
        sprintf(port_name, "psy-input-%d", i);
        jack_port_t *port = jack_port_register(
            client, port_name, JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);

        g_ptr_array_add(self->capture_ports, port);
    }

    for (int i = 0;; i++) {
        if (playback_ports[i] == NULL) {
            break;
        }
        sprintf(port_name, "psy-output-%d", i);
        jack_port_t *port = jack_port_register(
            client, port_name, JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);

        g_ptr_array_add(self->playback_ports, port);
    }

    jack_free(capture_ports);
    jack_free(playback_ports);
    return 0;
}

static int
jack_audio_device_connect_ports(PsyJackAudioDevice *self)
{
    jack_client_t *client = self->client; // alias

    enum JackPortFlags playback_flags = JackPortIsPhysical | JackPortIsInput;
    enum JackPortFlags capture_flags  = JackPortIsPhysical | JackPortIsOutput;

    const char **capture_ports
        = jack_get_ports(client, NULL, JACK_DEFAULT_AUDIO_TYPE, capture_flags);
    const char **playback_ports
        = jack_get_ports(client, NULL, JACK_DEFAULT_AUDIO_TYPE, playback_flags);

    int status;

    for (unsigned i = 0; i < self->capture_ports->len; i++) {
        jack_port_t *port = self->capture_ports->pdata[i];

        status = jack_connect(client, capture_ports[i], jack_port_name(port));
        if (status) {
            g_critical("Unable to connect ports %s to %s, status is %x",
                       capture_ports[i],
                       jack_port_name(port),
                       status);
        }
    }
    for (unsigned i = 0; i < self->playback_ports->len; i++) {
        jack_port_t *port = self->playback_ports->pdata[i];

        status = jack_connect(client, jack_port_name(port), playback_ports[i]);
        if (status) {
            g_critical("Unable to connect ports %s to %s, status is %x",
                       jack_port_name(port),
                       playback_ports[i],
                       status);
        }
    }
    return 0;
}

/* *********** virtual methods ***************** */

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

    self->capture_ports  = g_ptr_array_new();
    self->playback_ports = g_ptr_array_new();

    self->psy_clock = psy_clock_new();
}

static void
psy_jack_audio_device_dispose(GObject *object)
{
    PsyJackAudioDevice *self = PSY_JACK_AUDIO_DEVICE(object);
    (void) self;

    // self->jack_client is closed in psy_audio_device_close()
    g_clear_object(&self->psy_clock);

    G_OBJECT_CLASS(psy_jack_audio_device_parent_class)->dispose(object);
}

static void
psy_jack_audio_device_finalize(GObject *object)
{
    PsyJackAudioDevice *self = PSY_JACK_AUDIO_DEVICE(object);
    (void) self;

    // self->jack_client is closed in psy_audio_device_close()

    g_ptr_array_free(self->capture_ports, TRUE);
    g_ptr_array_free(self->playback_ports, TRUE);

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

    status = jack_audio_device_register_callbacks(PSY_JACK_AUDIO_DEVICE(self));
    if (status) {
        g_set_error(error,
                    psy_audio_device_error_get_type(),
                    PSY_AUDIO_DEVICE_ERROR_FAILED,
                    "Unable to set jack callbacks");
        return;
    }

    // Create ports here or below register we can connect them.
    jack_audio_device_get_ports(PSY_JACK_AUDIO_DEVICE(self));

    status = jack_activate(jack_self->client);
    if (status) {
        g_set_error(error,
                    psy_audio_device_error_get_type(),
                    PSY_AUDIO_DEVICE_ERROR_FAILED,
                    "Unable to activate client: %d",
                    status);
        return;
    }

    jack_audio_device_connect_ports(PSY_JACK_AUDIO_DEVICE(self));

    PSY_AUDIO_DEVICE_CLASS(psy_jack_audio_device_parent_class)
        ->open(self, error);
}

static void
jack_audio_device_close(PsyAudioDevice *self)
{
    PsyJackAudioDevice *jack_self = PSY_JACK_AUDIO_DEVICE(self);
    jack_client_close(jack_self->client);
    PSY_AUDIO_DEVICE_CLASS(psy_jack_audio_device_parent_class)->close(self);
}

static const gchar *
jack_audio_device_get_default_name(PsyAudioDevice *self)
{
    (void) self;
    // NOTE This is only correct when using ALSA as jack backend.
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

    // We currently do not have properties
    //    g_object_class_install_properties(
    //        gobject_class, NUM_PROPERTIES, jack_audio_device_properties);
}

/* ************ public functions ******************** */

/**
 * psy_jack_audio_device_new:(constructor)
 *
 * Constructs an jack audio device.
 * This object will try to connect to or instantiate a jack server in order
 * to obtain the playback and capture devices.
 *
 * Returns: a instance of [class@PsyJackAudioDevice]
 */
PsyAudioDevice *
psy_jack_audio_device_new(void)
{
    return g_object_new(PSY_TYPE_JACK_AUDIO_DEVICE, NULL);
}
