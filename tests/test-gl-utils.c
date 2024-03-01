
#include <CUnit/CUnit.h>
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

static int
setup_gl_utils_suite(void)
{
    // The canvas creates a gl context applicable for all drawing in this
    // test suite.
    g_canvas = psy_gl_canvas_new_full(640, 480, FALSE, TRUE, 3, 3);

    return 0;
}

static int
tear_down_gl_utils_suite(void)
{
    psy_gl_canvas_free(g_canvas);

    return 0;
}

static void
test_gl_program(void)
{
    PsyShaderProgram *program = PSY_SHADER_PROGRAM(psy_gl_program_new());
    GError           *error   = NULL;
    PsyShader        *vertex, *fragment;
    gboolean          is_linked = FALSE;
    guint             object_id = 0;

    CU_ASSERT_PTR_NOT_NULL_FATAL(program);

    psy_shader_program_set_vertex_shader_source(
        program, g_correct_vertex_source, &error);
    CU_ASSERT_PTR_NULL(error);
    vertex = psy_shader_program_get_vertex_shader(program);
    CU_ASSERT(psy_shader_is_compiled(vertex));
    g_object_get(vertex, "object-id", &object_id, NULL);
    CU_ASSERT_NOT_EQUAL(object_id, 0);

    psy_shader_program_set_fragment_shader_source(
        program, g_correct_fragment_source, &error);
    CU_ASSERT_PTR_NULL(error);
    fragment = psy_shader_program_get_fragment_shader(program);
    CU_ASSERT(psy_shader_is_compiled(fragment));

    psy_shader_program_link(program, &error);
    CU_ASSERT_PTR_NULL(error);

    g_object_get(
        program, "is-linked", &is_linked, "object-id", &object_id, NULL);
    CU_ASSERT_TRUE(is_linked);
    CU_ASSERT_NOT_EQUAL(object_id, 0);

    psy_gl_program_free(PSY_GL_PROGRAM(program));
}

static void
test_emit_of_linking_error(void)
{
    PsyShaderProgram *program = PSY_SHADER_PROGRAM(psy_gl_program_new());
    PsyShader        *vertex, *fragment;
    GError           *error1 = NULL;
    GError           *error2 = NULL;
    gboolean          is_linked;
    guint             object_id;

    psy_shader_program_set_vertex_shader_source(
        program, g_incorrect_vertex_source, &error1);
    CU_ASSERT_PTR_NULL(error1);

    psy_shader_program_set_fragment_shader_source(
        program, g_correct_fragment_source, &error1);
    CU_ASSERT_PTR_NULL(error1);

    vertex   = psy_shader_program_get_vertex_shader(program);
    fragment = psy_shader_program_get_vertex_shader(program);
    CU_ASSERT(psy_shader_is_compiled(vertex));
    CU_ASSERT(psy_shader_is_compiled(fragment));

    psy_shader_program_link(program, &error1);
    CU_ASSERT_PTR_NOT_NULL(error1);

    g_object_get(
        program, "is-linked", &is_linked, "object-id", &object_id, NULL);
    CU_ASSERT_FALSE(is_linked);
    CU_ASSERT_NOT_EQUAL(object_id, 0);

    psy_shader_program_use(program, &error2);
    CU_ASSERT_PTR_NOT_NULL(error2);
    CU_ASSERT_EQUAL(error1->code, error2->code)
    CU_ASSERT_EQUAL(error1->domain, error2->domain)
    CU_ASSERT_STRING_EQUAL(error1->message, error2->message)

    g_clear_error(&error1);
    g_clear_error(&error2);

    psy_gl_program_free(PSY_GL_PROGRAM(program));
}

static void
test_gl_shader_compile_error(void)
{
    GError      *error  = NULL;
    PsyGlShader *shader = PSY_GL_SHADER(psy_gl_vertex_shader_new());
    CU_ASSERT_PTR_NOT_NULL_FATAL(shader);

    const gchar *source = "compile error source";

    psy_shader_set_source(PSY_SHADER(shader), source);
    psy_shader_compile(PSY_SHADER(shader), &error);
    CU_ASSERT_PTR_NOT_NULL_FATAL(error);
    CU_ASSERT_EQUAL(error->domain, PSY_GL_ERROR);
    CU_ASSERT_EQUAL(error->code, PSY_GL_ERROR_SHADER_COMPILE);
    CU_ASSERT_STRING_EQUAL(psy_shader_get_source(PSY_SHADER(shader)), source);

    g_clear_error(&error);
    psy_gl_vertex_shader_free(PSY_GL_VERTEX_SHADER(shader));
}

int
add_gl_utils_suite(void)
{
    CU_Suite *suite = CU_add_suite("psy OpenGL utils suite",
                                   setup_gl_utils_suite,
                                   tear_down_gl_utils_suite);
    CU_Test  *test;
    if (!suite)
        return 1;

    test = CU_ADD_TEST(suite, test_gl_program);
    if (!test)
        return 1;

    test = CU_ADD_TEST(suite, test_emit_of_linking_error);
    if (!test)
        return 1;

    test = CU_ADD_TEST(suite, test_gl_shader_compile_error);
    if (!test)
        return 1;

    return 0;
}
