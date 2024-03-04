
#include <CUnit/CUnit.h>
#include <math.h>
#include <psylib.h>

#include "unit-test-utilities.h"

static const gint WIDTH  = 640;
static const gint HEIGHT = 480;

static PsyCanvas *g_canvas     = NULL; // PsyImageCanvas
static PsyColor  *g_stim_color = NULL;
static PsyColor  *g_bg_color   = NULL;

// a convenient start time  start + 16.67 ms otherwise stimuli are
// scheduled to a frame that has already been drawn.
static PsyTimePoint *g_tp_start = NULL;

static int
visual_stimuli_setup(void)
{
    set_log_handler_file("test-visual-stimuli.txt");
    g_debug("Entering %s", __func__);
    g_canvas     = PSY_CANVAS(psy_image_canvas_new(WIDTH, HEIGHT));
    g_stim_color = psy_color_new_rgbi(random_int_range(0, 255),
                                      random_int_range(0, 255),
                                      random_int_range(0, 255));
    g_bg_color   = psy_color_new_rgbi(random_int_range(0, 255),
                                    random_int_range(0, 255),
                                    random_int_range(0, 255));

    PsyTimePoint *temp = psy_image_canvas_get_time(PSY_IMAGE_CANVAS(g_canvas));
    g_tp_start         = psy_time_point_add(
        temp, psy_canvas_get_frame_dur(PSY_CANVAS(g_canvas)));
    psy_time_point_free(temp);

    if (!g_canvas || !g_stim_color || !g_bg_color || !g_tp_start)
        return 1;

    // make random but significantly different colors
    while (psy_color_equal_eps(g_stim_color, g_bg_color, 0.25f)) {
        psy_color_set_redi(g_stim_color, random_int_range(0, 255));
        psy_color_set_greeni(g_stim_color, random_int_range(0, 255));
        psy_color_set_bluei(g_stim_color, random_int_range(0, 255));
    }

    return 0;
}

static int
visual_stimuli_teardown(void)
{
    g_debug("Entering %s", __func__);
    g_clear_object(&g_canvas);
    g_clear_object(&g_stim_color);
    g_clear_object(&g_bg_color);
    g_clear_pointer(&g_tp_start, psy_time_point_free);

    set_log_handler_file(NULL);

    return 0;
}

static void
cross_default_values(void)
{
    PsyCross *cross = psy_cross_new(g_canvas);

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

    CU_ASSERT_EQUAL(line_length_x, line_length_y);
    CU_ASSERT_EQUAL(line_length_x, 10);
    CU_ASSERT_EQUAL(line_width_x, line_width_y);
    CU_ASSERT_EQUAL(line_width_x, 3);

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

    CU_ASSERT_EQUAL(line_length_x, 10.0f);
    CU_ASSERT_EQUAL(line_length_y, 20.0f);
    CU_ASSERT_EQUAL(line_width_x, 1.0f);
    CU_ASSERT_EQUAL(line_width_y, 5.0f);

    psy_cross_free(cross);
}

static gboolean
test_cross_image(PsyImage    *image,
                 const gfloat line_length,
                 const gfloat line_width)
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

        if (psy_color_equal_eps(g_stim_color, col_length, 1.0f / 255))
            count_x_length++;
        if (psy_color_equal_eps(g_stim_color, col_width, 1.0f / 255))
            count_x_width++;

        psy_color_free(col_length);
        psy_color_free(col_width);
    }

    for (guint row = 0; row < img_height; row++) {
        PsyColor *col_length = psy_image_get_pixel(image, row, width_length);
        PsyColor *col_width  = psy_image_get_pixel(image, row, width_width);

        if (psy_color_equal_eps(g_stim_color, col_length, 1.0f / 255))
            count_y_length++;
        if (psy_color_equal_eps(g_stim_color, col_width, 1.0f / 255))
            count_y_width++;

        psy_color_free(col_length);
        psy_color_free(col_width);
    }

    if (abs(count_x_length - (int) line_length) >= 2) {
        g_warning("found length of %d along the x axis, expected: %f",
                  count_x_length,
                  line_length);
        return FALSE;
    }
    if (abs(count_y_length - (int) line_length) >= 2) {
        g_warning("found length of %d along the y axis, expected: %f",
                  count_y_length,
                  line_length);
        return FALSE;
    }
    if (abs(count_x_width - (int) line_width) >= 2) {
        g_warning("found width of %d along the x axis, expected: %f",
                  count_x_width,
                  line_width);
        return FALSE;
    }
    if (abs(count_y_width - (int) line_width) >= 2) {
        g_warning("found width of %d along the y axis, expected: %f",
                  count_y_width,
                  line_width);
        return FALSE;
    }
    return TRUE;
}

static void
cross_specific_values(void)
{
    const float length = 50, width = 10;
    PsyCross   *cross = psy_cross_new_full(g_canvas, 0, 0, length, width);
    psy_visual_stimulus_set_color(PSY_VISUAL_STIMULUS(cross), g_stim_color);
    psy_canvas_reset(g_canvas);
    psy_canvas_set_background_color(g_canvas, g_bg_color);
    psy_stimulus_play(PSY_STIMULUS(cross), g_tp_start);

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

    CU_ASSERT_EQUAL(line_length_x, line_length_y);
    CU_ASSERT_EQUAL(line_length_x, length);
    CU_ASSERT_EQUAL(line_width_x, line_width_y);
    CU_ASSERT_EQUAL(line_width_x, width);

    // Tests with pixels whether drawing is correctly
    psy_image_canvas_iterate(PSY_IMAGE_CANVAS(g_canvas));
    PsyImage *image = psy_canvas_get_image(g_canvas);

    if (save_images())
        save_image_tmp_png(image, "%s.png", __func__);

    CU_ASSERT_TRUE(test_cross_image(image, length, width));

    psy_cross_free(cross);
    psy_image_free(image);
}

int
add_visual_stimuli_suite(void)
{
    CU_Suite *suite = CU_add_suite(
        "Visual stimuli suite", visual_stimuli_setup, visual_stimuli_teardown);

    CU_Test *test = NULL;

    if (!suite)
        return 1;

    test = CU_ADD_TEST(suite, cross_default_values);
    if (!test)
        return 1;

    test = CU_ADD_TEST(suite, cross_specific_values);
    if (!test)
        return 1;

    return 0;
}
