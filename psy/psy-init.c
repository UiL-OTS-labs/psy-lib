
#include "psy-init.h"
#include "psy-timer-private.h"

static gint   init_count;
static GMutex init_mutex;

typedef struct _PsyInitializer {
    GObject parent;
} PsyInitializer;

G_DEFINE_TYPE(PsyInitializer, psy_initializer, G_TYPE_OBJECT)

static void
psy_initializer_init(PsyInitializer *self)
{
    (void) self;
    psy_init();
}

static void
initializer_finalize(GObject *self)
{
    (void) self;
    psy_deinit();
}

static void
psy_initializer_class_init(PsyInitializerClass *klass)
{
    GObjectClass *obj_class = G_OBJECT_CLASS(klass);
    obj_class->finalize     = initializer_finalize;
}

static void
initialize_psylib(void)
{
    timer_private_start_timer_thread();
}

static void
deinitialize_psylib(void)
{
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
