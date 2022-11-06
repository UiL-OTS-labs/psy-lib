

#include "psy-drawing-context.h"
#include "psy-shader.h"
#include "psy-vbuffer.h"

typedef struct _PsyDrawingContextPrivate {
} PsyDrawingContextPrivate;

G_DEFINE_TYPE_WITH_PRIVATE(PsyDrawingContext, psy_drawing_context, G_TYPE_OBJECT)

typedef enum {
    PROP_NULL,
    NUM_PROPERTIES
} PsyDrawingContextProperty;

/*
 * static GParamSpec* drawing_context_properties[NUM_PROPERTIES];
 */

static void
psy_drawing_context_set_property(GObject        *object,
                         guint           prop_id,
                         const GValue   *value,
                         GParamSpec     *pspec)
{
    PsyDrawingContext* self = PSY_DRAWING_CONTEXT(object);
    (void) self;
    (void) value;

    switch((PsyDrawingContextProperty) prop_id) {
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

static void
psy_drawing_context_get_property(GObject    *object,
                         guint       prop_id,
                         GValue     *value,
                         GParamSpec *pspec)
{
    PsyDrawingContext* self = PSY_DRAWING_CONTEXT(object);
    PsyDrawingContextPrivate* priv = psy_drawing_context_get_instance_private(self);
    (void) value;
    (void) priv;

    switch((PsyDrawingContextProperty) prop_id) {
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

static void
psy_drawing_context_init(PsyDrawingContext *self)
{
    PsyDrawingContextPrivate* priv = psy_drawing_context_get_instance_private(self);
    (void) priv;
}

static void
psy_drawing_context_dispose(GObject* object)
{
    PsyDrawingContext* self = PSY_DRAWING_CONTEXT(object);
    PsyDrawingContextPrivate* priv = psy_drawing_context_get_instance_private(self);
    (void) priv;

    G_OBJECT_CLASS(psy_drawing_context_parent_class)->dispose(object);
}

static void
psy_drawing_context_finalize(GObject* object)
{
    PsyDrawingContext* self = PSY_DRAWING_CONTEXT(object);
    PsyDrawingContextPrivate* priv = psy_drawing_context_get_instance_private(self);
    (void) priv;

    G_OBJECT_CLASS(psy_drawing_context_parent_class)->dispose(object);
}


static void
psy_drawing_context_class_init(PsyDrawingContextClass* class)
{
    GObjectClass   *gobject_class = G_OBJECT_CLASS(class);

    gobject_class->set_property = psy_drawing_context_set_property;
    gobject_class->get_property = psy_drawing_context_get_property;
    gobject_class->finalize     = psy_drawing_context_finalize;
    gobject_class->dispose      = psy_drawing_context_dispose;
}

/* ************ public functions ******************** */

PsyProgram*
psy_drawing_context_create_program (PsyDrawingContext* self)
{
    g_return_val_if_fail(PSY_IS_DRAWING_CONTEXT(self), NULL);
    PsyDrawingContextClass* cls = PSY_DRAWING_CONTEXT_GET_CLASS(self);

    g_return_val_if_fail(cls->create_program, NULL);
    return cls->create_program(self);
}

PsyShader*
psy_drawing_context_create_shader (PsyDrawingContext* self)
{
    g_return_val_if_fail(PSY_IS_DRAWING_CONTEXT(self), NULL);
    PsyDrawingContextClass* cls = PSY_DRAWING_CONTEXT_GET_CLASS(self);

    g_return_val_if_fail(cls->create_shader, NULL);
    return cls->create_shader(self);
}

PsyVBuffer*
psy_drawing_context_create_vbuffer (PsyDrawingContext* self)
{
    g_return_val_if_fail(PSY_IS_DRAWING_CONTEXT(self), NULL);
    PsyDrawingContextClass* cls = PSY_DRAWING_CONTEXT_GET_CLASS(self);

    g_return_val_if_fail(cls->create_shader, NULL);
    return cls->create_vbuffer(self);
}

