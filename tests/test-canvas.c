

#include <CUnit/CUnit.h>
#include <CUnit/TestDB.h>

#include <gl/psy-gl-canvas.h>

const gint WIDTH  = 640;
const gint HEIGHT = 480;

static void
canvas_initialization(void)
{
    // Use PsyGlCanvas as PsyCanvas is abstract
    PsyGlCanvas *canvas = psy_gl_canvas_new(WIDTH, HEIGHT);
    gint         width, height;

    CU_ASSERT_PTR_NOT_NULL_FATAL(canvas);

    g_object_get(canvas, "height", &height, "width", &width, NULL);
    CU_ASSERT_EQUAL(width, WIDTH);
    CU_ASSERT_EQUAL(height, HEIGHT);

    g_object_unref(canvas);
}

static void
canvas_background_color(void)
{
    PsyGlCanvas *canvas     = psy_gl_canvas_new(WIDTH, HEIGHT);
    PsyColor    *default_bg = NULL;
    PsyColor    *new_color  = g_object_new(
        PSY_TYPE_COLOR, "r", 0.0f, "g", 0.0f, "b", 0.0f, "a", 0.0f, NULL);
    gfloat r, g, b;

    CU_ASSERT_PTR_NOT_NULL_FATAL(canvas);

    g_object_get(canvas, "background-color", &default_bg, NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(default_bg);
    g_object_get(default_bg, "r", &r, "g", &g, "b", &b, NULL);

    CU_ASSERT_EQUAL(r, 0.5);
    CU_ASSERT_EQUAL(g, 0.5);
    CU_ASSERT_EQUAL(b, 0.5);

    g_object_set(canvas, "background-color", new_color, NULL);

    CU_ASSERT(psy_color_equal(
        new_color, psy_canvas_get_background_color(PSY_CANVAS(canvas))));

    g_object_unref(new_color);
    g_object_unref(default_bg);
    g_object_unref(canvas);
}

static void
canvas_size_vd(void)
{
    gfloat width_vd, height_vd;

    gint width_mm = 640, height_mm = 480, distance_mm = 1000;

    // clang-format off
    PsyCanvas* canvas = g_object_new(
            PSY_TYPE_GL_CANVAS,
            "width", WIDTH,
            "height", HEIGHT,
            "width-mm", width_mm,
            "height-mm", height_mm,
            "distance-mm", distance_mm,
            NULL
            );
    
    g_object_get(
            canvas,
            "width-vd", &width_vd,
            "height-vd", &height_vd,
            NULL);
    // clang-format on

    CU_ASSERT_EQUAL(
        width_vd,
        2 * psy_radians_to_degrees(atan(width_mm / 2.0 / distance_mm)));
    CU_ASSERT_EQUAL(
        height_vd,
        2 * psy_radians_to_degrees(atan(height_mm / 2.0 / distance_mm)));

    g_print("width_vd = %f\t height_vd = %f\n", width_vd, height_vd);
}

int
add_canvas_suite(void)
{
    CU_Suite *suite = CU_add_suite("canvas tests", NULL, NULL);
    CU_Test  *test  = NULL;

    if (!suite)
        return 1;

    test = CU_ADD_TEST(suite, canvas_initialization);
    if (!test)
        return 1;

    test = CU_ADD_TEST(suite, canvas_background_color);
    if (!test)
        return 1;

    test = CU_ADD_TEST(suite, canvas_size_vd);
    if (!test)
        return 1;

    return 0;
}
