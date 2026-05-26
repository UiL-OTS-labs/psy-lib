
#include <math.h>
#include <psylib.h>

#include "psy-init.h"
#include "unit-test-utilities.h"

typedef struct CrossFixture {
    PsyCanvas    *canvas; // PsyImageCanvas
    PsyColor     *stim_color;
    PsyColor     *bg_color;
    PsyTimePoint *tp_start;
} CrossFixture;

static const gint WIDTH  = 640;
static const gint HEIGHT = 480;

static void
cross_setup(CrossFixture *fix, gconstpointer unused)
{
    (void) unused;
    g_debug("Entering %s", __func__);
    fix->canvas     = PSY_CANVAS(psy_image_canvas_new(WIDTH, HEIGHT));
    fix->stim_color = psy_color_new_rgbi(g_test_rand_int_range(0, 255),
                                         g_test_rand_int_range(0, 255),
                                         g_test_rand_int_range(0, 255));
    fix->bg_color   = psy_color_new_rgbi(g_test_rand_int_range(0, 255),
                                       g_test_rand_int_range(0, 255),
                                       g_test_rand_int_range(0, 255));

    PsyTimePoint *temp
        = psy_image_canvas_get_time(PSY_IMAGE_CANVAS(fix->canvas));
    fix->tp_start = psy_time_point_add(
        temp, psy_canvas_get_frame_dur(PSY_CANVAS(fix->canvas)));
    psy_time_point_free(temp);

    if (!fix->canvas || !fix->stim_color || !fix->bg_color || !fix->tp_start)
        g_error("Unable to setup a cross test");

    // make random but significantly different colors
    while (psy_color_equal_eps(fix->stim_color, fix->bg_color, 0.25f)) {
        psy_color_set_redi(fix->stim_color, g_test_rand_int_range(0, 255));
        psy_color_set_greeni(fix->stim_color, g_test_rand_int_range(0, 255));
        psy_color_set_bluei(fix->stim_color, g_test_rand_int_range(0, 255));
    }
}

static void
cross_teardown(CrossFixture *fix, gconstpointer unused)
{
    (void) unused;
    g_debug("Entering %s", __func__);
    g_clear_object(&fix->canvas);
    g_clear_object(&fix->stim_color);
    g_clear_object(&fix->bg_color);
    g_clear_pointer(&fix->tp_start, psy_time_point_free);
}

static void
test_cross_default_values(CrossFixture *fix, gconstpointer unused)
{
    (void) unused;
    PsyCross *cross = psy_cross_new(fix->canvas);

    gfloat line_length_x, line_length_y;
    gfloat line_width_x, line_width_y;

    // clang-format off
    g_object_get(cross,
            "line-length-x", &line_length_x,
            "line-length-y", &line_length_y,
            "line-width-x", &line_width_x,
            "line-width-y", &line_width_y,
            NULL);
    // clang-format on

    g_assert_cmpfloat(line_length_x, ==, line_length_y);
    g_assert_cmpfloat(line_length_x, ==, 10);
    g_assert_cmpfloat(line_width_x, ==, line_width_y);
    g_assert_cmpfloat(line_width_x, ==, 3);

    // clang-format off
    g_object_set(cross,
            "line-length-x", 10.0f,
            "line-length-y", 20.0f,
            "line-width-x", 1.0f,
            "line-width-y", 5.0f,
            NULL);
    
    g_object_get(cross,
            "line-length-x", &line_length_x,
            "line-length-y", &line_length_y,
            "line-width-x", &line_width_x,
            "line-width-y", &line_width_y,
            NULL);
    // clang-format on

    g_assert_cmpfloat(line_length_x, ==, 10.0f);
    g_assert_cmpfloat(line_length_y, ==, 20.0f);
    g_assert_cmpfloat(line_width_x, ==, 1.0f);
    g_assert_cmpfloat(line_width_y, ==, 5.0f);

    psy_cross_free(cross);
}

static gboolean
test_cross_image(PsyImage           *image,
                 const CrossFixture *fix,
                 const gfloat        line_length,
                 const gfloat        line_width)
{
    const guint img_width  = psy_image_get_width(image);
    const guint img_height = psy_image_get_height(image);

    gint count_x_length = 0;
    gint count_y_length = 0;
    gint count_x_width  = 0;
    gint count_y_width  = 0;

    guint height_length = img_height / 2;
    guint height_width  = img_height / 2 + line_width;
    guint width_length  = img_width / 2;
    guint width_width   = img_width / 2 + line_width;

    for (guint col = 0; col < img_width; col++) {
        PsyColor *col_length = psy_image_get_pixel(image, height_length, col);
        PsyColor *col_width  = psy_image_get_pixel(image, height_width, col);

        if (psy_color_equal_eps(fix->stim_color, col_length, 1.0f / 255))
            count_x_length++;
        if (psy_color_equal_eps(fix->stim_color, col_width, 1.0f / 255))
            count_x_width++;

        psy_color_free(col_length);
        psy_color_free(col_width);
    }

    for (guint row = 0; row < img_height; row++) {
        PsyColor *col_length = psy_image_get_pixel(image, row, width_length);
        PsyColor *col_width  = psy_image_get_pixel(image, row, width_width);

        if (psy_color_equal_eps(fix->stim_color, col_length, 1.0f / 255))
            count_y_length++;
        if (psy_color_equal_eps(fix->stim_color, col_width, 1.0f / 255))
            count_y_width++;

        psy_color_free(col_length);
        psy_color_free(col_width);
    }

    if (abs(count_x_length - (int) line_length) >= 2) {
        g_message("found length of %d along the x axis, expected: %f",
                  count_x_length,
                  line_length);
        return FALSE;
    }
    if (abs(count_y_length - (int) line_length) >= 2) {
        g_message("found length of %d along the y axis, expected: %f",
                  count_y_length,
                  line_length);
        return FALSE;
    }
    if (abs(count_x_width - (int) line_width) >= 2) {
        g_message("found width of %d along the x axis, expected: %f",
                  count_x_width,
                  line_width);
        return FALSE;
    }
    if (abs(count_y_width - (int) line_width) >= 2) {
        g_message("found width of %d along the y axis, expected: %f",
                  count_y_width,
                  line_width);
        return FALSE;
    }
    return TRUE;
}

static void
test_cross_specific_values(CrossFixture *fix, gconstpointer unused)
{
    (void) unused;
    const float length = 50, width = 10;
    PsyCross   *cross = psy_cross_new_full(fix->canvas, 0, 0, length, width);
    psy_visual_stimulus_set_color(PSY_VISUAL_STIMULUS(cross), fix->stim_color);
    psy_canvas_reset(fix->canvas);
    psy_canvas_set_background_color(fix->canvas, fix->bg_color);
    psy_stimulus_play(PSY_STIMULUS(cross), fix->tp_start);

    gfloat line_length_x, line_length_y;
    gfloat line_width_x, line_width_y;

    // clang-format off
    g_object_get(cross,
            "line-length-x", &line_length_x,
            "line-length-y", &line_length_y,
            "line-width-x", &line_width_x,
            "line-width-y", &line_width_y,
            NULL);
    // clang-format on

    g_assert_cmpfloat(line_length_x, ==, line_length_y);
    g_assert_cmpfloat(line_length_x, ==, length);
    g_assert_cmpfloat(line_width_x, ==, line_width_y);
    g_assert_cmpfloat(line_width_x, ==, width);

    // Tests with pixels whether drawing is correctly
    psy_image_canvas_iterate(PSY_IMAGE_CANVAS(fix->canvas));
    PsyImage *image = psy_canvas_get_image(fix->canvas);

    if (save_images())
        save_image_tmp_png(image, "%s.png", __func__);

    g_assert_true(test_cross_image(image, fix, length, width));

    psy_cross_free(cross);
    psy_image_free(image);
}

int
main(int argc, char **argv)
{
    g_test_init(&argc, &argv, NULL);

    UnitTestUtilsInit init_utils = {.log_file      = "test-cross.txt",
                                    .domains       = NULL,
                                    .log_level     = G_LOG_LEVEL_INFO,
                                    .save_pictures = TRUE};

    unit_test_utils_init(&init_utils);

    PsyInitializer *init = g_object_new(
        PSY_TYPE_INITIALIZER, "portaudio", FALSE, "gstreamer", FALSE, NULL);

    g_test_add("/cross/default_values",
               CrossFixture,
               NULL,
               cross_setup,
               test_cross_default_values,
               cross_teardown);

    g_test_add("/cross/test_specific_values",
               CrossFixture,
               NULL,
               cross_setup,
               test_cross_specific_values,
               cross_teardown);

    int save = g_test_run();

    g_object_unref(init);
    init = NULL;

    return save;
}
