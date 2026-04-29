
#include <criterion/criterion.h>
#include <criterion/new/assert.h>

#include <psylib.h>

Test(ref, starts_with_one)
{
    PsyColor *color      = g_object_new(PSY_TYPE_COLOR, NULL);
    GObject  *color_gobj = G_OBJECT(color);

    cr_assert(eq(color_gobj->ref_count, 1u));

    g_object_unref(color);
}

Test(ref, transfer_none_method)
{
    // clang-format off
    PsyImage *image = g_object_new(PSY_TYPE_IMAGE,
                                   "width", 640,
                                   "height", 480,
                                   "format", PSY_IMAGE_FORMAT_RGB,
                                   NULL);
    // clang-format on

    cr_assert(ne(image, NULL));
    PsyColor *color = g_object_new(PSY_TYPE_COLOR, NULL);

    // Cast to conveniently obtain the reference count
    GObject *image_gobj = G_OBJECT(image);
    GObject *color_gobj = G_OBJECT(color);

    /*
     * This is (transfer none), so we should free it.
     */
    psy_visual_stimulus_set_color(PSY_VISUAL_STIMULUS(image), color);

    cr_expect(eq(image_gobj->ref_count, 1u),
              "Image should have a ref count of 1");
    cr_expect(eq(color_gobj->ref_count, 1u),
              "Since, nothing is transferred, ref count should remain 1");

    g_object_unref(image);
    cr_expect(eq(color_gobj->ref_count, 1u));
    g_object_unref(color);
}

Test(ref, set_property_transfer_full)
{
    PsyTrial *trial = psy_trial_new();
    PsyLoop  *loop  = psy_loop_new();

    // Cast to gobject to easily access refcount.
    GObject *trial_gobj = G_OBJECT(trial);
    GObject *loop_gobj  = G_OBJECT(loop);

    cr_assert(eq(trial_gobj->ref_count, 1u));
    cr_assert(eq(loop_gobj->ref_count, 1u));

    g_object_set(loop, "child", trial, NULL); // transfer full

    cr_expect(eq(trial_gobj->ref_count, 1u)); // hence still one.

    g_object_unref(loop);
}
