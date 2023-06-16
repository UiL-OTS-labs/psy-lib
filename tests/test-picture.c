
#include <CUnit/CUnit.h>
#include <psy-image-canvas.h>
#include <psy-picture.h>
#include <stdbool.h>

#include "unit-test-utilities.h"

const guint g_img_width     = 300;
const guint g_img_height    = 200;
const guint g_canvas_width  = 600;
const guint g_canvas_height = 400;

static PsyImage       *g_image;
static PsyImageCanvas *g_canvas;
static PsyTimePoint   *g_tstart;
static char           *g_path;

static bool
is_upper_left(guint width, guint height, guint row, guint col)
{
    return row < height / 2 && col < width / 2;
}

static bool
is_upper_right(guint width, guint height, guint row, guint col)
{
    return row < height / 2 && col >= width / 2;
}

static bool
is_lower_left(guint width, guint height, guint row, guint col)
{
    return row >= height / 2 && col < width / 2;
}

static bool
is_lower_right(guint width, guint height, guint row, guint col)
{
    return row >= height / 2 && col >= width / 2;
}

static void
init_squares(PsyImage *image)
{
    // Creates an image that roughly looks like:
    /************************
     *           |          *
     *   red     |   green  *
     * 0xff0000  | 0x00ff00 *
     *           |          *
     *  ---------+--------  *
     *           |          *
     *  blue     |  purple  *
     * 0x0000ff  | 0xff00ff *
     *           |          *
     ************************/
    guint w      = psy_image_get_width(image);
    guint h      = psy_image_get_height(image);
    guint stride = psy_image_get_stride(image);
    guint nc     = psy_image_get_num_channels(image);

    guint8 *bytes = psy_image_get_ptr(image);
    for (guint row = 0; row < h; row++) {
        guint8 *line = bytes + row * stride;
        for (guint col = 0; col < w; col++) {
            guint8 *pixel = line + col * nc;
            if (is_upper_left(g_img_width, g_img_height, row, col)) {
                pixel[0] = 0xff;
                pixel[1] = 0x00;
                pixel[2] = 0x00;
            }
            else if (is_upper_right(g_img_width, g_img_height, row, col)) {
                pixel[0] = 0x00;
                pixel[1] = 0xff;
                pixel[2] = 0x00;
            }
            else if (is_lower_left(g_img_width, g_img_height, row, col)) {
                pixel[0] = 0x00;
                pixel[1] = 0x00;
                pixel[2] = 0xff;
            }
            else if (is_lower_right(g_img_width, g_img_height, row, col)) {
                pixel[0] = 0xff;
                pixel[1] = 0x00;
                pixel[2] = 0xff;
            }
            else {
                g_assert_not_reached();
            }
        }
    }
}

static int
picture_setup(void)
{
    GError *error = NULL;
    g_image  = psy_image_new(g_img_width, g_img_height, PSY_IMAGE_FORMAT_RGB);
    g_canvas = psy_image_canvas_new(g_canvas_width, g_canvas_height);
    PsyColor *bg = psy_color_new_rgbi(128, 128, 128);
    psy_canvas_set_background_color(PSY_CANVAS(g_canvas), bg);
    g_object_unref(bg);

    PsyDrawingContext *context = psy_canvas_get_context(PSY_CANVAS(g_canvas));

    PsyTimePoint *tnull = psy_time_point_new();
    g_tstart            = psy_time_point_add(
        tnull, psy_canvas_get_frame_dur(PSY_CANVAS(g_canvas)));
    g_object_unref(tnull);

    g_path = g_build_filename(g_get_tmp_dir(), "quarters.png", NULL);

    init_squares(g_image);

    if (save_images()) {
        save_image_tmp_png(g_image, "%s-%s.png", __func__, "quarters");
    }

    psy_image_save_path(g_image, g_path, "png", &error);

    psy_drawing_context_load_files_as_texture(context, &g_path, 1, &error);

    return g_image && g_canvas && g_path ? 0 : 1;
}

static int
picture_teardown(void)
{
    GError *error = NULL;

    g_clear_object(&g_image);
    g_clear_object(&g_canvas);
    g_clear_object(&g_tstart);

    GFile *file = g_file_new_for_path(g_path);

    g_file_delete(file, NULL, &error);
    if (error) {
        g_printerr("Unable to delete %s:%s\n", g_path, error->message);
    }
    g_object_unref(file);
    g_free(g_path);

    return 0;
}

static void
picture_default_values(void)
{
    PsyPicture *picture = psy_picture_new(PSY_CANVAS(g_canvas));

    PsyPictureSizeStrategy strategy;
    gchar                 *filename = NULL;

    // clang-format off
    g_object_get(
            picture,
            "filename", &filename,
            "size-strategy", &strategy,
            NULL);
    // clang-format on
    CU_ASSERT_EQUAL(strategy, PSY_PICTURE_STRATEGY_AUTOMATIC);
    CU_ASSERT_EQUAL(filename, NULL);

    g_object_unref(picture);
}

static void
picture_with_filename(void)
{
    PsyPicture *picture
        = psy_picture_new_filename(PSY_CANVAS(g_canvas), g_path);

    PsyPictureSizeStrategy strategy;
    gchar                 *filename = NULL;

    // clang-format off
    g_object_get(
            picture,
            "filename", &filename,
            "size-strategy", &strategy,
            NULL);
    // clang-format on
    CU_ASSERT_EQUAL(strategy, PSY_PICTURE_STRATEGY_AUTOMATIC);
    CU_ASSERT_STRING_EQUAL(filename, g_path);

    g_free(filename);
    g_object_unref(picture);
}

static void
on_resize(PsyPicture *picture, gfloat width, gfloat height, gpointer data)
{
    gboolean *set_true = data;

    (void) picture;
    (void) width;
    (void) height;

    *set_true = TRUE;
}

static void
picture_draw_auto_resize(void)
{
    PsyPictureSizeStrategy strat_static;
    PsyPictureSizeStrategy strat_dynamic;

    bool static_resized = FALSE, dynamic_resized = FALSE;

    PsyPicture *pic_static
        = psy_picture_new_full(PSY_CANVAS(g_canvas), 0, 0, 100, 100, g_path);
    PsyPicture *pic_dynamic
        = psy_picture_new_filename(PSY_CANVAS(g_canvas), g_path);

    g_object_get(pic_static, "size-strategy", &strat_static, NULL);
    g_object_get(pic_dynamic, "size-strategy", &strat_dynamic, NULL);

    CU_ASSERT_EQUAL(strat_static, PSY_PICTURE_STRATEGY_MANUAL);
    CU_ASSERT_EQUAL(strat_dynamic, PSY_PICTURE_STRATEGY_AUTOMATIC);

    g_signal_connect(
        pic_static, "auto-resize", G_CALLBACK(on_resize), &static_resized);
    g_signal_connect(
        pic_dynamic, "auto-resize", G_CALLBACK(on_resize), &dynamic_resized);

    psy_stimulus_play(PSY_STIMULUS(pic_static), g_tstart);
    psy_stimulus_play(PSY_STIMULUS(pic_dynamic), g_tstart);

    psy_image_canvas_iterate(g_canvas);

    CU_ASSERT_TRUE(dynamic_resized);
    CU_ASSERT_FALSE(static_resized);

    g_object_unref(pic_static);
    g_object_unref(pic_dynamic);
}

/*
 * Sample 100 points, determine whether they have the intended color
 *
 * The coordinates between -150 <= x < 150 and 100 > y >= -100 should
 * intersect with the image, the outside of this region, should be the default
 * canvas background color.
 */
static bool
test_colors(PsyImage *image)
{
    gboolean ret = TRUE;
    g_assert(psy_image_get_width(image) == g_canvas_width);
    g_assert(psy_image_get_height(image) == g_canvas_height);

    PsyColor *red    = psy_color_new_rgbi(255, 0, 0);
    PsyColor *green  = psy_color_new_rgbi(0, 255, 0);
    PsyColor *blue   = psy_color_new_rgbi(0, 0, 255);
    PsyColor *purple = psy_color_new_rgbi(255, 0, 255);

    // owned by canvas
    PsyColor *bg = psy_canvas_get_background_color(PSY_CANVAS(g_canvas));

    guint num_samples = 0;

    // Bounds of image inside of the canvas
    guint left_bound  = (g_canvas_width - g_img_width) / 2;
    guint right_bound = g_canvas_width - (g_canvas_width - g_img_width) / 2;
    guint upper_bound = (g_canvas_height - g_img_height) / 2;
    guint lower_bound = g_canvas_height - (g_canvas_height - g_img_height) / 2;

    while (num_samples < 10) {
        guint x = random_int_range(0, g_canvas_width - 1);
        guint y = random_int_range(0, g_canvas_height - 1);

        PsyColor *probe = psy_image_get_pixel(image, y, x);

        if (x < left_bound || x >= right_bound || y < upper_bound
            || y >= lower_bound) {
            // We should be outside the image
            if (!psy_color_equal(bg, probe)) {
                ret = FALSE;
                g_printerr(
                    "point (%u, %u) is not equal to the background.\n", x, y);
            }
        }
        else {
            // convert coordinate to match the image.
            guint ix = x - (g_canvas_width - g_img_width) / 2;
            guint iy = y - (g_canvas_height - g_img_height) / 2;

            // g_print("x = %u, y = %u, ix = %u, iy = %u\n", x, y, ix, iy);

            g_assert(ix < g_img_width);
            g_assert(iy < g_img_height);

            if (is_upper_left(g_img_width, g_img_height, iy, ix)) {
                if (!psy_color_equal(red, probe)) {
                    ret = FALSE;
                    g_printerr(
                        "point g(%u, %u), i(%u, %u) is not equal to red.\n",
                        x,
                        y,
                        ix,
                        iy);
                }
            }
            else if (is_upper_right(g_img_width, g_img_height, iy, ix)) {
                if (!psy_color_equal(green, probe)) {
                    ret = FALSE;
                    g_printerr(
                        "point g(%u, %u), i(%u, %u) is not equal to green.\n",
                        x,
                        y,
                        ix,
                        iy);
                }
            }
            else if (is_lower_left(g_img_width, g_img_height, iy, ix)) {
                if (!psy_color_equal(blue, probe)) {
                    ret = FALSE;
                    g_printerr(
                        "point g(%u, %u), i(%u, %u) is not equal to blue.\n",
                        x,
                        y,
                        ix,
                        iy);
                }
            }
            else if (is_lower_right(g_img_width, g_img_height, iy, ix)) {
                if (!psy_color_equal(purple, probe)) {
                    ret = FALSE;
                    g_printerr(
                        "point g(%u, %u), i(%u, %u) is not equal to purple.\n",
                        x,
                        y,
                        ix,
                        iy);
                }
            }
            else {
                g_printerr("No sensible coordinate x=%u, y=%u; ix=%u, iy=%u\n",
                           x,
                           y,
                           ix,
                           iy);
                g_assert_not_reached();
            }
        }

        g_object_unref(probe);
        num_samples++;
    }
    g_object_unref(red);
    g_object_unref(green);
    g_object_unref(blue);
    g_object_unref(purple);

    return ret;
}

/*
 * The main goal of this test is to see whether the image is into it's place
 * and not upside down, left side right.
 */
static void
picture_test_drawing(void)
{
    PsyPicture *pic = psy_picture_new_filename(PSY_CANVAS(g_canvas), g_path);
    PsyImage   *image;

    psy_stimulus_play(PSY_STIMULUS(pic), g_tstart);

    psy_image_canvas_iterate(g_canvas);

    image = psy_canvas_get_image(PSY_CANVAS(g_canvas));
    if (save_images()) {
        save_image_tmp_png(image, "%s.png", __func__);
    }

    CU_ASSERT_TRUE(test_colors(image));

    g_object_unref(pic);
    g_object_unref(image);
}

int
add_picture_suite(void)
{
    CU_Suite *suite
        = CU_add_suite("picture tests", picture_setup, picture_teardown);
    CU_Test *test = NULL;

    if (!suite)
        return 1;

    test = CU_ADD_TEST(suite, picture_default_values);
    if (!test)
        return 1;

    test = CU_ADD_TEST(suite, picture_with_filename);
    if (!test)
        return 1;

    test = CU_ADD_TEST(suite, picture_draw_auto_resize);
    if (!test)
        return 1;

    test = CU_ADD_TEST(suite, picture_test_drawing);
    if (!test)
        return 1;

    return 0;
}
