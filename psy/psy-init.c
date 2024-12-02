
#include "psy-init.h"
#include "psy-config.h"
#include "psy-timer-private.h"

#ifdef HAVE_GSTREAMER
    #include <gst/gst.h>
#endif
#ifdef HAVE_PORTAUDIO
    #include <portaudio.h>
#endif

#ifdef WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>

    #include <timeapi.h>
#endif

static gint   init_count;
static GMutex init_mutex;

// Is used by psy_init() and -deinit()
static PsyInitializer *g_initializer;

typedef struct _PsyInitializer {
    GObject parent;
#ifdef HAVE_GSTREAMER
    guint gstreamer              : 1;
    guint gstreamer_force_unload : 1;
#endif
#ifdef HAVE_PORTAUDIO
    guint portaudio : 1;
#endif
    // guint   gtk       : 1; // we init gtk in the thread where we use it.
} PsyInitializer;

G_DEFINE_TYPE(PsyInitializer, psy_initializer, G_TYPE_OBJECT)

typedef enum {
    PROP_NULL, // GObject internal use
#ifdef HAVE_GSTREAMER
    PROP_GSTREAMER,
    PROP_GSTREAMER_FORCE_UNLOAD,
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

    g_info("Initializer::Initializing psylib count: %d", init_count);

    if (init_count == 1) {
#ifdef WIN32
        // Increases accuracy of Sleep() to roughly 1ms instead of > 10ms.
        if (timeBeginPeriod(1) != TIMERR_NOERROR) {
            g_critical(
                "Unable to improve the accuracy of the windows Sleep function, "
                "timers may be inaccurate.");
        }
#endif
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
    G_OBJECT_CLASS(psy_initializer_parent_class)->constructed(obj);
}

static void
initializer_finalize(GObject *obj)
{
    PsyInitializer *self = PSY_INITIALIZER(obj);

    g_mutex_lock(&init_mutex);
    init_count--;

    g_info("Initializer::Deinitializing psylib count: %d", init_count);

    if (init_count == 0) {

        // specific libs
        if (self->gstreamer) {
            // You are not allowed to deinit gstreamer twice. So only unload
            // gstreamer when you are really sure.
            if (self->gstreamer_force_unload)
                gst_deinit();
        }

        if (self->portaudio) {
            Pa_Terminate();
        }
        // stuff we always deinit
        timer_private_stop_timer_thread();

#if WIN32
        timeEndPeriod(1);
#endif
    }
    else if (init_count <= 0) {
        g_warning("Deinitialized psylib more often than initialized.");
    }
    g_mutex_unlock(&init_mutex);

    G_OBJECT_CLASS(psy_initializer_parent_class)->finalize(obj);
}

static void
initializer_get_property(GObject    *obj,
                         guint       id,
                         GValue     *value,
                         GParamSpec *pspec)
{
    PsyInitializer *self = PSY_INITIALIZER(obj);

    switch (id) {
#ifdef HAVE_GSTREAMER
    case PROP_GSTREAMER:
        g_value_set_boolean(value, self->gstreamer != 0);
        break;
    case PROP_GSTREAMER_FORCE_UNLOAD:
        g_value_set_boolean(value, self->gstreamer != 0);
        break;
#endif
#ifdef HAVE_PORTAUDIO
    case PROP_PORTAUDIO:
        g_value_set_boolean(value, self->portaudio != 0);
        break;
#endif
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
#ifdef HAVE_GSTREAMER
    case PROP_GSTREAMER:
        self->gstreamer = g_value_get_boolean(value);
        g_info("use gstreamer = %d", self->gstreamer == 1);
        break;
    case PROP_GSTREAMER_FORCE_UNLOAD:
        self->gstreamer_force_unload = g_value_get_boolean(value);
        break;
#endif
#ifdef HAVE_PORTAUDIO
    case PROP_PORTAUDIO:
        self->portaudio = g_value_get_boolean(value);
        g_info("use portaudio = %d", self->portaudio == 1);
        break;
#endif
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

#ifdef HAVE_GSTREAMER
    /**
     * Initializer:gstreamer
     *
     * If set to true psylib will init gstreamer on your behalf
     */
    initializer_properties[PROP_GSTREAMER] = g_param_spec_boolean(
        "gstreamer",
        "GStreamer",
        "Initialize gstreamer along with the rest of psylib",
        TRUE,
        G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

    /**
     * Initializer:gstreamer-force-unload
     *
     * If set to true psylib will deinit gstreamer on your behalf. You'll have
     * to notice that doing this twice WILL crash your program, so you'll
     * probably want to keep this off, as that doesn't hurt.
     */
    initializer_properties[PROP_GSTREAMER_FORCE_UNLOAD]
        = g_param_spec_boolean("gstreamer-force-unload",
                               "GStreamerForceUnload",
                               "deinit gstreamer when done. Keep it FALSE.",
                               FALSE,
                               G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
#endif

#ifdef HAVE_PORTAUDIO

    /**
     * Initializer:portaudio
     *
     * If set to true psylib will init portaudio on your behalf
     */
    initializer_properties[PROP_PORTAUDIO] = g_param_spec_boolean(
        "portaudio",
        "PortAudio",
        "Initialize portaudio along with the rest of psylib",
        TRUE,
        G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
#endif

    g_object_class_install_properties(
        obj_class, NUM_PROPS, initializer_properties);
}

/**
 * psy_initializer_new:(constructor)
 *
 * Returns an object, that initializes psylib, and all of its dependencies.
 * You can use g_object_new(), yourself, inorder to set a number of properties,
 * using the properties you can avoid loading of some dependencies. This
 * typically is not recommended, however, it is possible if you know what
 * librayies you really need.
 *
 * Returns:(transfer full): an instance of [class@Initializer], you should
 *     free this either with g_object_unref or [method@Psy.Initializer.free].
 *     in many of the bindings, free will not be necessary. As the bindings
 *     will do this on your behalf.
 */
PsyInitializer *
psy_initializer_new(void)
{
    return g_object_new(PSY_TYPE_INITIALIZER, NULL);
}

/**
 * psy_initializer_free:(skip)
 *
 * Destroys, the initializer and thereby undos the initialization of psylib.
 */
void
psy_initializer_free(PsyInitializer *self)
{
    g_object_unref(self);
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
        g_initializer = psy_initializer_new();
    }

    g_mutex_unlock(&init_mutex);
}

void
psy_deinit(void)
{
    g_mutex_lock(&init_mutex);
    init_count--;
    if (init_count == 0) {
        g_clear_object(&g_initializer);
    }
    else if (init_count < 0) {
        g_warning("psylib: init_count = %d", init_count);
    }

    g_mutex_unlock(&init_mutex);
}
