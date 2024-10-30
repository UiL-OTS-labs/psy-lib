
#include <assert.h>

#include <CUnit/CUError.h>
#include <CUnit/CUnit.h>
#include <psylib.h>

static void
ref_starts_with_one(void)
{
    PsyColor *color      = g_object_new(PSY_TYPE_COLOR, NULL);
    GObject  *color_gobj = G_OBJECT(color);

    CU_ASSERT_EQUAL(color_gobj->ref_count, 1);

    g_object_unref(color);
}

static void
transfer_none_method(void)
{
    // clang-format off
    PsyImage *image = g_object_new(PSY_TYPE_IMAGE,
                                   "width", 640,
                                   "height", 480,
                                   "format", PSY_IMAGE_FORMAT_RGB,
                                   NULL);
    // clang-format on

    CU_ASSERT_PTR_NOT_NULL_FATAL(image);
    PsyColor *color = g_object_new(PSY_TYPE_COLOR, NULL);

    // Cast to conveniently obtain the reference count
    GObject *image_gobj = G_OBJECT(image);
    GObject *color_gobj = G_OBJECT(color);

    /*
     * This is (transfer none), so we should free it.
     */
    psy_visual_stimulus_set_color(PSY_VISUAL_STIMULUS(image), color);

    CU_ASSERT_EQUAL(image_gobj->ref_count, 1);
    CU_ASSERT_EQUAL(color_gobj->ref_count, 1);

    g_object_unref(image);
    CU_ASSERT_EQUAL(color_gobj->ref_count, 1);
    g_object_unref(color);
}

static void
set_property_transfer_full(void)
{
    PsyTrial *trial = psy_trial_new();
    PsyLoop  *loop  = psy_loop_new();

    // Cast to gobject to easily access refcount.
    GObject *trial_gobj = G_OBJECT(trial);
    GObject *loop_gobj  = G_OBJECT(loop);

    CU_ASSERT_EQUAL(trial_gobj->ref_count, 1);
    CU_ASSERT_EQUAL(loop_gobj->ref_count, 1);

    g_object_set(loop, "child", trial, NULL); // transfer full

    CU_ASSERT_EQUAL(trial_gobj->ref_count, 1); // hence still one.

    g_object_unref(loop);
}

int
add_ref_count_suite(void)
{
    CU_Suite *suite = CU_add_suite("test reference count", NULL, NULL);
    CU_Test  *test  = NULL;

    if (!suite)
        return 1;

    test = CU_ADD_TEST(suite, ref_starts_with_one);
    if (!test)
        return 1;

    test = CU_ADD_TEST(suite, transfer_none_method);
    if (!test)
        return 1;

    test = CU_ADD_TEST(suite, set_property_transfer_full);
    if (!test)
        return 1;

    return 0;
}
