

#include "psy-drawing-context.h"
#include "psy-enums.h"
#include "psy-program.h"
#include "psy-shader.h"
#include "psy-vbuffer.h"

/**
 * PsyDrawingContext:
 *
 * A `PsyDrawingContext` is the connection between the drawing backend of the
 * window and a stimulus (`PsyArtist`). The drawing context make it contains
 * general methods to draw  to the surface of a window. This makes it possible
 * to send geometry and textures to the hardware backend whether the backend is
 * OpenGL or Direct3D. The client also doesn't need to know.
 *
 * Also the drawing context contains a cache of programs, textures or some
 * other resources that could be shared among PsyArtists.
 *
 * The drawing context may contain some general shader programs in order to
 * render pictures or shapes in an arbitrary color.
 */

// clang-format off
G_DEFINE_QUARK(psy-drawing-context-error-quark,
               psy_drawing_context_error)
// clang-format on

/**
 * PSY_UNIFORM_COLOR_PROGRAM_NAME:
 *
 * The name for a string constant used to register a shader program that
 * draws using a uniform color.
 */
const gchar *PSY_UNIFORM_COLOR_PROGRAM_NAME = "uniform-color-program";

/**
 * PSY_PICTURE_PROGRAM_NAME:
 *
 * The name for a string constant used to register a shader program that
 * is able to draw a picture/texture.
 */
const gchar *PSY_PICTURE_PROGRAM_NAME = "picture-program";

typedef struct TextureAsyncLoad {
    GCancellable *cancelable;
    gsize         num_to_load;
    gsize         num_loaded;
} TextureAsyncLoad;

typedef struct _PsyDrawingContextPrivate {
    GHashTable      *shader_programs;
    GHashTable      *textures;
    TextureAsyncLoad tload_info;
} PsyDrawingContextPrivate;

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE(PsyDrawingContext,
                                    psy_drawing_context,
                                    G_TYPE_OBJECT)

typedef enum { PROP_NULL, NUM_PROPERTIES } PsyDrawingContextProperty;

/*
 * static GParamSpec* drawing_context_properties[NUM_PROPERTIES];
 */

static void
psy_drawing_context_set_property(GObject      *object,
                                 guint         prop_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
    PsyDrawingContext *self = PSY_DRAWING_CONTEXT(object);
    (void) self;
    (void) value;

    switch ((PsyDrawingContextProperty) prop_id) {
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
    PsyDrawingContext        *self = PSY_DRAWING_CONTEXT(object);
    PsyDrawingContextPrivate *priv
        = psy_drawing_context_get_instance_private(self);
    (void) value;
    (void) priv;

    switch ((PsyDrawingContextProperty) prop_id) {
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

static void
psy_drawing_context_init(PsyDrawingContext *self)
{
    PsyDrawingContextPrivate *priv
        = psy_drawing_context_get_instance_private(self);
    priv->shader_programs = g_hash_table_new_full(
        g_str_hash, g_str_equal, g_free, g_object_unref);
    priv->textures = g_hash_table_new_full(
        g_str_hash, g_str_equal, g_free, g_object_unref);
}

static void
psy_drawing_context_dispose(GObject *object)
{
    PsyDrawingContext        *self = PSY_DRAWING_CONTEXT(object);
    PsyDrawingContextPrivate *priv
        = psy_drawing_context_get_instance_private(self);

    if (priv->shader_programs) {
        g_hash_table_destroy(priv->shader_programs);
        priv->shader_programs = NULL;
    }
    if (priv->textures)
        g_clear_pointer(&priv->textures, g_hash_table_destroy);

    G_OBJECT_CLASS(psy_drawing_context_parent_class)->dispose(object);
}

static void
psy_drawing_context_finalize(GObject *object)
{
    PsyDrawingContext        *self = PSY_DRAWING_CONTEXT(object);
    PsyDrawingContextPrivate *priv
        = psy_drawing_context_get_instance_private(self);
    (void) priv;

    G_OBJECT_CLASS(psy_drawing_context_parent_class)->finalize(object);
}

static void
psy_drawing_context_class_init(PsyDrawingContextClass *class)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(class);

    gobject_class->set_property = psy_drawing_context_set_property;
    gobject_class->get_property = psy_drawing_context_get_property;
    gobject_class->finalize     = psy_drawing_context_finalize;
    gobject_class->dispose      = psy_drawing_context_dispose;
}

/* ************ public functions ******************** */

/**
 * psy_drawing_context_free_resources:
 * @self: The `PsyDrawingContext` instance that needs to free it's resources
 *
 * This function should be called when the context should giveup it's resources
 * for example when the window is unrealized, there should not be any
 * drawing any more. Also OpenGL needs to be current, in order to function,
 * these resources may be managed by the window, so the window can tell
 * when the drawing context can free the resources.
 */
void
psy_drawing_context_free_resources(PsyDrawingContext *self)
{
    g_return_if_fail(PSY_IS_DRAWING_CONTEXT(self));
    PsyDrawingContextPrivate *priv
        = psy_drawing_context_get_instance_private(self);

    if (priv->shader_programs) {
        g_hash_table_destroy(priv->shader_programs);
        priv->shader_programs = NULL;
    }
    if (priv->textures) {
        g_hash_table_destroy(priv->textures);
        priv->textures = NULL;
    }
}

/**
 * psy_drawing_context_register_program:
 * @self: an instance of `PsyDrawingContext`
 * @name: the name to use to register the program, it should not have been used
 *        to register a `PsyProgram` before.
 * @program:(transfer full):The program which you'd like to have registered. To
 *           make sure the program works with this context, you should use
 *           `psy_drawing_context_create` to make it.
 * @error:(out):If an error occurs such as using the same name twice will be
 *              returned here.
 *
 * Register an Shader program with this context. The program is registered
 * with the drawing context for future use. The context just stores the program,
 * so you'll have to add the necessary shaders and do compilation steps
 * yourself.
 */
void
psy_drawing_context_register_program(PsyDrawingContext *self,
                                     const gchar       *name,
                                     PsyProgram        *program,
                                     GError           **error)
{
    g_return_if_fail(PSY_IS_DRAWING_CONTEXT(self));
    g_return_if_fail(name);
    g_return_if_fail(PSY_IS_PROGRAM(program));
    g_return_if_fail(error == NULL || *error == NULL);

    PsyDrawingContextPrivate *priv
        = psy_drawing_context_get_instance_private(self);

    if (g_hash_table_lookup(priv->shader_programs, name) != NULL) {
        g_set_error(error,
                    PSY_DRAWING_CONTEXT_ERROR,
                    PSY_DRAWING_CONTEXT_ERROR_NAME_EXISTS,
                    "A program with the name %s has already been registered.",
                    name);
        return;
    }
    g_hash_table_insert(priv->shader_programs, g_strdup(name), program);
    g_object_ref(program);
}

/**
 * psy_drawing_context_register_texture:
 * @self: An instance of [class@DrawingContext]
 * @texture_name: The name the texture to use when retrieving the texture.
 * @texture:(transfer full): The texture to store inside of this context. We
 *          advise to use a Texture returned by
 *          psy_drawing_context_create_texture.
 * @error:(out): Errors might be returned here.
 *
 * Register a shader by name for future use. You are free to choose any name
 * not yet used by this context. Psy-lib might use absolute path names of
 * images etc.
 */
void
psy_drawing_context_register_texture(PsyDrawingContext *self,
                                     const gchar       *texture_name,
                                     PsyTexture        *texture,
                                     GError           **error)
{
    g_return_if_fail(PSY_IS_DRAWING_CONTEXT(self));
    g_return_if_fail(texture_name);
    g_return_if_fail(PSY_IS_TEXTURE(texture));
    g_return_if_fail(error == NULL || *error == NULL);

    PsyDrawingContextPrivate *priv
        = psy_drawing_context_get_instance_private(self);

    if (g_hash_table_lookup(priv->shader_programs, texture_name) != NULL) {
        g_set_error(error,
                    PSY_DRAWING_CONTEXT_ERROR,
                    PSY_DRAWING_CONTEXT_ERROR_NAME_EXISTS,
                    "A texture with the name %s has already been registered.",
                    texture_name);
        return;
    }
    g_hash_table_insert(priv->textures, g_strdup(texture_name), texture);
    g_object_ref(texture);
}

/**
 * psy_drawing_context_load_files_as_texture:
 * @self: an instance of [class@PsyDrawingContext]
 * @files:(array length=num_files)(transfer none): An array with filenames in
 *       utf8
 * @num_files: The number fo files.
 * @error: Errors may be returned here.
 *
 * This functions loads the files, into memory and will decode them.
 * TODO, also upload the files to GPU if possible.
 * The files should be unique and the will be stored inside of this drawing
 * context. They will be registered with a full canonical file name, but
 * The names may be relative. If files would have the same full path name,
 * the latter might override the first, although, the file presumably will not
 * change between
 *
 * When this is done, the texture-uploaded signal will be emitted.
 */
void
psy_drawing_context_load_files_as_texture(PsyDrawingContext *self,
                                          const gchar       *files[],
                                          gsize              num_files,
                                          GError           **error)
{
    g_return_if_fail(PSY_IS_DRAWING_CONTEXT(self));
    g_return_if_fail(files);

    GHashTable *uniques
        = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);

    for (gsize i = 0; i < num_files; i++) {
        GFile *file      = g_file_new_for_path(files[i]);
        char  *canonical = g_file_get_path(file);
        if (canonical)
            g_hash_table_insert(uniques, canonical, NULL);
        else
            g_warning("Unable to get canonical path for %s", files[i]);
        g_object_unref(file);
    }

    GHashTableIter iter;
    gpointer       key;
    g_hash_table_iter_init(&iter, uniques);

    while (g_hash_table_iter_next(&iter, &key, NULL)) {
        gchar      *path    = key;
        PsyTexture *texture = psy_drawing_context_create_texture(self);
        psy_texture_set_num_channels(texture, 4);

        psy_texture_set_path(texture, path);
        psy_texture_upload(texture, error);

        if (error && *error) {
            g_object_unref(texture);
            continue;
        }

        psy_drawing_context_register_texture(self, path, texture, error);
        g_object_unref(texture);
    }
    g_hash_table_destroy(uniques);
}

/**
 * psy_drawing_context_get_program:
 * @self: an instance of `PsyDrawingContext`
 * @name: the name used to register the Program
 *
 * Obtain a PsyProgram that was previously registered.
 *
 * Returns:(transfer none): an Instance of `PsyProgram` or NULL
 */
PsyProgram *
psy_drawing_context_get_program(PsyDrawingContext *self, const gchar *name)
{
    g_return_val_if_fail(PSY_IS_DRAWING_CONTEXT(self), NULL);
    g_return_val_if_fail(name, NULL);

    PsyDrawingContextPrivate *priv
        = psy_drawing_context_get_instance_private(self);

    return g_hash_table_lookup(priv->shader_programs, name);
}

/**
 * psy_drawing_context_get_texture:
 * @self: an instance of [class@PsyDrawingContext]
 * @name: the name used to register the Program
 *
 * Obtain a PsyTexture that was previously registered.
 *
 * Returns:(transfer none): an Instance of [class@PsyTexture] or NULL
 */
PsyTexture *
psy_drawing_context_get_texture(PsyDrawingContext *self, const gchar *name)
{
    g_return_val_if_fail(PSY_IS_DRAWING_CONTEXT(self), NULL);
    g_return_val_if_fail(name, NULL);

    PsyDrawingContextPrivate *priv
        = psy_drawing_context_get_instance_private(self);

    return g_hash_table_lookup(priv->textures, name);
}

/**
 * psy_drawing_context_create_program:
 * @self: An instance of `PsyDrawingContext`
 *
 * Returns:(transfer full): A default ShaderProgram suitable for use with
 *                          this context.
 */
PsyProgram *
psy_drawing_context_create_program(PsyDrawingContext *self)
{
    g_return_val_if_fail(PSY_IS_DRAWING_CONTEXT(self), NULL);
    PsyDrawingContextClass *cls = PSY_DRAWING_CONTEXT_GET_CLASS(self);

    g_return_val_if_fail(cls->create_program, NULL);
    return cls->create_program(self);
}

/**
 * psy_drawing_context_create_vertex_shader:
 * @self: An instance of `PsyDrawingContext`
 *
 * Returns:(transfer full): A `PsyVertexShader` suitable for use with
 *                          this context.
 */
PsyShader *
psy_drawing_context_create_vertex_shader(PsyDrawingContext *self)
{
    g_return_val_if_fail(PSY_IS_DRAWING_CONTEXT(self), NULL);
    PsyDrawingContextClass *cls = PSY_DRAWING_CONTEXT_GET_CLASS(self);

    g_return_val_if_fail(cls->create_vertex_shader, NULL);
    return cls->create_vertex_shader(self);
}

/**
 * psy_drawing_context_create_fragment_shader:
 * @self: An instance of `PsyDrawingContext`
 *
 * Returns:(transfer full): A `PsyFragmentShader` suitable for use with
 *                          this context.
 */
PsyShader *
psy_drawing_context_create_fragment_shader(PsyDrawingContext *self)
{
    g_return_val_if_fail(PSY_IS_DRAWING_CONTEXT(self), NULL);
    PsyDrawingContextClass *cls = PSY_DRAWING_CONTEXT_GET_CLASS(self);

    g_return_val_if_fail(cls->create_fragment_shader, NULL);
    return cls->create_fragment_shader(self);
}

/**
 * psy_drawing_context_create_texture:
 * @self: An instance of [class@DrawingContext] that will allocate a
 * [class@Texture]
 *
 * Requests the backend of this drawing context to allocate a
 * [class@Texture] that is compatible with this context.
 *
 * Returns:(transfer none): An instance of [class@Texture]
 */
PsyTexture *
psy_drawing_context_create_texture(PsyDrawingContext *self)
{
    g_return_val_if_fail(PSY_IS_DRAWING_CONTEXT(self), NULL);

    PsyDrawingContextClass *cls = PSY_DRAWING_CONTEXT_GET_CLASS(self);

    g_return_val_if_fail(cls->create_texture, NULL);
    return cls->create_texture(self);
}

/**
 * psy_drawing_context_create_vbuffer:
 * @self: An instance of `PsyDrawingContext`
 *
 * Returns:(transfer full): A `PsyVBuffer` suitable for use with
 *                          this context.
 */
PsyVBuffer *
psy_drawing_context_create_vbuffer(PsyDrawingContext *self)
{
    g_return_val_if_fail(PSY_IS_DRAWING_CONTEXT(self), NULL);
    PsyDrawingContextClass *cls = PSY_DRAWING_CONTEXT_GET_CLASS(self);

    g_return_val_if_fail(cls->create_vbuffer, NULL);
    return cls->create_vbuffer(self);
}
