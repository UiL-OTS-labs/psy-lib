

#include "psy-program.h"
#include "psy-matrix4.h"
#include "psy-shader.h"

typedef struct _PsyProgramPrivate {
    PsyShader *vertex_shader;
    PsyShader *fragment_shader;
} PsyProgramPrivate;

G_DEFINE_TYPE_WITH_PRIVATE(PsyProgram, psy_program, G_TYPE_OBJECT)

typedef enum {
    PROP_NULL,
    NUM_PROPERTIES
} PsyProgramProperty;

/*
 * static GParamSpec* program_properties[NUM_PROPERTIES];
 */

static void
psy_program_set_property(GObject        *object,
                         guint           prop_id,
                         const GValue   *value,
                         GParamSpec     *pspec)
{
    PsyProgram* self = PSY_PROGRAM(object);
    (void) self;
    (void) value;

    switch((PsyProgramProperty) prop_id) {
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

static void
psy_program_get_property(GObject    *object,
                         guint       prop_id,
                         GValue     *value,
                         GParamSpec *pspec)
{
    PsyProgram* self = PSY_PROGRAM(object);
    PsyProgramPrivate* priv = psy_program_get_instance_private(self);
    (void) value;
    (void) priv;

    switch((PsyProgramProperty) prop_id) {
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

static void
psy_program_init(PsyProgram *self)
{
    PsyProgramPrivate* priv = psy_program_get_instance_private(self);
    (void) priv;
}

static void
psy_program_dispose(GObject* object)
{
    PsyProgram* self = PSY_PROGRAM(object);
    PsyProgramPrivate* priv = psy_program_get_instance_private(self);
    (void) priv;

    g_clear_object(&priv->vertex_shader);
    g_clear_object(&priv->fragment_shader);

    G_OBJECT_CLASS(psy_program_parent_class)->dispose(object);
}

static void
psy_program_finalize(GObject* object)
{
    PsyProgram* self = PSY_PROGRAM(object);
    PsyProgramPrivate* priv = psy_program_get_instance_private(self);
    (void) priv;

    G_OBJECT_CLASS(psy_program_parent_class)->finalize(object);
}


static void
psy_program_class_init(PsyProgramClass* class)
{
    GObjectClass   *gobject_class = G_OBJECT_CLASS(class);

    gobject_class->set_property = psy_program_set_property;
    gobject_class->get_property = psy_program_get_property;
    gobject_class->finalize     = psy_program_finalize;
    gobject_class->dispose      = psy_program_dispose;
}

/* ************ public functions ******************** */

void
psy_program_set_vertex_shader(PsyProgram       *self,
                              PsyShader        *shader,
                              GError          **error)
{
    g_return_if_fail(PSY_IS_PROGRAM(self));
    g_return_if_fail(PSY_IS_SHADER(shader));
    g_return_if_fail(error != NULL && *error == NULL);
    
    PsyProgramClass *klass = PSY_PROGRAM_GET_CLASS(self);

    g_return_if_fail(klass->set_vertex_shader != NULL);

    klass->set_vertex_shader(self, shader, error);
}

void
psy_program_set_vertex_source (PsyProgram* self, const gchar* source, GError** error)
{
    g_return_if_fail(PSY_IS_PROGRAM(self));
    g_return_if_fail(source != NULL);
    g_return_if_fail(error != NULL && *error == NULL);

    PsyProgramClass *klass = PSY_PROGRAM_GET_CLASS(self);

    g_return_if_fail(klass->set_vertex_shader_source != NULL);

    klass->set_vertex_shader_source(self, source, error);
}

void
psy_program_set_vertex_shader_from_file(PsyProgram    *self,
                                        GFile         *shader_file,
                                        GError       **error)
{
    g_return_if_fail(PSY_IS_PROGRAM(self));
    g_return_if_fail(G_IS_FILE(shader_file));
    g_return_if_fail(error != NULL && *error == NULL);

    PsyProgramClass *klass = PSY_PROGRAM_GET_CLASS(self);

    g_return_if_fail(klass->set_vertex_shader_from_file != NULL);

    klass->set_vertex_shader_from_file(self, shader_file, error);
}


void
psy_program_set_vertex_shader_from_path(PsyProgram    *self,
                                        const gchar   *shader_path,
                                        GError       **error)
{
    g_return_if_fail(PSY_IS_PROGRAM(self));
    g_return_if_fail(shader_path != NULL);
    g_return_if_fail(error != NULL && *error == NULL);

    PsyProgramClass *klass = PSY_PROGRAM_GET_CLASS(self);

    g_return_if_fail(klass->set_vertex_shader_from_path!= NULL);

    klass->set_vertex_shader_from_path(self, shader_path, error);
}

void
psy_program_set_fragment_shader(PsyProgram       *self,
                                PsyShader        *shader,
                                GError          **error)
{
    g_return_if_fail(PSY_IS_PROGRAM(self));
    g_return_if_fail(PSY_IS_SHADER(shader));
    g_return_if_fail(error != NULL && *error == NULL);
    
    PsyProgramClass *klass = PSY_PROGRAM_GET_CLASS(self);
    
    g_return_if_fail(klass->set_fragment_shader != NULL);

    klass->set_fragment_shader(self, shader, error);
}

void
psy_program_set_fragment_shader_source (PsyProgram  *self,
                                        const gchar *source,
                                        GError     **error)
{
    g_return_if_fail(PSY_IS_PROGRAM(self));
    g_return_if_fail(source != NULL);
    g_return_if_fail(error != NULL && *error == NULL);

    PsyProgramClass *klass = PSY_PROGRAM_GET_CLASS(self);

    g_return_if_fail(klass->set_fragment_shader_source != NULL);

    klass->set_fragment_shader_source(self, source, error);
}

void
psy_program_set_fragment_shader_from_file(PsyProgram    *self,
                                          GFile         *shader_file,
                                          GError       **error)
{
    g_return_if_fail(PSY_IS_PROGRAM(self));
    g_return_if_fail(G_IS_FILE(shader_file));
    g_return_if_fail(error != NULL && *error == NULL);

    PsyProgramClass *klass = PSY_PROGRAM_GET_CLASS(self);

    g_return_if_fail(klass->set_fragment_shader_from_file != NULL);

    klass->set_fragment_shader_from_file(self, shader_file, error);
}


void
psy_program_set_fragment_shader_from_path(PsyProgram    *self,
                                          const gchar   *shader_path,
                                          GError       **error)
{
    g_return_if_fail(PSY_IS_PROGRAM(self));
    g_return_if_fail(shader_path != NULL);
    g_return_if_fail(error != NULL && *error == NULL);

    PsyProgramClass *klass = PSY_PROGRAM_GET_CLASS(self);

    g_return_if_fail(klass->set_fragment_shader_from_path!= NULL);

    klass->set_fragment_shader_from_path(self, shader_path, error);
}

void
psy_program_link(PsyProgram *self, GError **error)
{
    g_return_if_fail(PSY_IS_PROGRAM(self));
    g_return_if_fail(error == NULL || *error == NULL);

    PsyProgramClass *klass = PSY_PROGRAM_GET_CLASS(self);

    g_return_if_fail(klass->link != NULL);

    klass->link(self, error);
}

gboolean
psy_program_is_linked(PsyProgram *self)
{
    g_return_val_if_fail(PSY_IS_PROGRAM(self), FALSE);

    PsyProgramClass *klass = PSY_PROGRAM_GET_CLASS(self);

    g_return_val_if_fail(klass->is_linked != NULL, FALSE);

    return klass->is_linked(self);
}

void
psy_program_use(PsyProgram* self, GError **error)
{
    g_return_if_fail(PSY_IS_PROGRAM(self));
    g_return_if_fail(error == NULL || *error == NULL);

    PsyProgramClass *klass = PSY_PROGRAM_GET_CLASS(self);

    g_return_if_fail(klass->use_program != NULL);

    klass->use_program(self, error);
}

void
psy_program_set_uniform_matrix4(
        PsyProgram     *self,
        const gchar    *name,
        PsyMatrix4     *matrix,
        GError        **error
        )
{
    g_return_if_fail(PSY_IS_PROGRAM(self));
    g_return_if_fail(name);
    g_return_if_fail(PSY_IS_MATRIX4(matrix));
    g_return_if_fail(error == NULL || *error == NULL);

    PsyProgramClass* class = PSY_PROGRAM_GET_CLASS(self);

    g_return_if_fail(class->set_uniform_matrix4);

    class->set_uniform_matrix4(self, name, matrix, error);
}

/**
 * psy_program_set_uniform_4f:
 * @self: an instance of `PsyProgram`
 * @name: the name in order to refer to proper name of the uniform
 * @values:(array fixed-size=4): the 4 value to send to the uniform
 * @error:An error might be returned here.
 *
 * Set a uniform property with 4 floats in the shader
 */
void
psy_program_set_uniform_4f(
        PsyProgram     *self,
        const gchar    *name,
        gfloat         *values,
        GError        **error
        )
{
    g_return_if_fail(PSY_IS_PROGRAM(self));
    g_return_if_fail(name);
    g_return_if_fail(error == NULL || *error == NULL);

    PsyProgramClass* class = PSY_PROGRAM_GET_CLASS(self);

    g_return_if_fail(class->set_uniform_4f);
    class->set_uniform_4f(self, name, values, error);
}

