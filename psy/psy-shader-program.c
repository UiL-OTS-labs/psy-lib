

#include "psy-shader-program.h"
#include "psy-matrix4.h"
#include "psy-shader.h"

typedef struct _PsyShaderProgramPrivate {
    PsyShader *vertex_shader;
    PsyShader *fragment_shader;
} PsyShaderProgramPrivate;

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE(PsyShaderProgram,
                                    psy_shader_program,
                                    G_TYPE_OBJECT)

typedef enum { PROP_NULL, NUM_PROPERTIES } PsyShaderProgramProperty;

/*
 * static GParamSpec* program_properties[NUM_PROPERTIES];
 */

static void
shader_program_set_property(GObject      *object,
                            guint         prop_id,
                            const GValue *value,
                            GParamSpec   *pspec)
{
    PsyShaderProgram *self = PSY_SHADER_PROGRAM(object);
    (void) self;
    (void) value;

    switch ((PsyShaderProgramProperty) prop_id) {
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

static void
shader_program_get_property(GObject    *object,
                            guint       prop_id,
                            GValue     *value,
                            GParamSpec *pspec)
{
    PsyShaderProgram        *self = PSY_SHADER_PROGRAM(object);
    PsyShaderProgramPrivate *priv
        = psy_shader_program_get_instance_private(self);
    (void) value;
    (void) priv;

    switch ((PsyShaderProgramProperty) prop_id) {
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

static void
psy_shader_program_init(PsyShaderProgram *self)
{
    PsyShaderProgramPrivate *priv
        = psy_shader_program_get_instance_private(self);
    (void) priv;
}

static void
shader_program_dispose(GObject *object)
{
    PsyShaderProgram        *self = PSY_SHADER_PROGRAM(object);
    PsyShaderProgramPrivate *priv
        = psy_shader_program_get_instance_private(self);
    (void) priv;

    g_clear_object(&priv->vertex_shader);
    g_clear_object(&priv->fragment_shader);

    G_OBJECT_CLASS(psy_shader_program_parent_class)->dispose(object);
}

static void
shader_program_finalize(GObject *object)
{
    PsyShaderProgram        *self = PSY_SHADER_PROGRAM(object);
    PsyShaderProgramPrivate *priv
        = psy_shader_program_get_instance_private(self);
    (void) priv;

    G_OBJECT_CLASS(psy_shader_program_parent_class)->finalize(object);
}

static void
shader_program_set_vertex_shader(PsyShaderProgram *self,
                                 PsyShader        *shader,
                                 GError          **error)
{
    PsyShaderProgramPrivate *priv
        = psy_shader_program_get_instance_private(self);

    if (!psy_shader_is_compiled(shader)) {
        psy_shader_compile(shader, error);
        if (*error)
            return;
    }

    if (priv->vertex_shader) {
        g_object_unref(priv->vertex_shader);
    }
    priv->vertex_shader = shader;
}

static void
shader_program_set_fragment_shader(PsyShaderProgram *self,
                                   PsyShader        *shader,
                                   GError          **error)
{
    (void) error; // for subclasses
    PsyShaderProgramPrivate *priv
        = psy_shader_program_get_instance_private(self);

    if (!psy_shader_is_compiled(shader)) {
        psy_shader_compile(shader, error);
        if (*error)
            return;
    }

    if (priv->fragment_shader) {
        g_object_unref(priv->fragment_shader);
    }

    priv->fragment_shader = shader;
}

static void
psy_shader_program_class_init(PsyShaderProgramClass *class)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(class);

    gobject_class->set_property = shader_program_set_property;
    gobject_class->get_property = shader_program_get_property;
    gobject_class->finalize     = shader_program_finalize;
    gobject_class->dispose      = shader_program_dispose;

    class->set_vertex_shader   = shader_program_set_vertex_shader;
    class->set_fragment_shader = shader_program_set_fragment_shader;
}

/* ************ public functions ******************** */

/**
 * psy_shader_program_set_vertex_shader:
 * @self: The program to which you want to add a shader
 * @shader:(transfer full): The instance of [class@Shader] that you want add
 *                          to this program. The program will be compiled if
 *                         neccessary.
 * @error:(out): If an error occurs it will be returned here.
 *               An optional error that might occur is for example that
 *               the shader can't be compiled.
 *
 * Adds a shader to the program if the shader isn't ready for use the
 * backend of this Shader program (currently only a OpenGL program) will
 * try to make it ready for use. When the Shader hasn't a valid source
 * for example you can expect that this function will fail, as it
 * needs a valid source.
 * If this program already has a vertex shader it will be removed.
 */
void
psy_shader_program_set_vertex_shader(PsyShaderProgram *self,
                                     PsyShader        *shader,
                                     GError          **error)
{
    g_return_if_fail(PSY_IS_SHADER_PROGRAM(self));
    g_return_if_fail(PSY_IS_SHADER(shader));
    g_return_if_fail(error != NULL && *error == NULL);

    PsyShaderProgramClass *klass = PSY_SHADER_PROGRAM_GET_CLASS(self);

    g_return_if_fail(klass->set_vertex_shader != NULL);

    klass->set_vertex_shader(self, shader, error);
}

/**
 * psy_shader_program_get_vertex_shader:
 * @self: An instance of [class@ShaderProgram]
 *
 * Get the vertex shader of this program if it has been set.
 *
 * Returns:(transfer none)(nullable): The instance of [class@Shader] that
 *         that represents the vertex shader of this class.
 */
PsyShader *
psy_shader_program_get_vertex_shader(PsyShaderProgram *self)
{
    g_return_val_if_fail(PSY_IS_SHADER_PROGRAM(self), NULL);

    PsyShaderProgramPrivate *priv
        = psy_shader_program_get_instance_private(self);

    return priv->vertex_shader;
}

/**
 * psy_shader_program_set_vertex_shader_source:
 * @self: The program to which a shader should be added.
 * @source:(transfer none): The string that should be a valid source code
 *                          for a vertex shader.
 * @error:(out): If an error occurs it will be returned here.
 *               An optional error that might occur is for example that
 *               the shader can't be compiled.
 *
 * Adds a shader to the program via adding source to a to be created shader.
 * The backend of this Shader program (currently only a OpenGL program) will
 * try to make it ready for use. When the Shader hasn't a valid source
 * for example you can expect that this function will fail, as it
 * needs a valid source.
 * This method is abstract, hence subclasses must implement it. Sub classes
 * create a appropriate shader for this program. So a opengl program create
 * a opengl vertex shader. The derived method will call
 * [method@ShaderProgram.set_vertex_shader] on you behalf, to finalize this
 * process.
 */
void
psy_shader_program_set_vertex_shader_source(PsyShaderProgram *self,
                                            const gchar      *source,
                                            GError          **error)
{
    g_return_if_fail(PSY_IS_SHADER_PROGRAM(self));
    g_return_if_fail(source != NULL);
    g_return_if_fail(error != NULL && *error == NULL);

    PsyShaderProgramClass *klass = PSY_SHADER_PROGRAM_GET_CLASS(self);

    g_return_if_fail(klass->set_vertex_shader_source != NULL);

    klass->set_vertex_shader_source(self, source, error);
}

void
psy_shader_program_set_vertex_shader_from_file(PsyShaderProgram *self,
                                               GFile            *shader_file,
                                               GError          **error)
{
    g_return_if_fail(PSY_IS_SHADER_PROGRAM(self));
    g_return_if_fail(G_IS_FILE(shader_file));
    g_return_if_fail(error != NULL && *error == NULL);

    PsyShaderProgramClass *klass = PSY_SHADER_PROGRAM_GET_CLASS(self);

    g_return_if_fail(klass->set_vertex_shader_from_file != NULL);

    klass->set_vertex_shader_from_file(self, shader_file, error);
}

void
psy_shader_program_set_vertex_shader_from_path(PsyShaderProgram *self,
                                               const gchar      *shader_path,
                                               GError          **error)
{
    g_return_if_fail(PSY_IS_SHADER_PROGRAM(self));
    g_return_if_fail(shader_path != NULL);
    g_return_if_fail(error != NULL && *error == NULL);

    PsyShaderProgramClass *klass = PSY_SHADER_PROGRAM_GET_CLASS(self);

    g_return_if_fail(klass->set_vertex_shader_from_path != NULL);

    klass->set_vertex_shader_from_path(self, shader_path, error);
}

void
psy_shader_program_set_fragment_shader(PsyShaderProgram *self,
                                       PsyShader        *shader,
                                       GError          **error)
{
    g_return_if_fail(PSY_IS_SHADER_PROGRAM(self));
    g_return_if_fail(PSY_IS_SHADER(shader));
    g_return_if_fail(error != NULL && *error == NULL);

    PsyShaderProgramClass *klass = PSY_SHADER_PROGRAM_GET_CLASS(self);

    g_return_if_fail(klass->set_fragment_shader != NULL);

    klass->set_fragment_shader(self, shader, error);
}

/**
 * psy_shader_program_get_fragment_shader:
 * @self: An instance of [class@ShaderProgram]
 *
 * Get the fragment shader of this program if it has been set.
 *
 * Returns:(transfer none)(nullable): The instance of [class@Shader] that
 *         that represents the fragment shader of this class.
 */
PsyShader *
psy_shader_program_get_fragment_shader(PsyShaderProgram *self)
{
    g_return_val_if_fail(PSY_IS_SHADER_PROGRAM(self), NULL);

    PsyShaderProgramPrivate *priv
        = psy_shader_program_get_instance_private(self);

    return priv->fragment_shader;
}

void
psy_shader_program_set_fragment_shader_source(PsyShaderProgram *self,
                                              const gchar      *source,
                                              GError          **error)
{
    g_return_if_fail(PSY_IS_SHADER_PROGRAM(self));
    g_return_if_fail(source != NULL);
    g_return_if_fail(error != NULL && *error == NULL);

    PsyShaderProgramClass *klass = PSY_SHADER_PROGRAM_GET_CLASS(self);

    g_return_if_fail(klass->set_fragment_shader_source != NULL);

    klass->set_fragment_shader_source(self, source, error);
}

void
psy_shader_program_set_fragment_shader_from_file(PsyShaderProgram *self,
                                                 GFile            *shader_file,
                                                 GError          **error)
{
    g_return_if_fail(PSY_IS_SHADER_PROGRAM(self));
    g_return_if_fail(G_IS_FILE(shader_file));
    g_return_if_fail(error != NULL && *error == NULL);

    PsyShaderProgramClass *klass = PSY_SHADER_PROGRAM_GET_CLASS(self);

    g_return_if_fail(klass->set_fragment_shader_from_file != NULL);

    klass->set_fragment_shader_from_file(self, shader_file, error);
}

void
psy_shader_program_set_fragment_shader_from_path(PsyShaderProgram *self,
                                                 const gchar      *shader_path,
                                                 GError          **error)
{
    g_return_if_fail(PSY_IS_SHADER_PROGRAM(self));
    g_return_if_fail(shader_path != NULL);
    g_return_if_fail(error != NULL && *error == NULL);

    PsyShaderProgramClass *klass = PSY_SHADER_PROGRAM_GET_CLASS(self);

    g_return_if_fail(klass->set_fragment_shader_from_path != NULL);

    klass->set_fragment_shader_from_path(self, shader_path, error);
}

void
psy_shader_program_link(PsyShaderProgram *self, GError **error)
{
    g_return_if_fail(PSY_IS_SHADER_PROGRAM(self));
    g_return_if_fail(error == NULL || *error == NULL);

    PsyShaderProgramClass *klass = PSY_SHADER_PROGRAM_GET_CLASS(self);

    g_return_if_fail(klass->link != NULL);

    klass->link(self, error);
}

gboolean
psy_shader_program_is_linked(PsyShaderProgram *self)
{
    g_return_val_if_fail(PSY_IS_SHADER_PROGRAM(self), FALSE);

    PsyShaderProgramClass *klass = PSY_SHADER_PROGRAM_GET_CLASS(self);

    g_return_val_if_fail(klass->is_linked != NULL, FALSE);

    return klass->is_linked(self);
}

void
psy_shader_program_use(PsyShaderProgram *self, GError **error)
{
    g_return_if_fail(PSY_IS_SHADER_PROGRAM(self));
    g_return_if_fail(error == NULL || *error == NULL);

    PsyShaderProgramClass *klass = PSY_SHADER_PROGRAM_GET_CLASS(self);

    g_return_if_fail(klass->use_program != NULL);

    klass->use_program(self, error);
}

void
psy_shader_program_set_uniform_matrix4(PsyShaderProgram *self,
                                       const gchar      *name,
                                       PsyMatrix4       *matrix,
                                       GError          **error)
{
    g_return_if_fail(PSY_IS_SHADER_PROGRAM(self));
    g_return_if_fail(name);
    g_return_if_fail(PSY_IS_MATRIX4(matrix));
    g_return_if_fail(error == NULL || *error == NULL);

    PsyShaderProgramClass *class = PSY_SHADER_PROGRAM_GET_CLASS(self);

    g_return_if_fail(class->set_uniform_matrix4);

    class->set_uniform_matrix4(self, name, matrix, error);
}

/**
 * psy_shader_program_set_uniform_4f:
 * @self: an instance of `PsyShaderProgram`
 * @name: the name in order to refer to proper name of the uniform
 * @values:(array fixed-size=4): the 4 value to send to the uniform
 * @error:An error might be returned here.
 *
 * Set a uniform property with 4 floats in the shader
 */
void
psy_shader_program_set_uniform_4f(PsyShaderProgram *self,
                                  const gchar      *name,
                                  gfloat           *values,
                                  GError          **error)
{
    g_return_if_fail(PSY_IS_SHADER_PROGRAM(self));
    g_return_if_fail(name);
    g_return_if_fail(error == NULL || *error == NULL);

    PsyShaderProgramClass *class = PSY_SHADER_PROGRAM_GET_CLASS(self);

    g_return_if_fail(class->set_uniform_4f);
    class->set_uniform_4f(self, name, values, error);
}
