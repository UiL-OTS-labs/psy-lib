
#include <CUnit/CUnit.h>
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
test_text_setup(void)
{
    g_debug("Entering %s", __func__);
    g_canvas     = psy_image_canvas_new(WIDTH, HEIGHT);
    g_stim_color = psy_color_new_rgbi(random_int_range(0, 255),
                                      random_int_range(0, 255),
                                      random_int_range(0, 255));
    g_bg_color   = psy_color_new_rgbi(random_int_range(0, 255),
                                    random_int_range(0, 255),
                                    random_int_range(0, 255));

    PsyTimePoint *temp = psy_image_canvas_get_time(g_canvas);
    g_tp_start         = psy_time_point_add(
        temp, psy_canvas_get_frame_dur(PSY_CANVAS(g_canvas)));
    psy_time_point_free(temp);

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
test_text_teardown(void)
{
    g_debug("Entering %s", __func__);
    g_clear_object(&g_canvas);
#pragma message "_free colors"
    g_clear_object(&g_stim_color);
    g_clear_object(&g_bg_color);
    g_clear_pointer(&g_tp_start, psy_time_point_free);

    return 0;
}

static void
text_default_values(void)
{
    PsyText *text = psy_text_new(PSY_CANVAS(g_canvas));

    CU_ASSERT_PTR_NOT_NULL_FATAL(text);

    PsyColor *font_color         = NULL;
    PsyColor *background_color   = NULL;
    PsyColor *default_bg_color   = psy_color_new();
    PsyColor *default_font_color = psy_color_new_rgb(1.0, 1, 1);
    gboolean  is_dirty, use_markup;

    // clang-format off
    g_object_get(text,
            "color", &background_color,
            "font-color", &font_color,
            "is-dirty", &is_dirty,
            "use-markup", &use_markup,
             NULL);
    // clang-format on

    CU_ASSERT_TRUE(is_dirty); // if it isn't drawn, it's dirty
    CU_ASSERT_FALSE(use_markup);
    CU_ASSERT_TRUE(psy_color_equal(default_bg_color, background_color));
    CU_ASSERT_TRUE(psy_color_equal(default_font_color, font_color));

    g_object_unref(text);
    g_object_unref(font_color);
    g_object_unref(background_color);
    g_object_unref(default_font_color);
    g_object_unref(default_bg_color);
}

static void
text_markup_text_properties(void)
{
    PsyText *text
        = psy_text_new_full(PSY_CANVAS(g_canvas), 0, 0, 640, 480, "", TRUE);
    gboolean     use_markup;
    const gchar *markup    = "<markup>Some text</markup>";
    const gchar *some_text = "Some text";

    g_object_set(text, "markup", markup, NULL);
    g_object_get(text, "use-markup", &use_markup, NULL);

    CU_ASSERT_TRUE(use_markup);

    g_object_set(text, "text", some_text, NULL);
    g_object_get(text, "use-markup", &use_markup, NULL);

    CU_ASSERT_FALSE(use_markup);

    g_object_unref(text);
}

int
add_text_suite(void)
{
    CU_Suite *suite
        = CU_add_suite("Text suite", test_text_setup, test_text_teardown);

    CU_Test *test = NULL;

    if (!suite)
        return 1;

    test = CU_ADD_TEST(suite, text_default_values);
    if (!test)
        return 1;

    test = CU_ADD_TEST(suite, text_markup_text_properties);
    if (!test)
        return 1;

    return 0;
}
