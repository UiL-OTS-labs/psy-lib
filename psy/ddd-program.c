

#include "ddd-program.h"
#include "ddd-shader.h"

typedef struct _DddProgramPrivate {
    DddShader *vertex_shader;
    DddShader *fragment_shader;
} DddProgramPrivate;

G_DEFINE_TYPE_WITH_PRIVATE(DddProgram, ddd_program, G_TYPE_OBJECT)

typedef enum {
    PROP_NULL,
    NUM_PROPERTIES
} DddProgramProperty;

/*
 * static GParamSpec* program_properties[NUM_PROPERTIES];
 */

static void
ddd_program_set_property(GObject        *object,
                         guint           prop_id,
                         const GValue   *value,
                         GParamSpec     *pspec)
{
    DddProgram* self = DDD_PROGRAM(object);
    (void) self;
    (void) value;

    switch((DddProgramProperty) prop_id) {
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

static void
ddd_program_get_property(GObject    *object,
                         guint       prop_id,
                         GValue     *value,
                         GParamSpec *pspec)
{
    DddProgram* self = DDD_PROGRAM(object);
    DddProgramPrivate* priv = ddd_program_get_instance_private(self);
    (void) value;
    (void) priv;

    switch((DddProgramProperty) prop_id) {
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

static void
ddd_program_init(DddProgram *self)
{
    DddProgramPrivate* priv = ddd_program_get_instance_private(self);
    (void) priv;
}

static void
ddd_program_dispose(GObject* object)
{
    DddProgram* self = DDD_PROGRAM(object);
    DddProgramPrivate* priv = ddd_program_get_instance_private(self);
    (void) priv;

    G_OBJECT_CLASS(ddd_program_parent_class)->dispose(object);
}

static void
ddd_program_finalize(GObject* object)
{
    DddProgram* self = DDD_PROGRAM(object);
    DddProgramPrivate* priv = ddd_program_get_instance_private(self);
    (void) priv;

    G_OBJECT_CLASS(ddd_program_parent_class)->dispose(object);
}


static void
ddd_program_class_init(DddProgramClass* class)
{
    GObjectClass   *gobject_class = G_OBJECT_CLASS(class);

    gobject_class->set_property = ddd_program_set_property;
    gobject_class->get_property = ddd_program_get_property;
    gobject_class->finalize     = ddd_program_finalize;
    gobject_class->dispose      = ddd_program_dispose;
}

/* ************ public functions ******************** */

void
ddd_program_set_vertex_shader(DddProgram       *self,
                              DddShader        *shader,
                              GError          **error)
{
    g_return_if_fail(DDD_IS_PROGRAM(self));
    g_return_if_fail(DDD_IS_SHADER(shader));
    g_return_if_fail(error != NULL && *error == NULL);
    
    DddProgramClass *klass = DDD_PROGRAM_GET_CLASS(self);

    g_return_if_fail(klass->set_vertex_shader != NULL);

    klass->set_vertex_shader(self, shader, error);
}

void
ddd_program_set_vertex_source (DddProgram* self, const gchar* source, GError** error)
{
    g_return_if_fail(DDD_IS_PROGRAM(self));
    g_return_if_fail(source != NULL);
    g_return_if_fail(error != NULL && *error == NULL);

    DddProgramClass *klass = DDD_PROGRAM_GET_CLASS(self);

    g_return_if_fail(klass->set_vertex_shader_source != NULL);

    klass->set_vertex_shader_source(self, source, error);
}

void
ddd_program_set_vertex_shader_from_file(DddProgram    *self,
                                        GFile         *shader_file,
                                        GError       **error)
{
    g_return_if_fail(DDD_IS_PROGRAM(self));
    g_return_if_fail(G_IS_FILE(shader_file));
    g_return_if_fail(error != NULL && *error == NULL);

    DddProgramClass *klass = DDD_PROGRAM_GET_CLASS(self);

    g_return_if_fail(klass->set_vertex_shader_from_file != NULL);

    klass->set_vertex_shader_from_file(self, shader_file, error);
}


void
ddd_program_set_vertex_shader_from_path(DddProgram    *self,
                                        const gchar   *shader_path,
                                        GError       **error)
{
    g_return_if_fail(DDD_IS_PROGRAM(self));
    g_return_if_fail(shader_path != NULL);
    g_return_if_fail(error != NULL && *error == NULL);

    DddProgramClass *klass = DDD_PROGRAM_GET_CLASS(self);

    g_return_if_fail(klass->set_vertex_shader_from_path!= NULL);

    klass->set_vertex_shader_from_path(self, shader_path, error);
}

void
ddd_program_set_fragment_shader(DddProgram       *self,
                                DddShader        *shader,
                                GError          **error)
{
    g_return_if_fail(DDD_IS_PROGRAM(self));
    g_return_if_fail(DDD_IS_SHADER(shader));
    g_return_if_fail(error != NULL && *error == NULL);
    
    DddProgramClass *klass = DDD_PROGRAM_GET_CLASS(self);
    
    g_return_if_fail(klass->set_fragment_shader != NULL);

    klass->set_fragment_shader(self, shader, error);
}

void
ddd_program_set_fragment_shader_source (DddProgram  *self,
                                        const gchar *source,
                                        GError     **error)
{
    g_return_if_fail(DDD_IS_PROGRAM(self));
    g_return_if_fail(source != NULL);
    g_return_if_fail(error != NULL && *error == NULL);

    DddProgramClass *klass = DDD_PROGRAM_GET_CLASS(self);

    g_return_if_fail(klass->set_fragment_shader_source != NULL);

    klass->set_fragment_shader_source(self, source, error);
}

void
ddd_program_set_fragment_shader_from_file(DddProgram    *self,
                                          GFile         *shader_file,
                                          GError       **error)
{
    g_return_if_fail(DDD_IS_PROGRAM(self));
    g_return_if_fail(G_IS_FILE(shader_file));
    g_return_if_fail(error != NULL && *error == NULL);

    DddProgramClass *klass = DDD_PROGRAM_GET_CLASS(self);

    g_return_if_fail(klass->set_fragment_shader_from_file != NULL);

    klass->set_fragment_shader_from_file(self, shader_file, error);
}


void
ddd_program_set_fragment_shader_from_path(DddProgram    *self,
                                          const gchar   *shader_path,
                                          GError       **error)
{
    g_return_if_fail(DDD_IS_PROGRAM(self));
    g_return_if_fail(shader_path != NULL);
    g_return_if_fail(error != NULL && *error == NULL);

    DddProgramClass *klass = DDD_PROGRAM_GET_CLASS(self);

    g_return_if_fail(klass->set_fragment_shader_from_path!= NULL);

    klass->set_fragment_shader_from_path(self, shader_path, error);
}

void
ddd_program_link(DddProgram *self, GError **error)
{
    g_return_if_fail(DDD_IS_PROGRAM(self));
    g_return_if_fail(error == NULL || *error == NULL);

    DddProgramClass *klass = DDD_PROGRAM_GET_CLASS(self);

    g_return_if_fail(klass->link != NULL);

    klass->link(self, error);
}

gboolean
ddd_program_is_linked(DddProgram *self)
{
    g_return_val_if_fail(DDD_IS_PROGRAM(self), FALSE);

    DddProgramClass *klass = DDD_PROGRAM_GET_CLASS(self);

    g_return_val_if_fail(klass->is_linked != NULL, FALSE);

    return klass->is_linked(self);
}

void
ddd_program_use(DddProgram* self, GError **error)
{
    g_return_if_fail(DDD_IS_PROGRAM(self));
    g_return_if_fail(error == NULL || *error == NULL);

    DddProgramClass *klass = DDD_PROGRAM_GET_CLASS(self);

    g_return_if_fail(klass->use_program != NULL);

    klass->use_program(self, error);
}

