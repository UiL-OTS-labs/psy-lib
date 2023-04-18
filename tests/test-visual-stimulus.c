
#include <CUnit/CUnit.h>
#include <math.h>
#include <psylib.h>

#include "unit-test-utilities.h"

static const gint WIDTH  = 640;
static const gint HEIGHT = 480;

static PsyImageCanvas *g_canvas     = NULL;
static PsyColor       *g_stim_color = NULL;
static PsyColor       *g_bg_color   = NULL;
// a convenient start time  start + 16.67 ms otherwise stimuli are
// scheduled to a frame that has already been drawn.
static PsyTimePoint *g_tp_start     = NULL;

static int
test_visual_stimulus_setup(void)
{
    g_debug("Entering %s", __func__);
    g_canvas     = psy_image_canvas_new(WIDTH, HEIGHT);
    g_stim_color = psy_color_new_rgbi(random_int_range(0, 255),
                                      random_int_range(0, 255),
                                      random_int_range(0, 255));
    g_bg_color   = psy_color_new_rgbi(random_int_range(0, 255),
                                    random_int_range(0, 255),
                                    random_int_range(0, 255));
    g_tp_start
        = psy_time_point_add(psy_image_canvas_get_time(g_canvas),
                             psy_canvas_get_frame_dur(PSY_CANVAS(g_canvas)));

    if (!g_canvas || !g_stim_color || !g_bg_color || !g_tp_start)
        return 1;

    // make random but significantly different colors
    while (psy_color_equal_eps(g_stim_color, g_bg_color, 0.25)) {
        psy_color_set_redi(g_stim_color, random_int_range(0, 255));
        psy_color_set_greeni(g_stim_color, random_int_range(0, 255));
        psy_color_set_bluei(g_stim_color, random_int_range(0, 255));
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
    g_clear_object(&g_tp_start);

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

void
center_to_c(
    PsyImage *image, gint row_in, gint col_in, gint *row_out, gint *col_out)
{
    const gint WIDTH  = psy_image_get_width(image);
    const gint HEIGHT = psy_image_get_height(image);

    g_assert(row_in >= -HEIGHT / 2 && row_in < HEIGHT / 2);
    g_assert(col_in >= -WIDTH / 2 && col_in < WIDTH / 2);

    *col_out = WIDTH / 2 + col_in;
    *row_out = HEIGHT / 2 - row_in;

    g_assert(*row_out >= 0 && *row_out < HEIGHT);
    g_assert(*col_out >= 0 && *col_out < WIDTH);
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
    const gfloat radius       = 50;
    const gfloat num_vertices = 100;
    const gfloat scale        = (float) random_double_range(1.5, 2.5);
    PsyDuration *frame_dur    = psy_canvas_get_frame_dur(PSY_CANVAS(g_canvas));
    PsyDuration *stim_dur     = psy_duration_multiply_scalar(frame_dur, 10);
    PsyImage    *image        = NULL;

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
    psy_stimulus_play_for(PSY_STIMULUS(circle), g_tp_start, stim_dur);

    psy_image_canvas_iterate(g_canvas);

    image = psy_canvas_get_image(PSY_CANVAS(g_canvas));
    if (save_images())
        save_image_tmp_png(image, "%s-scale-%d.png", __func__, 1);

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

    if (save_images())
        save_image_tmp_png(image, "%s-scale-%d.png", __func__, 2);

    g_object_unref(image);
    g_object_unref(circle);
    g_object_unref(stim_dur);
}

void
vstim_translate(void)
{
    gfloat radius       = random_double_range(10, 20);
    guint  num_vertices = 100;

    gfloat  obtain_x, tx = random_double_range(-100, 100);
    gfloat  obtain_y, ty = random_double_range(-100, 100);
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

    CU_ASSERT_DOUBLE_EQUAL(obtain_x, tx, 1e-9);
    CU_ASSERT_DOUBLE_EQUAL(obtain_y, ty, 1e-9);

    psy_image_canvas_iterate(g_canvas);
    image = psy_canvas_get_image(PSY_CANVAS(g_canvas));

    compute_surface_avg_stim_pos(image, g_stim_color, &avg_x, &avg_y);
    avg_x = avg_x - WIDTH / 2.0;
    avg_y = avg_y - HEIGHT / 2.0;
    CU_ASSERT_DOUBLE_EQUAL(avg_x, tx, 1);
    CU_ASSERT_DOUBLE_EQUAL(avg_y, ty, 1);

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
vstim_rotate(void)
{
    psy_canvas_reset(PSY_CANVAS(g_canvas));
    psy_canvas_set_background_color(PSY_CANVAS(g_canvas), g_bg_color);

    gint   angle_0 = 0, angle_45 = 45, angle_m45 = -45;
    gfloat radians, expected = 0.0;

    PsyRectangle *rect
        = psy_rectangle_new_full(PSY_CANVAS(g_canvas), 0, 0, 10, 200);
    psy_visual_stimulus_set_color(PSY_VISUAL_STIMULUS(rect), g_stim_color);
    psy_visual_stimulus_set_rotation_deg(PSY_VISUAL_STIMULUS(rect), angle_0);
    g_object_get(rect, "rotation", &radians, NULL);
    CU_ASSERT_DOUBLE_EQUAL(radians, expected, 1e-9);

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
    center_to_c(image, 50, -50, &y1, &x1);
    center_to_c(image, 50, 0, &y2, &x2);
    center_to_c(image, 50, 50, &y3, &x3);
    center_to_c(image, 0, -50, &y4, &x4);
    center_to_c(image, 0, 0, &y5, &x5);
    center_to_c(image, 0, 50, &y6, &x6);
    center_to_c(image, -50, -50, &y7, &x7);
    center_to_c(image, -50, 0, &y8, &x8);
    center_to_c(image, -50, 50, &y9, &x9);

    PsyColor *c1 = psy_image_get_pixel(image, y1, x1);
    PsyColor *c2 = psy_image_get_pixel(image, y2, x2);
    PsyColor *c3 = psy_image_get_pixel(image, y3, x3);
    PsyColor *c4 = psy_image_get_pixel(image, y4, x4);
    PsyColor *c5 = psy_image_get_pixel(image, y5, x5);
    PsyColor *c6 = psy_image_get_pixel(image, y6, x6);
    PsyColor *c7 = psy_image_get_pixel(image, y7, x7);
    PsyColor *c8 = psy_image_get_pixel(image, y8, x8);
    PsyColor *c9 = psy_image_get_pixel(image, y9, x9);

    CU_ASSERT_FALSE(psy_color_equal_eps(g_stim_color, c1, 1.0 / 255));
    CU_ASSERT_TRUE(psy_color_equal_eps(g_stim_color, c2, 1.0 / 255));
    CU_ASSERT_FALSE(psy_color_equal_eps(g_stim_color, c3, 1.0 / 255));

    CU_ASSERT_FALSE(psy_color_equal_eps(g_stim_color, c4, 1.0 / 255));
    CU_ASSERT_TRUE(psy_color_equal_eps(g_stim_color, c5, 1.0 / 255));
    CU_ASSERT_FALSE(psy_color_equal_eps(g_stim_color, c6, 1.0 / 255));

    CU_ASSERT_FALSE(psy_color_equal_eps(g_stim_color, c7, 1.0 / 255));
    CU_ASSERT_TRUE(psy_color_equal_eps(g_stim_color, c8, 1.0 / 255));
    CU_ASSERT_FALSE(psy_color_equal_eps(g_stim_color, c9, 1.0 / 255));

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
    CU_ASSERT_DOUBLE_EQUAL(radians, expected, 1e-9);

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

    CU_ASSERT_TRUE(psy_color_equal_eps(g_stim_color, c1, 1.0 / 255));
    CU_ASSERT_FALSE(psy_color_equal_eps(g_stim_color, c2, 1.0 / 255));
    CU_ASSERT_FALSE(psy_color_equal_eps(g_stim_color, c3, 1.0 / 255));

    CU_ASSERT_FALSE(psy_color_equal_eps(g_stim_color, c4, 1.0 / 255));
    CU_ASSERT_TRUE(psy_color_equal_eps(g_stim_color, c5, 1.0 / 255));
    CU_ASSERT_FALSE(psy_color_equal_eps(g_stim_color, c6, 1.0 / 255));

    CU_ASSERT_FALSE(psy_color_equal_eps(g_stim_color, c7, 1.0 / 255));
    CU_ASSERT_FALSE(psy_color_equal_eps(g_stim_color, c8, 1.0 / 255));
    CU_ASSERT_TRUE(psy_color_equal_eps(g_stim_color, c9, 1.0 / 255));

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
    CU_ASSERT_DOUBLE_EQUAL(radians, expected, 1e-9);

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

    CU_ASSERT_FALSE(psy_color_equal_eps(g_stim_color, c1, 1.0 / 255));
    CU_ASSERT_FALSE(psy_color_equal_eps(g_stim_color, c2, 1.0 / 255));
    CU_ASSERT_TRUE(psy_color_equal_eps(g_stim_color, c3, 1.0 / 255));

    CU_ASSERT_FALSE(psy_color_equal_eps(g_stim_color, c4, 1.0 / 255));
    CU_ASSERT_TRUE(psy_color_equal_eps(g_stim_color, c5, 1.0 / 255));
    CU_ASSERT_FALSE(psy_color_equal_eps(g_stim_color, c6, 1.0 / 255));

    CU_ASSERT_TRUE(psy_color_equal_eps(g_stim_color, c7, 1.0 / 255));
    CU_ASSERT_FALSE(psy_color_equal_eps(g_stim_color, c8, 1.0 / 255));
    CU_ASSERT_FALSE(psy_color_equal_eps(g_stim_color, c9, 1.0 / 255));

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

    g_object_unref(dur);
    g_object_unref(rect);
}

#pragma GCC diagnostic push

static void
vstim_draworder_same_z(void)
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

    CU_ASSERT_DOUBLE_EQUAL(z1, 0.0f, 0);
    CU_ASSERT_DOUBLE_EQUAL(z2, 0.0f, 0);

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

    CU_ASSERT_TRUE(psy_color_equal_eps(test_color, rect2_color, 1.0 / 255));

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

    CU_ASSERT_TRUE(psy_color_equal_eps(test_color, rect1_color, 1.0 / 255));

    g_object_unref(test_color);
    g_object_unref(rect2);
    g_object_unref(rect1);
    g_object_unref(rect2_color);
    g_object_unref(rect1_color);
}

static void
vstim_draworder_different_z(void)
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

    CU_ASSERT_DOUBLE_EQUAL(z1, z1out, 0);
    CU_ASSERT_DOUBLE_EQUAL(z2, z2out, 0);

    // When the z-values are different, the one with the highest value
    // is "closer" to the user, and will be displayed.

    psy_stimulus_play_for(PSY_STIMULUS(rect1), g_tp_start, dur);
    psy_stimulus_play_for(PSY_STIMULUS(rect2), g_tp_start, dur);

    psy_image_canvas_iterate(g_canvas);

    image = psy_canvas_get_image(PSY_CANVAS(g_canvas));
    if (save_images())
        save_image_tmp_png(image, "%s_%d.png", __func__, 1);

    PsyColor *test_color = psy_image_get_pixel(image, HEIGHT / 2, WIDTH / 2);

    CU_ASSERT_TRUE(psy_color_equal_eps(test_color, rect2_color, 1.0 / 255));

    psy_canvas_reset(PSY_CANVAS(g_canvas));
    psy_canvas_set_background_color(PSY_CANVAS(g_canvas), g_bg_color);
    g_object_unref(test_color);

    psy_visual_stimulus_set_z(PSY_VISUAL_STIMULUS(rect1), z2);
    psy_visual_stimulus_set_z(PSY_VISUAL_STIMULUS(rect2), z1);

    g_object_get(rect1, "z", &z1out, NULL);
    g_object_get(rect2, "z", &z2out, NULL);

    CU_ASSERT_DOUBLE_EQUAL(z1, z2out, 0);
    CU_ASSERT_DOUBLE_EQUAL(z2, z1out, 0);

    psy_stimulus_play_for(PSY_STIMULUS(rect1), g_tp_start, dur);
    psy_stimulus_play_for(PSY_STIMULUS(rect2), g_tp_start, dur);

    psy_image_canvas_iterate(g_canvas);

    image = psy_canvas_get_image(PSY_CANVAS(g_canvas));
    if (save_images())
        save_image_tmp_png(image, "%s_%d.png", __func__, 2);

    test_color = psy_image_get_pixel(image, HEIGHT / 2, WIDTH / 2);

    CU_ASSERT_TRUE(psy_color_equal_eps(test_color, rect1_color, 1.0 / 255));

    g_object_unref(test_color);
    g_object_unref(rect2);
    g_object_unref(rect1);
    g_object_unref(rect2_color);
    g_object_unref(rect1_color);
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

    test = CU_ADD_TEST(suite, vstim_translate);
    if (!test)
        return 1;

    test = CU_ADD_TEST(suite, vstim_rotate);
    if (!test)
        return 1;

    test = CU_ADD_TEST(suite, vstim_draworder_same_z);
    if (!test)
        return 1;

    test = CU_ADD_TEST(suite, vstim_draworder_different_z);
    if (!test)
        return 1;

    return 0;
}
