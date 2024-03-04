

#include "psy-gl-program.h"
#include "psy-gl-error.h"
#include "psy-gl-fragment-shader.h"
#include "psy-gl-shader.h"
#include "psy-gl-vertex-shader.h"
#include "psy-shader.h"

#include <epoxy/gl.h>
#include <epoxy/gl_generated.h>

typedef struct _PsyGlProgram {
    PsyShaderProgram parent;
    GLuint           object_id;
    guint            is_linked : 1;
} PsyGlProgram;

G_DEFINE_TYPE(PsyGlProgram, psy_gl_program, PSY_TYPE_SHADER_PROGRAM)

typedef enum {
    PROP_NULL,
    PROP_OBJECT_ID,
    PROP_IS_LINKED,
    NUM_PROPERTIES
} PsyGlProgramProperty;

static GParamSpec *gl_program_properties[NUM_PROPERTIES];

static void
psy_gl_program_get_property(GObject    *object,
                            guint       prop_id,
                            GValue     *value,
                            GParamSpec *pspec)
{
    PsyGlProgram *self = PSY_GL_PROGRAM(object);

    switch ((PsyGlProgramProperty) prop_id) {
    case PROP_OBJECT_ID:
        g_value_set_uint(value, psy_gl_program_get_object_id(self));
        break;
    case PROP_IS_LINKED:
        g_value_set_boolean(value, self->is_linked);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

static void
psy_gl_program_init(PsyGlProgram *self)
{
    self->object_id = 0;
    self->is_linked = 0;
}

static void
psy_gl_program_finalize(GObject *object)
{
    PsyGlProgram *self = PSY_GL_PROGRAM(object);
    // PsyGlProgramPrivate* priv = psy_gl_program_get_instance_private(self);

    if (self->object_id) {
        glDeleteProgram(self->object_id);
        self->object_id = 0;
        self->is_linked = 0;
    }

    G_OBJECT_CLASS(psy_gl_program_parent_class)->finalize(object);
}

static void
psy_gl_program_set_vertex_shader_source(PsyShaderProgram *program,
                                        const gchar      *shader_src,
                                        GError          **error)
{
    g_return_if_fail(PSY_IS_GL_PROGRAM(program));

    PsyShaderProgramClass *klass = PSY_SHADER_PROGRAM_GET_CLASS(program);

    PsyGlVertexShader *shader = psy_gl_vertex_shader_new();
    psy_shader_set_source(PSY_SHADER(shader), shader_src);

    klass->set_vertex_shader(program, PSY_SHADER(shader), error);
}

static void
psy_gl_program_set_vertex_shader_from_file(PsyShaderProgram *program,
                                           GFile            *shader_file,
                                           GError          **error)
{
    g_return_if_fail(PSY_IS_GL_PROGRAM(program));

    PsyShaderProgramClass *klass = PSY_SHADER_PROGRAM_GET_CLASS(program);

    PsyGlVertexShader *shader = psy_gl_vertex_shader_new();
    psy_shader_source_from_file(PSY_SHADER(shader), shader_file, error);
    if (error != NULL && *error != NULL)
        return;

    klass->set_vertex_shader(program, PSY_SHADER(shader), error);
}

static void
psy_gl_program_set_vertex_shader_from_path(PsyShaderProgram *program,
                                           const gchar      *shader_path,
                                           GError          **error)
{
    g_return_if_fail(PSY_IS_GL_PROGRAM(program));

    PsyShaderProgramClass *klass = PSY_SHADER_PROGRAM_GET_CLASS(program);

    PsyGlVertexShader *shader = psy_gl_vertex_shader_new();
    psy_shader_source_from_path(PSY_SHADER(shader), shader_path, error);
    if (error != NULL && *error != NULL)
        return;

    klass->set_vertex_shader(program, PSY_SHADER(shader), error);
}

static void
psy_gl_program_set_fragment_shader_source(PsyShaderProgram *program,
                                          const gchar      *shader_src,
                                          GError          **error)
{
    g_return_if_fail(PSY_IS_GL_PROGRAM(program));

    PsyShaderProgramClass *klass = PSY_SHADER_PROGRAM_GET_CLASS(program);

    PsyGlFragmentShader *shader = psy_gl_fragment_shader_new();
    psy_shader_set_source(PSY_SHADER(shader), shader_src);

    klass->set_fragment_shader(program, PSY_SHADER(shader), error);
}

static void
psy_gl_program_set_fragment_shader_from_file(PsyShaderProgram *program,
                                             GFile            *shader_file,
                                             GError          **error)
{
    g_return_if_fail(PSY_IS_GL_PROGRAM(program));

    PsyShaderProgramClass *klass = PSY_SHADER_PROGRAM_GET_CLASS(program);

    PsyGlFragmentShader *shader = psy_gl_fragment_shader_new();
    psy_shader_source_from_file(PSY_SHADER(shader), shader_file, error);
    if (error != NULL && *error != NULL)
        return;

    klass->set_fragment_shader(program, PSY_SHADER(shader), error);
}

static void
psy_gl_program_set_fragment_shader_from_path(PsyShaderProgram *program,
                                             const gchar      *shader_path,
                                             GError          **error)
{
    g_return_if_fail(PSY_IS_GL_PROGRAM(program));

    PsyShaderProgramClass *klass = PSY_SHADER_PROGRAM_GET_CLASS(program);

    PsyGlFragmentShader *shader = psy_gl_fragment_shader_new();
    psy_shader_source_from_path(PSY_SHADER(shader), shader_path, error);
    if (error != NULL && *error != NULL)
        return;

    klass->set_fragment_shader(program, PSY_SHADER(shader), error);
}

static void
psy_gl_program_link(PsyShaderProgram *program, GError **error)
{
    gint          link_succes;
    PsyGlProgram *self = PSY_GL_PROGRAM(program);
    if (!self->object_id)
        self->object_id = glCreateProgram();

    guint prog_id     = self->object_id;
    guint vertex_id   = 0;
    guint fragment_id = 0;

    if (psy_shader_program_is_linked(PSY_SHADER_PROGRAM(self)))
        return;

    PsyShader *vertex_shader = psy_shader_program_get_vertex_shader(program);
    if (!psy_shader_is_compiled(vertex_shader)) {
        psy_shader_compile(vertex_shader, error);
        if (*error != NULL)
            return;
    }
    vertex_id = psy_gl_shader_get_object_id(PSY_GL_SHADER(vertex_shader));

    PsyShader *frag_shader = psy_shader_program_get_fragment_shader(program);
    if (!psy_shader_is_compiled(frag_shader)) {
        psy_shader_compile(frag_shader, error);
        if (*error != NULL)
            return;
    }

    fragment_id = psy_gl_shader_get_object_id(PSY_GL_SHADER(frag_shader));

    glAttachShader(prog_id, vertex_id);
    glAttachShader(prog_id, fragment_id);
    glLinkProgram(prog_id);

    glGetProgramiv(prog_id, GL_LINK_STATUS, &link_succes);

    if (!link_succes) {
        gchar log_buffer[16384];
        gint  logsize;
        glGetProgramiv(prog_id, GL_INFO_LOG_LENGTH, &logsize);
        glGetProgramInfoLog(prog_id, sizeof(log_buffer), &logsize, log_buffer);
        g_set_error(error,
                    PSY_GL_ERROR,
                    PSY_GL_ERROR_PROGRAM_LINK,
                    "Unable to link program:\n%s",
                    log_buffer);
        return;
    }
    self->is_linked = true;
}

static gboolean
psy_gl_program_is_linked(PsyShaderProgram *self)
{
    PsyGlProgram *gl_program = PSY_GL_PROGRAM(self);
    return gl_program->is_linked;
}

static void
psy_gl_program_use_program(PsyShaderProgram *self, GError **error)
{
    if (!psy_gl_program_is_linked(self)) {
        psy_gl_program_link(self, error);
        if (*error)
            return;
    }
    PsyGlProgram *gl_program = PSY_GL_PROGRAM(self);
    glUseProgram(gl_program->object_id);
    psy_gl_check_error(error);
}

static void
psy_gl_program_set_uniform_matrix_4(PsyShaderProgram *self,
                                    const gchar      *name,
                                    PsyMatrix4       *matrix,
                                    GError          **error)
{
    PsyGlProgram *program = PSY_GL_PROGRAM(self);
    GLfloat       elements[16];
    psy_matrix4_get_elements(matrix, elements);

    GLint location = glGetUniformLocation(program->object_id, name);
    if (psy_gl_check_error(error))
        return;
    if (location == -1) {
        g_set_error(error,
                    PSY_GL_ERROR,
                    PSY_GL_ERROR_INVALID_VALUE,
                    "no matrix uniform with name '%s",
                    name);
        return;
    }

    glUniformMatrix4fv(location, 1, GL_FALSE, elements);
}

static void
psy_gl_program_set_uniform_4f(PsyShaderProgram *self,
                              const gchar      *name,
                              gfloat           *values,
                              GError          **error)
{
    PsyGlProgram *program = PSY_GL_PROGRAM(self);

    GLint location = glGetUniformLocation(program->object_id, name);
    if (psy_gl_check_error(error))
        return;
    if (location == -1) {
        g_set_error(error,
                    PSY_GL_ERROR,
                    PSY_GL_ERROR_INVALID_VALUE,
                    "no uniform_4f with name '%s",
                    name);
        return;
    }

    glUniform4f(location, values[0], values[1], values[2], values[3]);
}

static void
psy_gl_program_class_init(PsyGlProgramClass *class)
{
    GObjectClass          *gobject_class = G_OBJECT_CLASS(class);
    PsyShaderProgramClass *program_class = PSY_SHADER_PROGRAM_CLASS(class);

    gobject_class->get_property = psy_gl_program_get_property;
    gobject_class->finalize     = psy_gl_program_finalize;

    program_class->set_vertex_shader_source
        = psy_gl_program_set_vertex_shader_source;
    program_class->set_vertex_shader_from_file
        = psy_gl_program_set_vertex_shader_from_file;
    program_class->set_vertex_shader_from_path
        = psy_gl_program_set_vertex_shader_from_path;

    program_class->set_fragment_shader_source
        = psy_gl_program_set_fragment_shader_source;
    program_class->set_fragment_shader_from_file
        = psy_gl_program_set_fragment_shader_from_file;
    program_class->set_fragment_shader_from_path
        = psy_gl_program_set_fragment_shader_from_path;

    program_class->link        = psy_gl_program_link;
    program_class->is_linked   = psy_gl_program_is_linked;
    program_class->use_program = psy_gl_program_use_program;

    program_class->set_uniform_matrix4 = psy_gl_program_set_uniform_matrix_4;
    program_class->set_uniform_4f      = psy_gl_program_set_uniform_4f;

    gl_program_properties[PROP_OBJECT_ID]
        = g_param_spec_uint("object-id",
                            "Object ID",
                            "The OpenGL id of the object",
                            0,
                            G_MAXUINT,
                            0,
                            G_PARAM_READABLE);

    gl_program_properties[PROP_IS_LINKED] = g_param_spec_boolean(
        "is-linked",
        "Is linked",
        "Whether the shader program is successfully linked.",
        FALSE,
        G_PARAM_READABLE);

    g_object_class_install_properties(
        gobject_class, NUM_PROPERTIES, gl_program_properties);
}

/* ************ public functions ******************** */

/**
 * psy_gl_program_new:(constructor)
 *
 * Construct a new PsyGlProgram.
 *
 * Returns: A freshly created PsyGlProgram, this object may be freed with
 * psy_gl_program_free or g_object_unref.
 */
PsyGlProgram *
psy_gl_program_new(void)
{
    PsyGlProgram *gl_program = g_object_new(PSY_TYPE_GL_PROGRAM, NULL);
    return gl_program;
}

/**
 * psy_gl_program_free:(skip)
 */
void
psy_gl_program_free(PsyGlProgram *self)
{
    g_return_if_fail(PSY_IS_GL_PROGRAM(self));
    g_object_unref(self);
}

/**
 * psy_gl_get_object_id:
 *
 * Objects within OpenGL are identified via an id, this is that id. When
 * the id is 0, it is either not yet initialized or initialization failed.
 *
 * Returns: the object id of this program
 */
guint
psy_gl_program_get_object_id(PsyGlProgram *self)
{
    g_return_val_if_fail(PSY_IS_GL_PROGRAM(self), 0);

    // PsyGlProgramPrivate* priv = psy_gl_program_get_instance_private(self);

    return self->object_id;
}
