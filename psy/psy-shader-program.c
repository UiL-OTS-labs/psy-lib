

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
psy_shader_program_set_property(GObject      *object,
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
psy_shader_program_get_property(GObject    *object,
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
psy_shader_program_dispose(GObject *object)
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
psy_shader_program_finalize(GObject *object)
{
    PsyShaderProgram        *self = PSY_SHADER_PROGRAM(object);
    PsyShaderProgramPrivate *priv
        = psy_shader_program_get_instance_private(self);
    (void) priv;

    G_OBJECT_CLASS(psy_shader_program_parent_class)->finalize(object);
}

static void
psy_shader_program_class_init(PsyShaderProgramClass *class)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(class);

    gobject_class->set_property = psy_shader_program_set_property;
    gobject_class->get_property = psy_shader_program_get_property;
    gobject_class->finalize     = psy_shader_program_finalize;
    gobject_class->dispose      = psy_shader_program_dispose;
}

/* ************ public functions ******************** */

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

void
psy_shader_program_set_vertex_source(PsyShaderProgram *self,
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
