
#include <criterion/criterion.h>
#include <criterion/internal/test.h>
#include <criterion/new/assert.h>
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

static void
test_text_setup(void)
{
    init_random();
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

    if (!g_canvas || !g_stim_color || !g_bg_color || !g_tp_start) {
        cr_log_warn("Unable to create a critical object for test-text");
        return;
    }

    // make random but significantly different colors
    while (psy_color_equal_eps(g_stim_color, g_bg_color, 0.25)) {
        psy_color_set_redi(g_stim_color, random_int_range(0, 255));
        psy_color_set_greeni(g_stim_color, random_int_range(0, 255));
        psy_color_set_bluei(g_stim_color, random_int_range(0, 255));
    }
}

static void
test_text_teardown(void)
{
    g_debug("Entering %s", __func__);
    g_clear_object(&g_canvas);
    g_clear_object(&g_stim_color);
    g_clear_object(&g_bg_color);
    g_clear_pointer(&g_tp_start, psy_time_point_free);

    deinitialize_random();
}

TestSuite(text, .init = test_text_setup, .fini = test_text_teardown);

Test(text, default_values)
{
    PsyText *text = psy_text_new(PSY_CANVAS(g_canvas));

    cr_assert(ne(text, NULL));

    PsyColor *font_color         = NULL;
    PsyColor *background_color   = NULL;
    PsyColor *default_bg_color   = psy_color_new();
    PsyColor *default_font_color = psy_color_new_rgb(1.0f, 1.0f, 1.0f);
    gboolean  is_dirty, use_markup;

    // clang-format off
    g_object_get(text,
            "color", &background_color,
            "font-color", &font_color,
            "is-dirty", &is_dirty,
            "use-markup", &use_markup,
             NULL);
    // clang-format on

    cr_expect(eq(is_dirty, TRUE)); // if it isn't drawn, it's dirty
    cr_expect(eq(use_markup, FALSE));
    cr_expect(eq(psy_color_equal(default_bg_color, background_color), TRUE));
    cr_expect(eq(psy_color_equal(default_font_color, font_color), TRUE));

    g_object_unref(text);
    g_object_unref(font_color);
    g_object_unref(background_color);
    g_object_unref(default_font_color);
    g_object_unref(default_bg_color);
}

Test(text, markup_text_properties)
{
    PsyText *text
        = psy_text_new_full(PSY_CANVAS(g_canvas), 0, 0, 640, 480, "", TRUE);
    gboolean     use_markup;
    const gchar *markup    = "<markup>Some text</markup>";
    const gchar *some_text = "Some text";

    g_object_set(text, "markup", markup, NULL);
    g_object_get(text, "use-markup", &use_markup, NULL);

    cr_assert(
        eq(use_markup, TRUE),
        "When setting the markup property it is expected that markup is used");

    g_object_set(text, "text", some_text, NULL);
    g_object_get(text, "use-markup", &use_markup, NULL);

    cr_expect(not(use_markup),
              "when setting the text property, it is expected that markup "
              "isn't used");

    g_object_unref(text);
}
