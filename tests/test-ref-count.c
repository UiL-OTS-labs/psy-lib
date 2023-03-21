
#include <assert.h>

#include <CUnit/CUError.h>
#include <CUnit/CUnit.h>
#include <psylib.h>

static PsyCanvas *canvas = NULL;

static int
init_window(void)
{
    canvas = PSY_CANVAS(psy_image_canvas_new(640, 480));
    if (canvas)
        return 0;
    else
        return 1;
}

static int
destoy_window(void)
{
    assert(G_OBJECT(canvas)->ref_count == 1);
    g_object_unref(canvas);
    canvas = NULL;
    return 0;
}

static void
ref_starts_with_one(void)
{
    PsyCircle *circle = g_object_new(PSY_TYPE_CIRCLE, "canvas", canvas, NULL);
    PsyColor  *color  = g_object_new(PSY_TYPE_COLOR, NULL);
    GObject   *circle_gobj = G_OBJECT(circle);
    GObject   *color_gobj  = G_OBJECT(color);

    CU_ASSERT_EQUAL(circle_gobj->ref_count, 1);
    CU_ASSERT_EQUAL(color_gobj->ref_count, 1);

    g_object_unref(circle);
    g_object_unref(color);
}

static void
ref_set_method(void)
{
    PsyCircle *circle = g_object_new(PSY_TYPE_CIRCLE, "canvas", canvas, NULL);
    PsyColor  *color  = g_object_new(PSY_TYPE_COLOR, NULL);

    // Cast to conveniently obtain the reference count
    GObject *circle_gobj = G_OBJECT(circle);
    GObject *color_gobj  = G_OBJECT(color);

    /*
     * This is (transfer none), so the circle does not own the reference,
     * but has a reference to it.
     * Both we and the Circle should free it.
     */
    psy_visual_stimulus_set_color(PSY_VISUAL_STIMULUS(circle), color);

    CU_ASSERT_EQUAL(circle_gobj->ref_count, 1);
    CU_ASSERT_EQUAL(color_gobj->ref_count, 2);

    // The circle disposes its reference to color
    g_object_unref(circle);

    CU_ASSERT_EQUAL(color_gobj->ref_count, 1);

    g_object_unref(color);
}

static void
ref_set_property(void)
{
    PsyCircle *circle = g_object_new(PSY_TYPE_CIRCLE, "canvas", canvas, NULL);
    PsyColor  *color  = g_object_new(PSY_TYPE_COLOR, NULL);
    GObject   *circle_gobj = G_OBJECT(circle);
    GObject   *color_gobj  = G_OBJECT(color);

    CU_ASSERT_EQUAL(circle_gobj->ref_count, 1);
    CU_ASSERT_EQUAL(color_gobj->ref_count, 1);

    // Circle is owning a reference
    g_object_set(circle, "color", color, NULL);

    CU_ASSERT_EQUAL(circle_gobj->ref_count, 1);
    CU_ASSERT_EQUAL(color_gobj->ref_count, 2);

    g_object_unref(circle); // Circle disposes its reference on color
    CU_ASSERT_EQUAL(color_gobj->ref_count, 1);

    g_object_unref(color);
}

int
add_ref_count_suite(void)
{
    CU_Suite *suite
        = CU_add_suite("test reference count", init_window, destoy_window);
    CU_Test *test = NULL;

    if (!suite)
        return 1;

    test = CU_add_test(
        suite, "Object start with reference of 1", ref_starts_with_one);
    if (!test)
        return 1;

    test = CU_add_test(suite, "Objects ref set method", ref_set_method);
    if (!test)
        return 1;

    test = CU_add_test(suite, "Objects ref set property", ref_set_property);
    if (!test)
        return 1;

    return 0;
}
