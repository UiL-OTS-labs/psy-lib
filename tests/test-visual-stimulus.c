
#include <CUnit/CUnit.h>
#include <psylib.h>

#include "unit-test-utilities.h"

static PsyImageCanvas *g_canvas;

static int
test_visual_stimulus_setup(void)
{
    g_canvas = psy_image_canvas_new();
    if (!g_canvas)
        return 1;

    return 0;
}

static int
test_visual_stimulus_teardown(void)
{
    g_clear_object(&g_canvas);

    return 0;
}

static int

    static void
    vstim_default_values(void)
{
    // Warns about the window parameter is NULL;
    PsyCircle *circle = psy_circle_new(NULL);

    CU_ASSERT_PTR_NOT_NULL_FATAL(circle);

    gfloat x, y, z, scale_x, scale_y, rotation;

    // clang-format off
    g_object_get(circle,
                 "x", &x,
                 "y", &y,
                 "z", &z,
                 "scale-x", &scale_x,
                 "scale-y", &scale_y,
                 "rotation", &rotation,
                 NULL);
    // clang-format on

    CU_ASSERT_DOUBLE_EQUAL(x, 0, 0);
    CU_ASSERT_DOUBLE_EQUAL(y, 0, 0);
    CU_ASSERT_DOUBLE_EQUAL(z, 0, 0);

    CU_ASSERT_DOUBLE_EQUAL(scale_x, 1, 0);
    CU_ASSERT_DOUBLE_EQUAL(scale_y, 1, 0);

    CU_ASSERT_DOUBLE_EQUAL(rotation, 0, 0);

    PsyColor *color
        = psy_visual_stimulus_get_color(PSY_VISUAL_STIMULUS(circle));
    CU_ASSERT_PTR_NULL(color);

    g_object_unref(circle);
}

static void
vstim_scale(void)
{
    const gfloat scale = (float) random_double_range(-10, 10);
    gfloat       x, y;

    PsyCircle *circle = psy_circle_new(NULL);

    g_object_set(circle, "scale", scale, NULL);
    g_object_get(circle, "scale_x", &x, "scale_y", &y, NULL);

    CU_ASSERT_DOUBLE_EQUAL(x, scale, 0);
    CU_ASSERT_DOUBLE_EQUAL(y, scale, 0);

    g_object_set(circle, "scale_x", scale / 2, NULL);
    g_object_set(circle, "scale_y", scale * 2, NULL);

    g_object_get(circle, "scale_x", &x, "scale_y", &y, NULL);

    CU_ASSERT_DOUBLE_EQUAL(x, scale / 2, 0);
    CU_ASSERT_DOUBLE_EQUAL(y, scale * 2, 0);

    g_object_unref(circle);
}

int
add_visual_stimulus_suite(void)
{
    CU_Suite *suite = CU_add_suite("Visual stimulus suite", NULL, NULL);

    CU_Test *test = NULL;

    if (!suite)
        return 1;

    test = CU_add_test(
        suite,
        "Test whether visual stimuli have sensible default values",
        vstim_default_values);
    if (!test)
        return 1;

    test = CU_add_test(suite, "Test whether scale works", vstim_scale);
    if (!test)
        return 1;

    return 0;
}
