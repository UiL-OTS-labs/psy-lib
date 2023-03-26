
#include <CUnit/CUnit.h>
#include <math.h>
#include <psylib.h>

#include "unit-test-utilities.h"

static const gint WIDTH  = 640;
static const gint HEIGHT = 480;

static PsyImageCanvas *g_canvas     = NULL;
static PsyColor       *g_stim_color = NULL;
static PsyColor       *g_bg_color   = NULL;
static PsyTimePoint   *g_tp_null    = NULL;

static int
test_visual_stimulus_setup(void)
{
    g_debug("Entering %s", __func__);
    g_canvas     = psy_image_canvas_new(WIDTH, HEIGHT);
    g_stim_color = psy_color_new_rgb(1, 1, 1);
    g_bg_color   = psy_color_new_rgb(0, 0, 0);
    g_tp_null    = psy_time_point_new();

    if (!g_canvas || !g_stim_color || !g_bg_color || !g_tp_null)
        return 1;

    // make random but significantly different colors
    while (psy_color_equal_eps(g_stim_color, g_bg_color, 0.25)) {
        psy_color_set_red(g_stim_color, random_double());
        psy_color_set_green(g_stim_color, random_double());
        psy_color_set_blue(g_stim_color, random_double());
    }

    return 0;
}

static int
test_visual_stimulus_teardown(void)
{
    g_debug("Entering %s", __func__);
    g_clear_object(&g_canvas);
    g_clear_object(&g_stim_color);
    g_clear_object(&g_bg_color);
    g_clear_object(&g_tp_null);

    return 0;
}

/**
 * compute_surface_area_by_color:
 * @image:(transfer none): The input image
 * @color:(transfer none): The color that counts as being part of the stimulus
 *
 * Count the pixels in @image that have the color specified by @color. Those
 * pixels are added to the surface.
 *
 * Returns: the number of pixels with the specified color.
 */
static gint64
compute_surface_area_by_color(PsyImage *image, PsyColor *color)
{
    gint64 count = 0;

    guint8 stim_pixel[4];
    gint   r, g, b, a;
    g_object_get(color, "ri", &r, "gi", &g, "bi", &b, "ai", &a, NULL);
    stim_pixel[0] = (guint8) r;
    stim_pixel[1] = (guint8) g;
    stim_pixel[2] = (guint8) b;
    stim_pixel[3] = (guint8) a;

    guint num_channels = psy_image_get_num_channels(image);
    guint stride       = psy_image_get_stride(image);
    guint width        = psy_image_get_width(image);
    guint height       = psy_image_get_height(image);

    guint8 *row_ptr, *pixel_ptr, *image_ptr = psy_image_get_ptr(image);
    for (guint row = 0; row < height; row++) {
        row_ptr = image_ptr + row * stride;
        for (guint column = 0; column < width; column++) {
            pixel_ptr = row_ptr + column * num_channels;
            if (memcmp(pixel_ptr, stim_pixel, sizeof(stim_pixel)) == 0)
                count++;
        }
    }
    return count;
}

gdouble
circle_area(gdouble radius)
{
    return radius * radius * M_PI;
}

static void
vstim_default_values(void)
{
    PsyCircle *circle = psy_circle_new(PSY_CANVAS(g_canvas));

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
    const gfloat  radius       = 50;
    const gfloat  num_vertices = 100;
    const gfloat  scale        = (float) random_double_range(1.5, 2.5);
    PsyDuration  *frame_dur    = psy_canvas_get_frame_dur(PSY_CANVAS(g_canvas));
    PsyDuration  *stim_dur     = psy_duration_multiply_scalar(frame_dur, 10);
    PsyTimePoint *tp_start     = psy_time_point_add(g_tp_null, frame_dur);
    PsyImage     *image        = NULL;

    gfloat x, y;

    PsyCircle *circle
        = psy_circle_new_full(PSY_CANVAS(g_canvas), 0, 0, radius, num_vertices);
    psy_visual_stimulus_set_color(PSY_VISUAL_STIMULUS(circle), g_stim_color);
    psy_canvas_set_background_color(PSY_CANVAS(g_canvas), g_bg_color);

    g_object_set(circle, "scale", scale, NULL);
    g_object_get(circle, "scale_x", &x, "scale_y", &y, NULL);

    CU_ASSERT_DOUBLE_EQUAL(x, scale, 0);
    CU_ASSERT_DOUBLE_EQUAL(y, scale, 0);

    g_object_set(circle, "scale_x", scale / 2, NULL);
    g_object_set(circle, "scale_y", scale * 2, NULL);

    g_object_get(circle, "scale_x", &x, "scale_y", &y, NULL);

    CU_ASSERT_DOUBLE_EQUAL(x, scale / 2, 0);
    CU_ASSERT_DOUBLE_EQUAL(y, scale * 2, 0);

    g_object_set(circle, "scale", 1.0, NULL);
    psy_stimulus_play_for(PSY_STIMULUS(circle), tp_start, stim_dur);

    psy_image_canvas_iterate(g_canvas);

    image            = psy_canvas_get_image(PSY_CANVAS(g_canvas));
    gint64 area      = compute_surface_area_by_color(image, g_stim_color);
    gfloat comp_area = circle_area(radius);
    // allow half a pixel radius margin
    gfloat margin    = circle_area(radius) - circle_area(radius - .5);

    CU_ASSERT_DOUBLE_EQUAL(area, comp_area, margin);

    // double scaling and check whether scaled stimulus has expected surface
    g_object_set(circle, "scale", 2.0, NULL);
    psy_image_canvas_iterate(g_canvas);

    g_object_unref(image);
    image     = psy_canvas_get_image(PSY_CANVAS(g_canvas));
    area      = compute_surface_area_by_color(image, g_stim_color);
    comp_area = circle_area(radius * 2);
    margin    = circle_area(radius * 2) - circle_area((radius * 2) - .5);

    CU_ASSERT_DOUBLE_EQUAL(area, comp_area, margin);

    g_object_unref(image);
    g_object_unref(circle);
    g_object_unref(tp_start);
    g_object_unref(stim_dur);
}

int
add_visual_stimulus_suite(void)
{
    CU_Suite *suite = CU_add_suite("Visual stimulus suite",
                                   test_visual_stimulus_setup,
                                   test_visual_stimulus_teardown);

    CU_Test *test = NULL;

    if (!suite)
        return 1;

    test = CU_ADD_TEST(suite, vstim_default_values);
    if (!test)
        return 1;

    test = CU_ADD_TEST(suite, vstim_scale);
    if (!test)
        return 1;

    return 0;
}
