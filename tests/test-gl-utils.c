
#include <criterion/criterion.h>
#include <criterion/new/assert.h>
#include <psylib.h>

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

static PsyGlCanvas *g_canvas;

static void
setup_gl_utils_suite(void)
{
    // The canvas creates a gl context applicable for all drawing in this
    // test suite.
    g_canvas = psy_gl_canvas_new_full(640, 480, FALSE, TRUE, 3, 3);
}

static void
tear_down_gl_utils_suite(void)
{
    psy_gl_canvas_free(g_canvas);
}

TestSuite(gl_utils,
          .init = setup_gl_utils_suite,
          .fini = tear_down_gl_utils_suite);

Test(gl_utils, gl_program)
{
    PsyShaderProgram *program = PSY_SHADER_PROGRAM(psy_gl_program_new());
    GError           *error   = NULL;
    PsyShader        *vertex, *fragment;
    gboolean          is_linked = FALSE;
    guint             object_id = 0;

    cr_expect(ne(program, NULL), "We should be able to create a gl_program");

    psy_shader_program_set_vertex_shader_source(
        program, g_correct_vertex_source, &error);
    cr_expect(zero(error));

    vertex = psy_shader_program_get_vertex_shader(program);
    cr_expect(eq(psy_shader_is_compiled(vertex), TRUE));
    g_object_get(vertex, "object-id", &object_id, NULL);
    cr_expect(ne(object_id, 0u),
              "Once compiled the vertex shader should have an id");

    psy_shader_program_set_fragment_shader_source(
        program, g_correct_fragment_source, &error);
    cr_expect(zero(error),
              "With a valid shader source, no errors should occur");

    fragment = psy_shader_program_get_fragment_shader(program);
    (psy_shader_is_compiled(fragment));

    psy_shader_program_link(program, &error);
    cr_expect(zero(error), "Valid programs can be linked together");

    g_object_get(
        program, "is-linked", &is_linked, "object-id", &object_id, NULL);
    cr_expect(eq(is_linked, TRUE), "The program should be marked as linked");
    cr_expect(ne(object_id, 0u),
              "A valid programs should have a valid object id");

    psy_gl_program_free(PSY_GL_PROGRAM(program));
}

Test(gl_utils, emit_of_linking_error)
{
    PsyShaderProgram *program = PSY_SHADER_PROGRAM(psy_gl_program_new());
    PsyShader        *vertex, *fragment;
    GError           *error1 = NULL;
    GError           *error2 = NULL;
    gboolean          is_linked;
    guint             object_id;

    psy_shader_program_set_vertex_shader_source(
        program, g_incorrect_vertex_source, &error1);
    cr_expect(zero(error1),
              "A source with linker error is still a valid shader source");

    psy_shader_program_set_fragment_shader_source(
        program, g_correct_fragment_source, &error1);
    cr_expect(zero(error1),
              "A source with a linker error is still a valid shader source.");

    vertex   = psy_shader_program_get_vertex_shader(program);
    fragment = psy_shader_program_get_fragment_shader(program);

    cr_expect(psy_shader_is_compiled(vertex));
    cr_expect(psy_shader_is_compiled(fragment));

    psy_shader_program_link(program, &error1);
    cr_expect(ne(error1, NULL),
              "An error linking the program should be returned");
    cr_expect(eq(error1->domain, PSY_GL_ERROR),
              "It should be in the PSY_GL_ERROR domain");
    cr_expect(eq(error1->code, PSY_GL_ERROR_PROGRAM_LINK),
              "The error should be PSY_GL_ERROR_PROGRAM_LINK");

    g_object_get(
        program, "is-linked", &is_linked, "object-id", &object_id, NULL);
    cr_expect(eq(is_linked, FALSE));
    cr_expect(ne(object_id, 0u),
              "Although not linked the program should still have an valid id");

    psy_shader_program_use(program, &error2);
    cr_expect(ne(error2, NULL), "One can't use unlinked programs");
    cr_expect(eq(error1->code, error2->code));
    cr_expect(eq(error1->domain, error2->domain));
    cr_expect(eq(str, error1->message, error2->message));

    g_clear_error(&error1);
    g_clear_error(&error2);

    psy_gl_program_free(PSY_GL_PROGRAM(program));
}

Test(gl_utils, gl_shader_compile_error)
{
    GError      *error  = NULL;
    PsyGlShader *shader = PSY_GL_SHADER(psy_gl_vertex_shader_new());
    cr_assert(ne(shader, NULL), "We should be able to create a shader");

    const gchar *source = "compile error source";

    psy_shader_set_source(PSY_SHADER(shader), source);
    psy_shader_compile(PSY_SHADER(shader), &error);

    cr_assert(ne(error, NULL), "When an error occurs it should be returned.");
    cr_assert(eq(error->domain, PSY_GL_ERROR),
              "The error is in the GL_ERROR domain");
    cr_assert(eq(error->code, PSY_GL_ERROR_SHADER_COMPILE),
              "The error should be PSY_GL_ERROR_SHADER_COMPILE");
    cr_assert(eq(str,
                 (char *) psy_shader_get_source(PSY_SHADER(shader)),
                 (char *) source),
              "The source should be unchanged");

    g_clear_error(&error);
    psy_gl_vertex_shader_free(PSY_GL_VERTEX_SHADER(shader));
}
