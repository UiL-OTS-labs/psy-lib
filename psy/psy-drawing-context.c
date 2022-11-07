

#include "psy-drawing-context.h"
#include "psy-program.h"
#include "psy-shader.h"
#include "psy-vbuffer.h"
#include "psy-enums.h"

/**
 * PsyDrawingContext:
 *
 * A `PsyDrawingContext` is the connection between the drawing backend of the
 * window and a stimulus (`PsyArtist`). The drawing context make it contains
 * general methods to draw  to the surface of a window. This makes it possible
 * to send geometry and textures to the hardware backend whether the backend is
 * OpenGL or Direct3D. The client also doesn't need to know.
 *
 * The drawing context may contain some general shader programs in order to
 * render pictures or shapes in an arbitrary color.
 */

G_DEFINE_QUARK(psy-context-error-quark, psy_context_error)

typedef struct _PsyDrawingContextPrivate {
    GHashTable *shader_programs;
} PsyDrawingContextPrivate;

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE(PsyDrawingContext, psy_drawing_context, G_TYPE_OBJECT)

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
    priv->shader_programs = g_hash_table_new_full(
            g_str_hash,
            g_str_equal,
            g_free,
            g_object_unref
            );
}

static void
psy_drawing_context_dispose(GObject* object)
{
    PsyDrawingContext* self = PSY_DRAWING_CONTEXT(object);
    PsyDrawingContextPrivate* priv = psy_drawing_context_get_instance_private(self);

    if (priv->shader_programs) {
        g_hash_table_destroy(priv->shader_programs);
        priv->shader_programs = NULL;
    }

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


/**
 * psy_drawing_context_register:
 * @self: an instance of `PsyDrawingContext`
 * @name: the name to use to register the program, it should not have been used
 *        to register a `PsyProgram` before.
 * @program: The program which you'd like to have registered. To make sure
 *           the program works with this context, you should use
 *           `psy_drawing_context_create` to make it.
 * @error:(out):If an error occurs such as using the same name twice will be
 *              returned here.
 *
 * Register an Shader program with this context. The program is registered
 * with the drawing context for future use. The context just stores the program,
 * so you'll have to add the necessary shaders and do compilation steps yourself.
 */
void
psy_drawing_context_register_program (
        PsyDrawingContext* self,
        const gchar* name,
        PsyProgram* program,
        GError** error
        )
{
    g_return_if_fail(PSY_IS_DRAWING_CONTEXT(self));
    g_return_if_fail(name);
    g_return_if_fail(PSY_IS_PROGRAM(program));
    g_return_if_fail(error == NULL || *error != NULL);

    PsyDrawingContextPrivate* priv = psy_drawing_context_get_instance_private(self);

    if (g_hash_table_lookup(priv->shader_programs, name) != NULL) {
        g_set_error(error,
                PSY_DRAWING_CONTEXT_ERROR,
                PSY_DRAWING_CONTEXT_ERROR_NAME_EXISTS,
                "A program with the name %s has already been registered.",
                name
                );
        return;
    }
    g_hash_table_insert(
            priv->shader_programs,
            g_strdup(name),
            program
            );
}

/**
 * psy_drawing_context_get_program:
 * @self: an instance of `PsyDrawingContext`
 * @name: the name used to register the Program
 *
 * Obtain a PsyProgram previously that was previously registered.
 *
 * Returns: an Instance of `PsyProgram` or NULL
 */
PsyProgram*
psy_drawing_context_get_program(PsyDrawingContext* self, const gchar* name)
{
    g_return_val_if_fail(PSY_IS_DRAWING_CONTEXT(self), NULL);
    g_return_val_if_fail(name, NULL);

    PsyDrawingContextPrivate* priv = psy_drawing_context_get_instance_private(self);

    return g_hash_table_lookup(priv->shader_programs, name);
}

PsyProgram*
psy_drawing_context_create_program (PsyDrawingContext* self)
{
    g_return_val_if_fail(PSY_IS_DRAWING_CONTEXT(self), NULL);
    PsyDrawingContextClass* cls = PSY_DRAWING_CONTEXT_GET_CLASS(self);

    g_return_val_if_fail(cls->create_program, NULL);
    return cls->create_program(self);
}

PsyShader*
psy_drawing_context_create_vertex_shader (PsyDrawingContext* self)
{
    g_return_val_if_fail(PSY_IS_DRAWING_CONTEXT(self), NULL);
    PsyDrawingContextClass* cls = PSY_DRAWING_CONTEXT_GET_CLASS(self);

    g_return_val_if_fail(cls->create_vertex_shader, NULL);
    return cls->create_vertex_shader(self);
}

PsyShader*
psy_drawing_context_create_fragment_shader (PsyDrawingContext* self)
{
    g_return_val_if_fail(PSY_IS_DRAWING_CONTEXT(self), NULL);
    PsyDrawingContextClass* cls = PSY_DRAWING_CONTEXT_GET_CLASS(self);

    g_return_val_if_fail(cls->create_fragment_shader, NULL);
    return cls->create_fragment_shader(self);
}

PsyVBuffer*
psy_drawing_context_create_vbuffer (PsyDrawingContext* self)
{
    g_return_val_if_fail(PSY_IS_DRAWING_CONTEXT(self), NULL);
    PsyDrawingContextClass* cls = PSY_DRAWING_CONTEXT_GET_CLASS(self);

    g_return_val_if_fail(cls->create_vbuffer, NULL);
    return cls->create_vbuffer(self);
}

