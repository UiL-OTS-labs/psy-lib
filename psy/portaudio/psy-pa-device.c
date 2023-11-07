
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
    PsyAudioDevice parent;
    PsyClock      *psy_clock;
    gboolean       pa_initialized;
} PsyPADevice;

G_DEFINE_FINAL_TYPE(PsyPADevice, psy_pa_device, PSY_TYPE_AUDIO_DEVICE)

typedef enum { PROP_NULL, NUM_PROPERTIES } PsyPADeviceProperty;

//
// static GParamSpec *pa_device_properties[NUM_PROPERTIES];

/* *************** private methods ************** */

/**
 * pa_device_on_process:(skip)
 * @n: The number of sample (for each channel) to process
 * @audio_device: A pointer back to the audio device
 *
 * The audio callback
 *
 * stability:private
 */
static int
pa_device_audio_callback(const void                     *input,
                         void                           *output,
                         unsigned long                   frameCount,
                         const PaStreamCallbackTimeInfo *timeInfo,
                         PaStreamCallbackFlags           statusFlags,
                         void                           *audio_device)
{
    PsyPADevice *self = audio_device;

    // TODO no system calls in audio callback..
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
    (void) self;

    G_OBJECT_CLASS(psy_pa_device_parent_class)->finalize(object);
}

static void
pa_device_open(PsyAudioDevice *self, GError **error)
{
    PsyPADevice *pa_self = PSY_PA_DEVICE(self);

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

    audio_klass->open             = pa_device_open;
    audio_klass->close            = pa_device_close;
    audio_klass->start            = pa_device_start;
    audio_klass->stop             = pa_device_stop;
    audio_klass->get_default_name = pa_device_get_default_name;

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
