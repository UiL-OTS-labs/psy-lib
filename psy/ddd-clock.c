
#include "ddd-clock.h"

typedef struct _DddClock {
    GObject parent;
    gint64  zero_time;
} DddClock;

static gint64 g_zero_time;

typedef enum {
    PROP_NULL, //
    PROP_NOW,
    NUM_PROPERTIES
} DddClockProperty;

G_DEFINE_TYPE_WITH_CODE(
        DddClock,
        ddd_clock,
        G_TYPE_OBJECT,
        g_zero_time = g_get_monotonic_time();
        )

static GParamSpec* obj_properties[NUM_PROPERTIES];

static void
ddd_clock_set_property(
        GObject      *object,
        guint         property_id,
        const GValue *value,
        GParamSpec   *pspec
        )
{
    (void) value;
    switch ((DddClockProperty) property_id) {
        // no writable properties
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

static void
ddd_clock_get_property(
        GObject    *object,
        guint       property_id,
        GValue     *value,
        GParamSpec *pspec
        )
{
    DddClock* self = DDD_CLOCK(object);
    switch ((DddClockProperty) property_id) {
        case PROP_NOW:
            g_value_take_object(value, ddd_clock_now(self));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

static void
ddd_clock_init(DddClock* self)
{
    self->zero_time = g_zero_time;
}

static void
ddd_clock_class_init(DddClockClass* klass)
{
    GObjectClass *obj_class = G_OBJECT_CLASS(klass);

    obj_class->set_property = ddd_clock_set_property;
    obj_class->get_property = ddd_clock_get_property;

    obj_properties[PROP_NOW] = g_param_spec_object(
            "now",
            "Now",
            "The current time since the first clock has been instantiated.",
            DDD_TYPE_TIME_POINT,
            G_PARAM_READABLE
            );

    g_object_class_install_properties(obj_class, NUM_PROPERTIES, obj_properties);
}

DddClock*
ddd_clock_new()
{
    DddClock *clock = g_object_new(DDD_TYPE_CLOCK, NULL);
    return clock;
}

DddTimePoint*
ddd_clock_now(DddClock* self)
{
    g_return_val_if_fail(DDD_IS_CLOCK(self), NULL);
    gint64 num_ticks = g_get_monotonic_time() - self->zero_time;
    DddTimePoint *tp = g_object_new(
            DDD_TYPE_TIME_POINT,
            "num-ticks", num_ticks,
            NULL
            );
    return tp;
}
