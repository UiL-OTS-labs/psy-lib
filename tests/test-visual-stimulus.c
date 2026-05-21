
#include <math.h>
#include <psylib.h>

#include "psy-init.h"
#include "unit-test-utilities.h"

static const gint WIDTH  = 640;
static const gint HEIGHT = 480;

static PsyImageCanvas *g_canvas     = NULL;
static PsyColor       *g_stim_color = NULL;
static PsyColor       *g_bg_color   = NULL;
static PsyInitializer *g_init       = NULL;
// a convenient start time  start + 16.67 ms otherwise stimuli are
// scheduled to a frame that has already been drawn.
static PsyTimePoint *g_tp_start     = NULL;

static void
visual_stimulus_setup(void)
{
    g_init = g_object_new(
        PSY_TYPE_INITIALIZER, "gstreamer", FALSE, "portaudio", FALSE, NULL);

    set_log_handler_file("test-visual-stimulus.txt");
    g_debug("Entering %s", __func__);
    g_canvas     = psy_image_canvas_new(WIDTH, HEIGHT);
    g_stim_color = psy_color_new_rgbi(g_test_rand_int_range(0, 255),
                                      g_test_rand_int_range(0, 255),
                                      g_test_rand_int_range(0, 255));
    g_bg_color   = psy_color_new_rgbi(g_test_rand_int_range(0, 255),
                                    g_test_rand_int_range(0, 255),
                                    g_test_rand_int_range(0, 255));

    PsyTimePoint *temp = psy_image_canvas_get_time(g_canvas);
    g_tp_start         = psy_time_point_add(
        temp, psy_canvas_get_frame_dur(PSY_CANVAS(g_canvas)));
    psy_time_point_free(temp);

    if (!g_canvas || !g_stim_color || !g_bg_color || !g_tp_start) {
        g_warning("Oops critical object NULL in :%s", __func__);
        return;
    }

    // make random but significantly different colors
    while (psy_color_equal_eps(g_stim_color, g_bg_color, 0.25f)) {
        psy_color_set_redi(g_stim_color, g_test_rand_int_range(0, 255));
        psy_color_set_greeni(g_stim_color, g_test_rand_int_range(0, 255));
        psy_color_set_bluei(g_stim_color, g_test_rand_int_range(0, 255));
    }
}

static void
visual_stimulus_teardown(void)
{
    g_debug("Entering %s", __func__);
    g_clear_object(&g_canvas);
    g_clear_object(&g_stim_color);
    g_clear_object(&g_bg_color);
    g_clear_pointer(&g_tp_start, psy_time_point_free);

    set_log_handler_file(NULL);

    g_clear_object(&g_init);
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

/**
 * compute_surface_avg_stim_pos:
 * @image:(transfer none): The input image
 * @color:(transfer none): The color that counts as being part of the stimulus
 * @x:(out): avg x
 * @y:(out): avg y
 *
 * Count the average x and y position of the pixels in @image that have the
 * color specified by @color.
 */
static void
compute_surface_avg_stim_pos(PsyImage *image,
                             PsyColor *color,
                             gdouble  *x,
                             gdouble  *y)
{
    gint64 count = 0;
    *x           = 0;
    *y           = 0;

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
            if (memcmp(pixel_ptr, stim_pixel, sizeof(stim_pixel)) == 0) {
                count++;
                *x += column;
                *y += row;
            }
        }
    }

    *x = *x / count;
    *y = *y / count;
}

gdouble
circle_area(gdouble radius)
{
    return radius * radius * M_PI;
}

static void
test_vstim_default_values(void)
{
    PsyCircle *circle        = psy_circle_new(PSY_CANVAS(g_canvas));
    PsyColor  *default_color = psy_color_new();

    g_assert_nonnull(circle);
    g_assert_nonnull(default_color);
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

    g_assert_cmpfloat(x, ==, 0); // The default x = 0
    g_assert_cmpfloat(y, ==, 0); // The default y = 0
    g_assert_cmpfloat(z, ==, 0); // The default z = 0

    g_assert_cmpfloat(scale_x, ==, 1); // The default x scaling = 1
    g_assert_cmpfloat(scale_y, ==, 1); // The default y scaling = 1

    g_assert_cmpfloat(rotation, ==, 0); // The default isn't rotated

    PsyColor *color
        = psy_visual_stimulus_get_color(PSY_VISUAL_STIMULUS(circle));
    g_assert_nonnull(color);

    // It should return a deep copy of the color
    g_assert_cmphex((uintptr_t) default_color, !=, (uintptr_t) color);
    g_assert_true(psy_color_equal(default_color, color));

    g_object_unref(circle);
    g_object_unref(default_color);
}

static void
test_vstim_scale(void)
{
    const gfloat radius       = 50;
    const gfloat num_vertices = 100;
    const gfloat scale        = (float) g_test_rand_double_range(1.5, 2.5);
    PsyDuration *frame_dur    = psy_canvas_get_frame_dur(PSY_CANVAS(g_canvas));
    PsyDuration *stim_dur     = psy_duration_multiply_scalar(frame_dur, 10);
    PsyImage    *image        = NULL;

    gfloat x, y;

    psy_canvas_reset(PSY_CANVAS(g_canvas));

    PsyCircle *circle
        = psy_circle_new_full(PSY_CANVAS(g_canvas), 0, 0, radius, num_vertices);
    psy_visual_stimulus_set_color(PSY_VISUAL_STIMULUS(circle), g_stim_color);
    psy_canvas_set_background_color(PSY_CANVAS(g_canvas), g_bg_color);

    g_object_set(circle, "scale", scale, NULL);
    g_object_get(circle, "scale_x", &x, "scale_y", &y, NULL);

    // setting the scale property scales in the x_direction
    g_assert_cmpfloat(x, ==, scale);
    // setting the scale property scales in the y direction
    g_assert_cmpfloat(y, ==, scale);

    g_object_set(circle, "scale_x", scale / 2, NULL);
    g_object_set(circle, "scale_y", scale * 2, NULL);

    g_object_get(circle, "scale_x", &x, "scale_y", &y, NULL);

    // x and y can be scaled separately
    g_assert_cmpfloat(x, ==, scale / 2);
    g_assert_cmpfloat(y, ==, scale * 2);

    g_object_set(circle, "scale", 1.0, NULL);
    psy_stimulus_play_for(PSY_STIMULUS(circle), g_tp_start, stim_dur);

    psy_image_canvas_iterate(g_canvas);

    image = psy_canvas_get_image(PSY_CANVAS(g_canvas));
    if (save_images())
        save_image_tmp_png(image, "%s-scale-%d.png", __func__, 1);

    gint64 area      = compute_surface_area_by_color(image, g_stim_color);
    gfloat comp_area = (float) circle_area(radius);
    // allow half a pixel radius margin
    gfloat margin
        = (float) circle_area(radius) - (float) circle_area(radius - .5);

    // The obtained area should math the mathematical area
    g_assert_cmpfloat_with_epsilon(area, comp_area, margin);

    // double scaling and check whether scaled stimulus has expected surface
    g_object_set(circle, "scale", 2.0, NULL);
    psy_image_canvas_iterate(g_canvas);

    g_object_unref(image);
    image     = psy_canvas_get_image(PSY_CANVAS(g_canvas));
    area      = compute_surface_area_by_color(image, g_stim_color);
    comp_area = (float) circle_area(radius * 2);
    margin    = (float) circle_area(radius * 2)
             - (float) circle_area((radius * 2) - .5);

    // A bigger circle gives a bigger area
    g_assert_cmpfloat_with_epsilon(area, comp_area, margin);

    if (save_images())
        save_image_tmp_png(image, "%s-scale-%d.png", __func__, 2);

    g_object_unref(image);
    g_object_unref(circle);
    psy_duration_free(stim_dur);
}

static void
test_vstim_translate(void)
{
    gfloat radius       = (float) g_test_rand_double_range(10, 20);
    guint  num_vertices = 100;

    gfloat  obtain_x, tx = (float) g_test_rand_double_range(-100, 100);
    gfloat  obtain_y, ty = (float) g_test_rand_double_range(-100, 100);
    gdouble avg_x, avg_y;

    psy_canvas_reset(PSY_CANVAS(g_canvas));

    PsyCircle *circle = psy_circle_new_full(
        PSY_CANVAS(g_canvas), tx, ty, radius, num_vertices);
    PsyImage *image = NULL;

    psy_visual_stimulus_set_color(PSY_VISUAL_STIMULUS(circle), g_stim_color);
    psy_canvas_set_background_color(PSY_CANVAS(g_canvas), g_bg_color);

    psy_stimulus_play_for(PSY_STIMULUS(circle),
                          g_tp_start,
                          psy_canvas_get_frame_dur(PSY_CANVAS(g_canvas)));

    g_object_get(circle, "x", &obtain_x, "y", &obtain_y, NULL);

    // the properties gotten from the stimulus should match the input
    g_assert_cmpfloat_with_epsilon(obtain_x, tx, 1e-9);
    g_assert_cmpfloat_with_epsilon(obtain_y, ty, 1e-9);

    psy_image_canvas_iterate(g_canvas);
    image = psy_canvas_get_image(PSY_CANVAS(g_canvas));

    compute_surface_avg_stim_pos(image, g_stim_color, &avg_x, &avg_y);
    //  avg_x = avg_x - WIDTH / 2.0;
    //  avg_y = avg_y + HEIGHT / 2.0;
    psy_coordinate_c_to_center(psy_image_get_width(image),
                               psy_image_get_height(image),
                               avg_x,
                               avg_y,
                               &avg_x,
                               &avg_y);
    g_info("\ntx =%lf, ty%lf, avg_x=%lf, avg_y=%lf\n", tx, ty, avg_x, avg_y);
    g_assert_cmpfloat_with_epsilon(avg_x, tx, 1);
    g_assert_cmpfloat_with_epsilon(avg_y, ty, 1);

    if (save_images()) {
        save_image_tmp_png(image,
                           "%s-x%d-y%d.png",
                           __func__,
                           (int) round(tx),
                           (int) round(ty));
    }

    g_object_unref(image);
    g_object_unref(circle);
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcomment"

static void
test_vstim_rotate(void)
{
    psy_canvas_reset(PSY_CANVAS(g_canvas));
    psy_canvas_set_background_color(PSY_CANVAS(g_canvas), g_bg_color);

    gint   angle_0 = 0, angle_45 = 45, angle_m45 = -45;
    gfloat radians, expected = 0.0f;

    PsyRectangle *rect
        = psy_rectangle_new_full(PSY_CANVAS(g_canvas), 0, 0, 10, 200);
    psy_visual_stimulus_set_color(PSY_VISUAL_STIMULUS(rect), g_stim_color);
    psy_visual_stimulus_set_rotation_deg(PSY_VISUAL_STIMULUS(rect), angle_0);
    g_object_get(rect, "rotation", &radians, NULL);
    g_assert_cmpfloat_with_epsilon(radians, expected, 1e-9);

    PsyDuration *dur = psy_duration_new_ms(50); // 3 frames

    psy_stimulus_play_for(PSY_STIMULUS(rect), g_tp_start, dur);

    // Run for image iteration with no rotation.
    //
    // The '*'s mark the sample points The /, | and \ mark the rectangle
    // So in this example the middle three should sample point should have
    // the stimulus color the others the background
    //
    //      |
    //   *  *  *
    //      |
    //   *  *  *
    //      |
    //   *  *  *
    //      |
    //

    psy_image_canvas_iterate(g_canvas);

    PsyImage *image = psy_canvas_get_image(PSY_CANVAS(g_canvas));
    if (save_images())
        save_image_tmp_png(image, "%s_%d.png", __func__, angle_0);

    gint x1, y1, x2, y2, x3, y3, x4, y4, x5, y5, x6, y6, x7, y7, x8, y8, x9, y9;

    gint WIDTH  = (int) psy_image_get_width(image);
    gint HEIGHT = (int) psy_image_get_height(image);

    psy_coordinate_center_to_c_i(WIDTH, HEIGHT, -50, 50, &x1, &y1);
    psy_coordinate_center_to_c_i(WIDTH, HEIGHT, 0, 50, &x2, &y2);
    psy_coordinate_center_to_c_i(WIDTH, HEIGHT, 50, 50, &x3, &y3);
    psy_coordinate_center_to_c_i(WIDTH, HEIGHT, -50, 0, &x4, &y4);
    psy_coordinate_center_to_c_i(WIDTH, HEIGHT, 0, 0, &x5, &y5);
    psy_coordinate_center_to_c_i(WIDTH, HEIGHT, 50, 0, &x6, &y6);
    psy_coordinate_center_to_c_i(WIDTH, HEIGHT, -50, -50, &x7, &y7);
    psy_coordinate_center_to_c_i(WIDTH, HEIGHT, 0, -50, &x8, &y8);
    psy_coordinate_center_to_c_i(WIDTH, HEIGHT, 50, -50, &x9, &y9);

    PsyColor *c1 = psy_image_get_pixel(image, y1, x1);
    PsyColor *c2 = psy_image_get_pixel(image, y2, x2);
    PsyColor *c3 = psy_image_get_pixel(image, y3, x3);
    PsyColor *c4 = psy_image_get_pixel(image, y4, x4);
    PsyColor *c5 = psy_image_get_pixel(image, y5, x5);
    PsyColor *c6 = psy_image_get_pixel(image, y6, x6);
    PsyColor *c7 = psy_image_get_pixel(image, y7, x7);
    PsyColor *c8 = psy_image_get_pixel(image, y8, x8);
    PsyColor *c9 = psy_image_get_pixel(image, y9, x9);

    g_assert_false(psy_color_equal_eps(g_stim_color, c1, 1.0 / 255));
    g_assert_true(psy_color_equal_eps(g_stim_color, c2, 1.0 / 255));
    g_assert_false(psy_color_equal_eps(g_stim_color, c3, 1.0 / 255));

    g_assert_false(psy_color_equal_eps(g_stim_color, c4, 1.0 / 255));
    g_assert_true(psy_color_equal_eps(g_stim_color, c5, 1.0 / 255));
    g_assert_false(psy_color_equal_eps(g_stim_color, c6, 1.0 / 255));

    g_assert_false(psy_color_equal_eps(g_stim_color, c7, 1.0 / 255));
    g_assert_true(psy_color_equal_eps(g_stim_color, c8, 1.0 / 255));
    g_assert_false(psy_color_equal_eps(g_stim_color, c9, 1.0 / 255));

    g_object_unref(c1);
    g_object_unref(c2);
    g_object_unref(c3);
    g_object_unref(c4);
    g_object_unref(c5);
    g_object_unref(c6);
    g_object_unref(c7);
    g_object_unref(c8);
    g_object_unref(c9);
    g_object_unref(image);

    // tilt rectangle 45% of 1/8 * (2*M_PI) to the left
    //
    //   \
    //    *  *  *
    //      \
    //    *  *  *
    //        \
    //    *  *  *
    //           \
    //
    psy_visual_stimulus_set_rotation_deg(PSY_VISUAL_STIMULUS(rect), angle_45);
    g_object_get(rect, "rotation", &radians, NULL);
    expected = 1.0 / 4 * M_PI;
    g_assert_cmpfloat_with_epsilon(radians, expected, 1e-9);

    psy_image_canvas_iterate(g_canvas);
    image = psy_canvas_get_image(PSY_CANVAS(g_canvas));
    if (save_images())
        save_image_tmp_png(image, "%s_%d.png", __func__, angle_45);

    c1 = psy_image_get_pixel(image, y1, x1);
    c2 = psy_image_get_pixel(image, y2, x2);
    c3 = psy_image_get_pixel(image, y3, x3);
    c4 = psy_image_get_pixel(image, y4, x4);
    c5 = psy_image_get_pixel(image, y5, x5);
    c6 = psy_image_get_pixel(image, y6, x6);
    c7 = psy_image_get_pixel(image, y7, x7);
    c8 = psy_image_get_pixel(image, y8, x8);
    c9 = psy_image_get_pixel(image, y9, x9);

    g_assert_true(psy_color_equal_eps(g_stim_color, c1, 1.0 / 255));
    g_assert_false(psy_color_equal_eps(g_stim_color, c2, 1.0 / 255));
    g_assert_false(psy_color_equal_eps(g_stim_color, c3, 1.0 / 255));

    g_assert_false(psy_color_equal_eps(g_stim_color, c4, 1.0 / 255));
    g_assert_true(psy_color_equal_eps(g_stim_color, c5, 1.0 / 255));
    g_assert_false(psy_color_equal_eps(g_stim_color, c6, 1.0 / 255));

    g_assert_false(psy_color_equal_eps(g_stim_color, c7, 1.0 / 255));
    g_assert_false(psy_color_equal_eps(g_stim_color, c8, 1.0 / 255));
    g_assert_true(psy_color_equal_eps(g_stim_color, c9, 1.0 / 255));

    g_object_unref(c1);
    g_object_unref(c2);
    g_object_unref(c3);
    g_object_unref(c4);
    g_object_unref(c5);
    g_object_unref(c6);
    g_object_unref(c7);
    g_object_unref(c8);
    g_object_unref(c9);
    g_object_unref(image);

    // tilt rectangle 45% of 1/8 * (2*M_PI) to the right
    //
    //            /
    //     *  *  *
    //          /
    //     *  *  *
    //       /
    //     *  *  *
    //    /
    //
    psy_visual_stimulus_set_rotation_deg(PSY_VISUAL_STIMULUS(rect), angle_m45);

    g_object_get(rect, "rotation", &radians, NULL);
    expected = -1.0 / 4 * M_PI;
    g_assert_cmpfloat_with_epsilon(radians, expected, 1e-9);

    psy_image_canvas_iterate(g_canvas);
    image = psy_canvas_get_image(PSY_CANVAS(g_canvas));
    if (save_images())
        save_image_tmp_png(image, "%s_%d.png", __func__, angle_m45);

    c1 = psy_image_get_pixel(image, y1, x1);
    c2 = psy_image_get_pixel(image, y2, x2);
    c3 = psy_image_get_pixel(image, y3, x3);
    c4 = psy_image_get_pixel(image, y4, x4);
    c5 = psy_image_get_pixel(image, y5, x5);
    c6 = psy_image_get_pixel(image, y6, x6);
    c7 = psy_image_get_pixel(image, y7, x7);
    c8 = psy_image_get_pixel(image, y8, x8);
    c9 = psy_image_get_pixel(image, y9, x9);

    g_assert_false(psy_color_equal_eps(g_stim_color, c1, 1.0f / 255));
    g_assert_false(psy_color_equal_eps(g_stim_color, c2, 1.0f / 255));
    g_assert_true(psy_color_equal_eps(g_stim_color, c3, 1.0f / 255));

    g_assert_false(psy_color_equal_eps(g_stim_color, c4, 1.0f / 255));
    g_assert_true(psy_color_equal_eps(g_stim_color, c5, 1.0f / 255));
    g_assert_false(psy_color_equal_eps(g_stim_color, c6, 1.0f / 255));

    g_assert_true(psy_color_equal_eps(g_stim_color, c7, 1.0f / 255));
    g_assert_false(psy_color_equal_eps(g_stim_color, c8, 1.0f / 255));
    g_assert_false(psy_color_equal_eps(g_stim_color, c9, 1.0f / 255));

    g_object_unref(c1);
    g_object_unref(c2);
    g_object_unref(c3);
    g_object_unref(c4);
    g_object_unref(c5);
    g_object_unref(c6);
    g_object_unref(c7);
    g_object_unref(c8);
    g_object_unref(c9);
    g_object_unref(image);

    psy_duration_free(dur);
    g_object_unref(rect);
}

#pragma GCC diagnostic push

static void
test_vstim_draworder_same_z(void)
{
    psy_canvas_reset(PSY_CANVAS(g_canvas));
    psy_canvas_set_background_color(PSY_CANVAS(g_canvas), g_bg_color);
    PsyImage    *image = NULL;
    PsyDuration *dur   = psy_duration_new_ms(50);

    gfloat z1, z2;

    PsyColor *rect1_color
        = g_object_new(PSY_TYPE_COLOR, "r", 1.0f, "g", 0.0f, "b", 0.0f, NULL);
    PsyColor *rect2_color
        = g_object_new(PSY_TYPE_COLOR, "r", 1.0f, "g", 1.0f, "b", 0.0f, NULL);

    // clang-format off

    // foreground
    PsyRectangle *rect1 = g_object_new(PSY_TYPE_RECTANGLE,
                                       "canvas", g_canvas,
                                       "x", 0.f,
                                       "y", 0.f,
                                       "width", 100.0f,
                                       "height", 100.0f,
                                       "color", rect1_color,
                                       NULL);
    // background
    PsyRectangle *rect2 = g_object_new(PSY_TYPE_RECTANGLE,
                                       "canvas", g_canvas,
                                       "x", 0.f,
                                       "y", 0.f,
                                       "width", 150.0f,
                                       "height", 150.0f,
                                       "color", rect2_color,
                                       NULL);
    // clang-format on

    g_object_get(rect1, "z", &z1, NULL);
    g_object_get(rect2, "z", &z2, NULL);

    g_assert_cmpfloat(z1, ==, 0);
    g_assert_cmpfloat(z2, ==, 0);

    // The order matters of the next two is significant as both should have
    // equal z values. According to the philosophy of psylib, visual stimuli
    // that are played last are drawn on top, just like stacking playing cards,
    // the one that is played last, is drawn on top.

    psy_stimulus_play_for(PSY_STIMULUS(rect1), g_tp_start, dur);
    psy_stimulus_play_for(PSY_STIMULUS(rect2), g_tp_start, dur);

    psy_image_canvas_iterate(g_canvas);

    image = psy_canvas_get_image(PSY_CANVAS(g_canvas));
    if (save_images())
        save_image_tmp_png(image, "%s_%d.png", __func__, 1);

    PsyColor *test_color = psy_image_get_pixel(image, HEIGHT / 2, WIDTH / 2);

    g_assert_true(psy_color_equal_eps(test_color, rect2_color, 1.0 / 255));

    g_clear_object(&image);

    // Test in reverse order.

    psy_canvas_reset(PSY_CANVAS(g_canvas));
    psy_canvas_set_background_color(PSY_CANVAS(g_canvas), g_bg_color);
    g_object_unref(test_color);

    psy_stimulus_play_for(PSY_STIMULUS(rect2), g_tp_start, dur);
    psy_stimulus_play_for(PSY_STIMULUS(rect1), g_tp_start, dur);

    psy_image_canvas_iterate(g_canvas);

    image = psy_canvas_get_image(PSY_CANVAS(g_canvas));
    if (save_images())
        save_image_tmp_png(image, "%s_%d.png", __func__, 2);

    test_color = psy_image_get_pixel(image, HEIGHT / 2, WIDTH / 2);

    g_assert_true(psy_color_equal_eps(test_color, rect1_color, 1.0 / 255));

    g_object_unref(image);
    g_object_unref(test_color);
    g_object_unref(rect2);
    g_object_unref(rect1);
    g_object_unref(rect2_color);
    g_object_unref(rect1_color);
    psy_duration_free(dur);
}

static void
test_vstim_draworder_different_z(void)
{
    psy_canvas_reset(PSY_CANVAS(g_canvas));
    psy_canvas_set_background_color(PSY_CANVAS(g_canvas), g_bg_color);
    PsyImage    *image = NULL;
    PsyDuration *dur   = psy_duration_new_ms(50);

    gfloat z1 = 0, z2 = 1;
    gfloat z1out, z2out;

    PsyColor *rect1_color
        = g_object_new(PSY_TYPE_COLOR, "r", 1.0f, "g", 0.0f, "b", 0.0f, NULL);
    PsyColor *rect2_color
        = g_object_new(PSY_TYPE_COLOR, "r", 1.0f, "g", 1.0f, "b", 0.0f, NULL);

    // clang-format off

    // foreground
    PsyRectangle *rect1 = g_object_new(PSY_TYPE_RECTANGLE,
                                       "canvas", g_canvas,
                                       "x", 0.f,
                                       "y", 0.f,
                                       "z", z1,
                                       "width", 100.0f,
                                       "height", 100.0f,
                                       "color", rect1_color,
                                       NULL);
    // background
    PsyRectangle *rect2 = g_object_new(PSY_TYPE_RECTANGLE,
                                       "canvas", g_canvas,
                                       "x", 0.f,
                                       "y", 0.f,
                                       "z", z2,
                                       "width", 150.0f,
                                       "height", 150.0f,
                                       "color", rect2_color,
                                       NULL);
    // clang-format on

    g_object_get(rect1, "z", &z1out, NULL);
    g_object_get(rect2, "z", &z2out, NULL);

    g_assert_cmpfloat(z1, ==, z1out);
    g_assert_cmpfloat(z2, ==, z2out);

    // When the z-values are different, the one with the highest value
    // is "closer" to the user, and will be displayed.

    psy_stimulus_play_for(PSY_STIMULUS(rect1), g_tp_start, dur);
    psy_stimulus_play_for(PSY_STIMULUS(rect2), g_tp_start, dur);

    psy_image_canvas_iterate(g_canvas);

    image = psy_canvas_get_image(PSY_CANVAS(g_canvas));
    if (save_images())
        save_image_tmp_png(image, "%s_%d.png", __func__, 1);

    PsyColor *test_color = psy_image_get_pixel(image, HEIGHT / 2, WIDTH / 2);

    g_assert_true(psy_color_equal_eps(test_color, rect2_color, 1.0 / 255));
    g_clear_object(&image);

    psy_canvas_reset(PSY_CANVAS(g_canvas));
    psy_canvas_set_background_color(PSY_CANVAS(g_canvas), g_bg_color);
    g_object_unref(test_color);

    psy_visual_stimulus_set_z(PSY_VISUAL_STIMULUS(rect1), z2);
    psy_visual_stimulus_set_z(PSY_VISUAL_STIMULUS(rect2), z1);

    g_object_get(rect1, "z", &z1out, NULL);
    g_object_get(rect2, "z", &z2out, NULL);

    g_assert_cmpfloat(z1, ==, z2out);
    g_assert_cmpfloat(z2, ==, z1out);

    psy_stimulus_play_for(PSY_STIMULUS(rect1), g_tp_start, dur);
    psy_stimulus_play_for(PSY_STIMULUS(rect2), g_tp_start, dur);

    psy_image_canvas_iterate(g_canvas);

    image = psy_canvas_get_image(PSY_CANVAS(g_canvas));
    if (save_images())
        save_image_tmp_png(image, "%s_%d.png", __func__, 2);

    test_color = psy_image_get_pixel(image, HEIGHT / 2, WIDTH / 2);

    g_assert_true(psy_color_equal_eps(test_color, rect1_color, 1.0 / 255));

    g_object_unref(image);
    g_object_unref(test_color);
    g_object_unref(rect2);
    g_object_unref(rect1);
    g_object_unref(rect2_color);
    g_object_unref(rect1_color);
    psy_duration_free(dur);
}

int
main(int argc, char **argv)
{
    g_test_init(&argc, &argv, NULL);

    visual_stimulus_setup();

    g_test_add_func("/vstim/default_values", test_vstim_default_values);
    g_test_add_func("/vstim/scale", test_vstim_scale);
    g_test_add_func("/vstim/translate", test_vstim_translate);
    g_test_add_func("/vstim/rotate", test_vstim_rotate);
    g_test_add_func("/vstim/draworder_same_z", test_vstim_draworder_same_z);
    g_test_add_func("/vstim/draworder_different_z",
                    test_vstim_draworder_different_z);

    int ret = g_test_run();

    visual_stimulus_teardown();

    return ret;
}
