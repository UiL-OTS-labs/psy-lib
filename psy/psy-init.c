
#include "psy-init.h"
#include "psy-config.h"
#include "psy-timer-private.h"

#ifdef HAVE_GSTREAMER
    #include <gst/gst.h>
#endif
#ifdef HAVE_PORTAUDIO
    #include <portaudio.h>
#endif

static gint   init_count;
static GMutex init_mutex;

typedef struct _PsyInitializer {
    GObject parent;
    guint   all : 1;
#ifdef HAVE_GSTREAMER
    guint gstreamer : 1;
#endif
#ifdef HAVE_PORTAUDIO
    guint portaudio : 1;
#endif
    // guint   gtk       : 1; // we init gtk in the thread where we use it.
} PsyInitializer;

G_DEFINE_TYPE(PsyInitializer, psy_initializer, G_TYPE_OBJECT)

typedef enum {
    PROP_NULL, // GObject internal use
    PROP_ALL,  // Turn everything on.
#ifdef HAVE_GSTREAMER
    PROP_GSTREAMER,
#endif
#ifdef HAVE_PORTAUDIO
    PROP_PORTAUDIO,
#endif
    // PROP_GTK,  Gtk is initialized in the thread where it should run.
    NUM_PROPS
} PsyInitializerProperty;

static GParamSpec *initializer_properties[NUM_PROPS] = {0};

static void
psy_initializer_init(PsyInitializer *self)
{
    (void) self;
}

static void
initializer_constructed(GObject *obj)
{
    PsyInitializer *self = PSY_INITIALIZER(obj);

    g_mutex_lock(&init_mutex);

    init_count++;

    if (init_count == 1) {

        // stuff we always init
        timer_private_start_timer_thread();

        // specific libs
#ifdef HAVE_GSTREAMER
        if (self->gstreamer) {
            gst_init(NULL, NULL);
        }
#endif
#ifdef HAVE_PORTAUDIO
        if (self->portaudio) {
            Pa_Initialize();
        }
#endif
    }
    else {
        g_warning(
            "Constructed an initializer when psylib seems already initialized");
    }
    g_mutex_unlock(&init_mutex);
}

static void
initializer_finalize(GObject *obj)
{
    PsyInitializer *self = PSY_INITIALIZER(obj);
    g_mutex_lock(&init_mutex);
    init_count--;
    if (init_count == 0) {
        // stuff we always deinit
        timer_private_stop_timer_thread();

        // specific libs
        if (self->gstreamer) {
            gst_deinit();
        }

        if (self->portaudio) {
            Pa_Terminate();
        }
    }
    else if (init_count <= 0) {
        g_warning("Deinitialized psylib more often than initialized.");
    }
    g_mutex_unlock(&init_mutex);
}

static void
initializer_get_property(GObject    *obj,
                         guint       id,
                         GValue     *value,
                         GParamSpec *pspec)
{
    PsyInitializer *self = PSY_INITIALIZER(obj);

    switch (id) {
    case PROP_ALL:
        g_value_set_boolean(value, self->all != 0);
        break;
    case PROP_GSTREAMER:
        g_value_set_boolean(value, self->gstreamer != 0);
        break;
    case PROP_PORTAUDIO:
        g_value_set_boolean(value, self->portaudio != 0);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, id, pspec);
    }
}

static void
initializer_set_property(GObject      *obj,
                         guint         id,
                         const GValue *value,
                         GParamSpec   *pspec)
{
    PsyInitializer *self = PSY_INITIALIZER(obj);

    switch (id) {
    case PROP_ALL:
        self->all = g_value_get_boolean(value);
        if (self->all) {
            self->gstreamer = TRUE;
            self->portaudio = TRUE;
        }
        break;
    case PROP_GSTREAMER:
        self->gstreamer = g_value_get_boolean(value);
        break;
    case PROP_PORTAUDIO:
        self->portaudio = g_value_get_boolean(value);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, id, pspec);
    }
}

static void
psy_initializer_class_init(PsyInitializerClass *klass)
{
    GObjectClass *obj_class = G_OBJECT_CLASS(klass);
    obj_class->get_property = initializer_get_property;
    obj_class->set_property = initializer_set_property;
    obj_class->finalize     = initializer_finalize;
    obj_class->constructed  = initializer_constructed;

    initializer_properties[PROP_ALL]
        = g_param_spec_boolean("all",
                               "All",
                               "Initialize all libs psylib uses",
                               TRUE,
                               G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

#ifdef HAVE_GSTREAMER
    initializer_properties[PROP_GSTREAMER] = g_param_spec_boolean(
        "gstreamer",
        "GStreamer",
        "Initialize gstreamer along with the rest of psylib",
        FALSE,
        G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
#endif

#ifdef HAVE_PORTAUDIO
    initializer_properties[PROP_PORTAUDIO] = g_param_spec_boolean(
        "portaudio",
        "PortAudio",
        "Initialize portaudio along with the rest of psylib",
        FALSE,
        G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
#endif

    g_object_class_install_properties(
        obj_class, NUM_PROPS, initializer_properties);
}

static void
initialize_psylib(void)
{
    timer_private_start_timer_thread();
#ifdef HAVE_GSTREAMER
    gst_init(NULL, NULL);
#endif
#ifdef HAVE_PORTAUDIO
    Pa_Initialize();
#endif
}

static void
deinitialize_psylib(void)
{
#ifdef HAVE_PORTAUDIO
    Pa_Terminate();
#endif
#ifdef HAVE_GSTREAMER
    gst_deinit();
#endif
    timer_private_stop_timer_thread();
}

/**
 * psy_init:
 *
 * Some of psylib's functions rely on psylib being initialized. So you
 * should call it once before using other psylib functions, otherwise it's
 * likely that psylib won't play nicely.
 */
void
psy_init(void)
{
    g_mutex_lock(&init_mutex);

    init_count++;

    if (init_count == 1) {
        g_info("Initializing psylib");
        initialize_psylib();
    }

    g_mutex_unlock(&init_mutex);
}

void
psy_deinit(void)
{
    g_mutex_lock(&init_mutex);
    init_count--;
    if (init_count == 0) {
        deinitialize_psylib();
    }
    else if (init_count < 0) {
        g_warning("psylib: init_count = %d", init_count);
    }

    g_mutex_unlock(&init_mutex);
}
