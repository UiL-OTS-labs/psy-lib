
#include <psylib.h>

#include "gl/psy-gl-canvas.h"
#include "unit-test-utilities.h"

const gchar *g_correct_vertex_source
    = "#version 440 core\n"
      "\n"
      "layout(location = 0) in vec3 aPos;\n"
      "\n"
      "uniform mat4 projection;\n"
      "uniform mat4 model;\n"
      "\n"
      "void\n"
      "main()\n"
      "{\n"
      "    // see how we directly give a vec3 to vec4's constructor\n"
      "    vec4 vertex = vec4(aPos, 1.0);\n"
      "    gl_Position = projection * model * vertex;\n"
      "}\n";

const gchar *g_correct_fragment_source = "#version 330 core\n"
                                         "\n"
                                         "out vec4 FragColor;\n"
                                         "\n"
                                         "uniform vec4 ourColor;\n"
                                         "  \n"
                                         "void main()\n"
                                         "{\n"
                                         "    FragColor = ourColor;\n"
                                         "}\n";

// Oops unifor ourColor defined differently as in fragment
const gchar *g_incorrect_vertex_source
    = "#version 440 core\n"
      "\n"
      "layout(location = 0) in vec3 aPos;\n"
      "\n"
      "uniform mat4 projection;\n"
      "uniform mat4 model;\n"
      "uniform mat3 ourColor;\n"
      "\n"
      "void\n"
      "main()\n"
      "{\n"
      "    // see how we directly give a vec3 to vec4's constructor\n"
      "    vec4 vertex = vec4(aPos, 1.0);\n"
      "    gl_Position = projection * model * vertex;\n"
      "}\n";

static void
test_gl_utils_gl_program(void)
{
    PsyShaderProgram *program = PSY_SHADER_PROGRAM(psy_gl_program_new());
    GError           *error   = NULL;
    PsyShader        *vertex, *fragment;
    gboolean          is_linked = FALSE;
    guint             object_id = 0;

    g_assert_nonnull(program);

    psy_shader_program_set_vertex_shader_source(
        program, g_correct_vertex_source, &error);
    g_assert_no_error(error);

    vertex = psy_shader_program_get_vertex_shader(program);
    g_assert_true(psy_shader_is_compiled(vertex));
    g_object_get(vertex, "object-id", &object_id, NULL);
    g_assert_cmpuint(object_id, !=, 0u);

    psy_shader_program_set_fragment_shader_source(
        program, g_correct_fragment_source, &error);
    g_assert_no_error(error);

    fragment = psy_shader_program_get_fragment_shader(program);
    (psy_shader_is_compiled(fragment));

    psy_shader_program_link(program, &error);
    g_assert_no_error(error);

    g_object_get(
        program, "is-linked", &is_linked, "object-id", &object_id, NULL);
    g_assert_true(is_linked);
    g_assert_cmpuint(object_id, !=, 0u);

    psy_gl_program_free(PSY_GL_PROGRAM(program));
}

static void
test_gl_utils_emit_of_linking_error(void)
{
    PsyShaderProgram *program = PSY_SHADER_PROGRAM(psy_gl_program_new());
    PsyShader        *vertex, *fragment;
    GError           *error1 = NULL;
    GError           *error2 = NULL;
    gboolean          is_linked;
    guint             object_id;

    psy_shader_program_set_vertex_shader_source(
        program, g_incorrect_vertex_source, &error1);
    g_assert_no_error(
        error1); // A source with linker error is still a valid shader source

    psy_shader_program_set_fragment_shader_source(
        program, g_correct_fragment_source, &error1);
    g_assert_no_error(error1);

    vertex   = psy_shader_program_get_vertex_shader(program);
    fragment = psy_shader_program_get_fragment_shader(program);

    g_assert_true(psy_shader_is_compiled(vertex));
    g_assert_true(psy_shader_is_compiled(fragment));

    psy_shader_program_link(program, &error1);
    g_assert_error(error1, PSY_GL_ERROR, PSY_GL_ERROR_PROGRAM_LINK);

    g_object_get(
        program, "is-linked", &is_linked, "object-id", &object_id, NULL);
    g_assert_false(is_linked);
    g_assert_cmpuint(
        object_id,
        !=,
        0u); // Although not linked the program should still have an valid id

    psy_shader_program_use(program, &error2);
    // One can't use unlinked programs
    g_assert_error(error2, error1->domain, error1->code);

    g_assert_cmpstr(error1->message, ==, error2->message);

    g_clear_error(&error1);
    g_clear_error(&error2);

    psy_gl_program_free(PSY_GL_PROGRAM(program));
}

static void
test_gl_utils_gl_shader_compile_error(void)
{
    GError      *error  = NULL;
    PsyGlShader *shader = PSY_GL_SHADER(psy_gl_vertex_shader_new());
    g_assert_nonnull(shader);

    const gchar *source = "compile error source";

    psy_shader_set_source(PSY_SHADER(shader), source);
    psy_shader_compile(PSY_SHADER(shader), &error);

    g_assert_error(error, PSY_GL_ERROR, PSY_GL_ERROR_SHADER_COMPILE);
    g_assert_cmpstr(psy_shader_get_source(PSY_SHADER(shader)), ==, source);

    g_clear_error(&error);
    psy_gl_vertex_shader_free(PSY_GL_VERTEX_SHADER(shader));
}

int
main(int argc, char **argv)
{
    g_test_init(&argc, &argv, NULL);

    PsyCanvas *canvas
        = PSY_CANVAS(psy_gl_canvas_new(640, 480)); // needed for OpenGL context

    g_test_add_func("/gl_utils/gl_program", test_gl_utils_gl_program);
    g_test_add_func("/gl_utils/gl_emit_linking_error",
                    test_gl_utils_emit_of_linking_error);
    g_test_add_func("/gl_utils/gl_shader_compile_error",
                    test_gl_utils_gl_shader_compile_error);

    int save = g_test_run();
    g_clear_object(&canvas);
    return save;
}
