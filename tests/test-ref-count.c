
#include "psy-image.h"
#include <psylib.h>

static void
test_ref_starts_with_one(void)
{
    PsyColor *color      = g_object_new(PSY_TYPE_COLOR, NULL);
    GObject  *color_gobj = G_OBJECT(color);

    g_assert_cmpuint(color_gobj->ref_count, ==, 1u);

    g_object_unref(color);
}

static void
test_ref_transfer_none_method(void)
{
    // clang-format off
    PsyImage *image = g_object_new(PSY_TYPE_IMAGE,
                                   "width", 640,
                                   "height", 480,
                                   "format", PSY_IMAGE_FORMAT_RGB,
                                   NULL);
    // clang-format on

    g_assert_nonnull(image);
    PsyColor *color = g_object_new(PSY_TYPE_COLOR, NULL);

    // Cast to conveniently obtain the reference count
    GObject *image_gobj = G_OBJECT(image);
    GObject *color_gobj = G_OBJECT(color);

    /*
     * This is (transfer none), so we should free it.
     */
    psy_image_clear(image, color);

    // Image should have a ref count of 1
    g_assert_cmpuint(image_gobj->ref_count, ==, 1u);
    // Since, nothing is transferred, ref count should remain 1
    g_assert_cmpuint(color_gobj->ref_count, ==, 1u);

    g_object_unref(image);
    g_assert_cmpuint(color_gobj->ref_count, ==, 1u);
    g_object_unref(color);
}

static void
test_ref_set_property_transfer_full(void)
{
    PsyTrial *trial = psy_trial_new();
    PsyLoop  *loop  = psy_loop_new();

    // Cast to gobject to easily access refcount.
    GObject *trial_gobj = G_OBJECT(trial);
    GObject *loop_gobj  = G_OBJECT(loop);

    g_assert_cmpuint(trial_gobj->ref_count, ==, 1u);
    g_assert_cmpuint(loop_gobj->ref_count, ==, 1u);

    g_object_set(loop, "child", trial, NULL); // transfer full

    g_assert_cmpuint(trial_gobj->ref_count, ==, 1u); // hence still one.

    g_object_unref(loop);
}

int
main(int argc, char **argv)
{
    g_test_init(&argc, &argv, NULL);

    g_test_add_func("/ref/starts_with_one", test_ref_starts_with_one);
    g_test_add_func("/ref/transfer_none", test_ref_transfer_none_method);
    g_test_add_func("/ref/set_property_transfer_full",
                    test_ref_set_property_transfer_full);

    return g_test_run();
}
