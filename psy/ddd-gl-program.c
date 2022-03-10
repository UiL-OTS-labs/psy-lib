

#include "ddd-gl-program.h"
#include "ddd-gl-error.h"
#include "ddd-shader.h"
#include "ddd-gl-shader.h"
#include "ddd-gl-vertex-shader.h"
#include "ddd-gl-fragment-shader.h"

#include <epoxy/gl.h>

typedef struct _DddGlProgram {
    DddProgram           parent;
    GLuint               object_id;
    DddGlVertexShader   *vertex_shader;
    DddGlFragmentShader *fragment_shader;
    guint                is_linked : 1;
} DddGlProgram;

G_DEFINE_TYPE(DddGlProgram, ddd_gl_program, DDD_TYPE_PROGRAM)

typedef enum {
    PROP_NULL,
    PROP_OBJECT_ID,
    PROP_IS_LINKED,
    NUM_PROPERTIES    
} DddGlProgramProperty;

static GParamSpec* gl_program_properties[NUM_PROPERTIES];

static void
ddd_gl_program_set_property(GObject        *object,
                           guint           prop_id,
                           const GValue   *value,
                           GParamSpec     *pspec)
{
    DddGlProgram* self = DDD_GL_PROGRAM(object);
    (void) self, (void) value;

    switch((DddGlProgramProperty) prop_id) {
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

static void
ddd_gl_program_get_property(GObject    *object,
                           guint       prop_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
    DddGlProgram* self = DDD_GL_PROGRAM(object);

    switch((DddGlProgramProperty) prop_id) {
        case PROP_OBJECT_ID:
            g_value_set_uint(value, self->object_id);
            break;
        case PROP_IS_LINKED:
            g_value_set_boolean(value, self->is_linked);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

static void
ddd_gl_program_init(DddGlProgram *self)
{
    self->object_id = 0;
    self->is_linked = 0;
}

static void
ddd_gl_program_dispose(GObject* object)
{
    DddGlProgram* self = DDD_GL_PROGRAM(object);
    (void) self;

    G_OBJECT_CLASS(ddd_gl_program_parent_class)->dispose(object);
}

static void
ddd_gl_program_finalize(GObject* object)
{
    DddGlProgram* self = DDD_GL_PROGRAM(object);
    //DddGlProgramPrivate* priv = ddd_gl_program_get_instance_private(self);

    if (self->object_id) {
        glDeleteProgram(self->object_id);
    }

    G_OBJECT_CLASS(ddd_gl_program_parent_class)->finalize(object);
}

static void
ddd_gl_program_set_vertex_shader(
        DddProgram *program,
        DddShader  *shader,
        GError    **error)
{
    g_return_if_fail(DDD_IS_GL_PROGRAM(program));
    g_return_if_fail(DDD_IS_GL_SHADER(shader));

    DddGlProgram* self = DDD_GL_PROGRAM(program);

    if (!ddd_shader_is_compiled(shader)) {
        ddd_shader_compile(shader, error);
        if (*error)
            return;
    }

    if (self->vertex_shader)
        g_object_unref(self->vertex_shader);

    self->vertex_shader = DDD_GL_VERTEX_SHADER(shader);
}

static void
ddd_gl_program_set_vertex_shader_source(
        DddProgram  *program,
        const gchar *shader_src,
        GError     **error)
{
    g_return_if_fail(DDD_IS_GL_PROGRAM(program));

    DddProgramClass *klass = DDD_PROGRAM_GET_CLASS(program);
    
    DddGlVertexShader *shader = ddd_gl_vertex_shader_new();
    ddd_shader_set_source(DDD_SHADER(shader), shader_src);

    klass->set_vertex_shader(program, DDD_SHADER(shader), error);
}

static void
ddd_gl_program_set_vertex_shader_from_file(
        DddProgram  *program,
        GFile       *shader_file,
        GError     **error)
{
    g_return_if_fail(DDD_IS_GL_PROGRAM(program));

    DddProgramClass* klass = DDD_PROGRAM_GET_CLASS(program);

    DddGlVertexShader *shader = ddd_gl_vertex_shader_new();
    ddd_shader_source_from_file(DDD_SHADER(shader), shader_file, error);
    if(error != NULL && *error != NULL)
        return;

    klass->set_vertex_shader(program, DDD_SHADER(shader), error);
}

static void
ddd_gl_program_set_vertex_shader_from_path (
        DddProgram  *program,
        const gchar *shader_path,
        GError     **error
        )
{
    g_return_if_fail(DDD_IS_GL_PROGRAM(program));

    DddProgramClass* klass = DDD_PROGRAM_GET_CLASS(program);

    DddGlVertexShader   *shader = ddd_gl_vertex_shader_new();
    ddd_shader_source_from_path(DDD_SHADER(shader), shader_path, error);
    if(error != NULL && *error != NULL)
        return;

    klass->set_vertex_shader(program, DDD_SHADER(shader), error);
}

static void
ddd_gl_program_set_fragment_shader(
        DddProgram *program,
        DddShader  *shader,
        GError    **error)
{
    g_return_if_fail(DDD_IS_GL_PROGRAM(program));
    g_return_if_fail(DDD_IS_GL_SHADER(shader));

    DddGlProgram* self = DDD_GL_PROGRAM(program);

    if (!ddd_shader_is_compiled(shader)) {
        ddd_shader_compile(shader, error);
        if (*error)
            return;
    }

    if (self->fragment_shader)
        g_object_unref(self->fragment_shader);

    self->fragment_shader = DDD_GL_FRAGMENT_SHADER(shader);
}

static void
ddd_gl_program_set_fragment_shader_source(
        DddProgram  *program,
        const gchar *shader_src,
        GError     **error)
{
    g_return_if_fail(DDD_IS_GL_PROGRAM(program));

    DddProgramClass* klass = DDD_PROGRAM_GET_CLASS(program);

    DddGlFragmentShader   *shader = ddd_gl_fragment_shader_new();
    ddd_shader_set_source(DDD_SHADER(shader), shader_src);

    klass->set_fragment_shader(program, DDD_SHADER(shader), error);
}

static void
ddd_gl_program_set_fragment_shader_from_file(
        DddProgram  *program,
        GFile       *shader_file,
        GError     **error)
{
    g_return_if_fail(DDD_IS_GL_PROGRAM(program));

    DddProgramClass* klass = DDD_PROGRAM_GET_CLASS(program);

    DddGlFragmentShader *shader = ddd_gl_fragment_shader_new();
    ddd_shader_source_from_file(DDD_SHADER(shader), shader_file, error);
    if(error != NULL && *error != NULL)
        return;

    klass->set_fragment_shader(program, DDD_SHADER(shader), error);
}

static void
ddd_gl_program_set_fragment_shader_from_path (
        DddProgram  *program,
        const gchar *shader_path,
        GError     **error
)
{
    g_return_if_fail(DDD_IS_GL_PROGRAM(program));

    DddProgramClass* klass = DDD_PROGRAM_GET_CLASS(program);

    DddGlFragmentShader   *shader = ddd_gl_fragment_shader_new();
    ddd_shader_source_from_path(DDD_SHADER(shader), shader_path, error);
    if(error != NULL && *error != NULL)
        return;

    klass->set_fragment_shader(program, DDD_SHADER(shader), error);
}


static void
ddd_gl_program_link(DddProgram* program, GError **error)
{
    gint link_succes;
    DddGlProgram* self  = DDD_GL_PROGRAM(program);
    if (!self->object_id)
        self->object_id = glCreateProgram();

    guint prog_id       = self->object_id;
    guint vertex_id     = 0;
    guint fragment_id   = 0;

    if (ddd_program_is_linked(DDD_PROGRAM(self)))
        return;

    if (!ddd_shader_is_compiled(DDD_SHADER(self->vertex_shader))) {
        ddd_shader_compile(DDD_SHADER(self->vertex_shader), error);
        if (*error != NULL)
            return;
    }
    vertex_id = ddd_gl_shader_get_object_id(DDD_GL_SHADER(self->vertex_shader));

    if (!ddd_shader_is_compiled(DDD_SHADER(self->fragment_shader))) {
        ddd_shader_compile(DDD_SHADER(self->vertex_shader), error);
        if (*error != NULL)
            return;
    }
    fragment_id = ddd_gl_shader_get_object_id(DDD_GL_SHADER(self->fragment_shader));

    glAttachShader(prog_id, vertex_id);
    glAttachShader(prog_id, fragment_id);
    glLinkProgram(prog_id);

    glGetProgramiv(prog_id, GL_LINK_STATUS, &link_succes);

    if (!link_succes) {
        GString* log = NULL;
        gint logsize;
        glGetProgramiv(prog_id, GL_INFO_LOG_LENGTH, &logsize);
        log = g_string_new_len("", logsize);
        glGetProgramInfoLog(prog_id, logsize, &logsize, log->str);
        g_set_error(error,
                    DDD_GL_ERROR,
                    DDD_GL_ERROR_PROGRAM_LINK,
                    "Unable to link program:\n%s", log->str);
        g_string_free(log, TRUE);
        return;
    }
    self->is_linked = true;
}

static gboolean
ddd_gl_program_is_linked(DddProgram* self)
{
    DddGlProgram* gl_program = DDD_GL_PROGRAM(self);
    return gl_program->is_linked;
}

static void
ddd_gl_program_use_program(DddProgram* self, GError **error)
{
    if (!ddd_gl_program_is_linked(self)) {
        ddd_gl_program_link(self, error);
        if (*error)
            return;
    }
    DddGlProgram *gl_program = DDD_GL_PROGRAM(self);
    glUseProgram(gl_program->object_id);
    ddd_gl_check_error(error);
}

static void
ddd_gl_program_class_init(DddGlProgramClass* class)
{
    GObjectClass       *gobject_class = G_OBJECT_CLASS(class);
    DddProgramClass    *program_class = DDD_PROGRAM_CLASS(class);

    gobject_class->set_property = ddd_gl_program_set_property;
    gobject_class->get_property = ddd_gl_program_get_property;
    gobject_class->finalize     = ddd_gl_program_finalize;
    gobject_class->dispose      = ddd_gl_program_dispose;

    program_class->set_vertex_shader =
        ddd_gl_program_set_vertex_shader;
    program_class->set_vertex_shader_source =
        ddd_gl_program_set_vertex_shader_source;
    program_class->set_vertex_shader_from_file = 
        ddd_gl_program_set_vertex_shader_from_file;
    program_class->set_vertex_shader_from_path = 
        ddd_gl_program_set_vertex_shader_from_path;

    program_class->set_fragment_shader =
        ddd_gl_program_set_fragment_shader;
    program_class->set_fragment_shader_source =
        ddd_gl_program_set_fragment_shader_source;
    program_class->set_fragment_shader_from_file = 
        ddd_gl_program_set_fragment_shader_from_file;
    program_class->set_fragment_shader_from_path = 
        ddd_gl_program_set_fragment_shader_from_path;

    program_class->link         = ddd_gl_program_link;
    program_class->is_linked    = ddd_gl_program_is_linked;
    program_class->use_program  = ddd_gl_program_use_program;

    gl_program_properties[PROP_OBJECT_ID] = g_param_spec_string(
            "object-id",
            "Object ID",
            "The OpenGL id of the object",
            0,
            G_PARAM_READWRITE
            );

    gl_program_properties[PROP_IS_LINKED] = g_param_spec_boolean(
            "is-linked",
            "Is linked",
            "Whether the shader program is succesfully linked.",
            FALSE,
            G_PARAM_READABLE
            );

    g_object_class_install_properties(gobject_class,
                                      NUM_PROPERTIES,
                                      gl_program_properties);
}

/* ************ public functions ******************** */

DddGlProgram*
ddd_gl_program_new()
{
    DddGlProgram *gl_program = g_object_new(DDD_TYPE_GL_PROGRAM, NULL);
    return gl_program;
}

guint
ddd_gl_program_get_object_id(DddGlProgram* self) {
    g_return_val_if_fail(DDD_IS_GL_PROGRAM(self), 0);
    
    //DddGlProgramPrivate* priv = ddd_gl_program_get_instance_private(self);
    
    return self->object_id;
}

