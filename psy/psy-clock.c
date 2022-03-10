
#include "psy-clock.h"

typedef struct _PsyClock {
    GObject parent;
    gint64  zero_time;
} PsyClock;

static gint64 g_zero_time;

typedef enum {
    PROP_NULL, //
    PROP_NOW,
    NUM_PROPERTIES
} PsyClockProperty;

G_DEFINE_TYPE_WITH_CODE(
        PsyClock,
        psy_clock,
        G_TYPE_OBJECT,
        g_zero_time = g_get_monotonic_time();
        )

static GParamSpec* obj_properties[NUM_PROPERTIES];

static void
psy_clock_set_property(
        GObject      *object,
        guint         property_id,
        const GValue *value,
        GParamSpec   *pspec
        )
{
    (void) value;
    switch ((PsyClockProperty) property_id) {
        // no writable properties
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

static void
psy_clock_get_property(
        GObject    *object,
        guint       property_id,
        GValue     *value,
        GParamSpec *pspec
        )
{
    PsyClock* self = PSY_CLOCK(object);
    switch ((PsyClockProperty) property_id) {
        case PROP_NOW:
            g_value_take_object(value, psy_clock_now(self));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

static void
psy_clock_init(PsyClock* self)
{
    self->zero_time = g_zero_time;
}

static void
psy_clock_class_init(PsyClockClass* klass)
{
    GObjectClass *obj_class = G_OBJECT_CLASS(klass);

    obj_class->set_property = psy_clock_set_property;
    obj_class->get_property = psy_clock_get_property;

    obj_properties[PROP_NOW] = g_param_spec_object(
            "now",
            "Now",
            "The current time since the first clock has been instantiated.",
            PSY_TYPE_TIME_POINT,
            G_PARAM_READABLE
            );

    g_object_class_install_properties(obj_class, NUM_PROPERTIES, obj_properties);
}

PsyClock*
psy_clock_new()
{
    PsyClock *clock = g_object_new(PSY_TYPE_CLOCK, NULL);
    return clock;
}

PsyTimePoint*
psy_clock_now(PsyClock* self)
{
    g_return_val_if_fail(PSY_IS_CLOCK(self), NULL);
    gint64 num_ticks = g_get_monotonic_time() - self->zero_time;
    PsyTimePoint *tp = g_object_new(
            PSY_TYPE_TIME_POINT,
            "num-ticks", num_ticks,
            NULL
            );
    return tp;
}
