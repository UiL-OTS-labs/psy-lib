#include <psylib.h>

static void
test_color_default_values(void)
{
    PsyColor *color = psy_color_new();
    gfloat    rf, gf, bf, af;
    gint      ri, gi, bi, ai;

    // clang-format off
    g_object_get(color,
            "r", &rf,
            "g", &gf,
            "b", &bf,
            "a", &af,
            "ri", &ri,
            "gi", &gi,
            "bi", &bi,
            "ai", &ai,
            NULL
            );
    // clang-format on

    // Default floating point values should be 0, except alpha channel which
    // should be 1.0.
    g_assert_cmpfloat(rf, ==, 0.0);
    g_assert_cmpfloat(gf, ==, 0.0);
    g_assert_cmpfloat(bf, ==, 0.0);
    g_assert_cmpfloat(af, ==, 1.0);

    // Default integer values should be 0, except alpha channel which
    // should be 255.
    g_assert_cmpint(ri, ==, 0);
    g_assert_cmpint(gi, ==, 0);
    g_assert_cmpint(bi, ==, 0);
    g_assert_cmpint(ai, ==, 255);

    g_object_unref(color);
}

static void
test_color_specific_rgb_values(void)
{
    gfloat       rf, gf, bf, af;
    gint         ri, gi, bi, ai;
    const gfloat red = 1.0f, green = 0.5f, blue = .25f, alpha = 0.5f;
    gint         max_color = 255;

    PsyColor *color = psy_color_new_rgba(red, green, blue, alpha);

    // clang-format off
    g_object_get(color,
            "r", &rf,
            "g", &gf,
            "b", &bf,
            "a", &af,
            "ri", &ri,
            "gi", &gi,
            "bi", &bi,
            "ai", &ai,
            NULL
            );
    // clang-format on

    g_assert_cmpfloat(rf, ==, 1.0);
    g_assert_cmpfloat(gf, ==, 0.5);
    g_assert_cmpfloat(bf, ==, 0.25);
    g_assert_cmpfloat(af, ==, 0.5);

    g_assert_cmpint(ri, ==, (int) (red * max_color));
    g_assert_cmpint(gi, ==, (int) (green * max_color));
    g_assert_cmpint(bi, ==, (int) (blue * max_color));
    g_assert_cmpint(ai, ==, (int) (alpha * max_color));

    g_object_unref(color);
}

static void
test_color_specific_rgbi_values(void)
{
    gfloat     rf, gf, bf, af;
    gint       ri, gi, bi, ai;
    const gint red = 0, green = 2, blue = 3, alpha = 4;
    gfloat     max_color = 255;

    PsyColor *color = psy_color_new_rgbai(red, green, blue, alpha);

    // clang-format off
    g_object_get(color,
            "r", &rf,
            "g", &gf,
            "b", &bf,
            "a", &af,
            "ri", &ri,
            "gi", &gi,
            "bi", &bi,
            "ai", &ai,
            NULL
            );
    // clang-format on

    gfloat epsilon = 1e-6f;

    g_assert_cmpfloat_with_epsilon(rf, red / max_color, epsilon);
    g_assert_cmpfloat_with_epsilon(gf, green / max_color, epsilon);
    g_assert_cmpfloat_with_epsilon(bf, blue / max_color, epsilon);
    g_assert_cmpfloat_with_epsilon(af, alpha / max_color, epsilon);

    g_assert_cmpint(ri, ==, red);
    g_assert_cmpint(gi, ==, green);
    g_assert_cmpint(bi, ==, blue);
    g_assert_cmpint(ai, ==, alpha);

    g_object_unref(color);
}

int
main(int argc, char **argv)
{
    g_test_init(&argc, &argv, NULL);

    g_test_add_func("/color/test_default_values", test_color_default_values);
    g_test_add_func("/color/specific_rgb_values",
                    test_color_specific_rgb_values);
    g_test_add_func("/color/specific_rgbi_values",
                    test_color_specific_rgbi_values);

    return g_test_run();
}
