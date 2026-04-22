#include <criterion/criterion.h>
#include <criterion/new/assert.h>
#include <psy-color.h>

Test(color, default_values)
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
    cr_expect(eq(rf, 0.0));
    cr_expect(eq(gf, 0.0));
    cr_expect(eq(bf, 0.0));
    cr_expect(eq(af, 1.0));

    // Default integer values should be 0, except alpha channel which
    // should be 255.
    cr_expect(eq(ri, 0));
    cr_expect(eq(gi, 0));
    cr_expect(eq(bi, 0));
    cr_expect(eq(ai, 255));

    g_object_unref(color);
}

Test(color, specific_rgb_values)
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

    cr_expect(eq(rf, 1.0));
    cr_expect(eq(gf, 0.5));
    cr_expect(eq(bf, 0.25));
    cr_expect(eq(af, 0.5));

    cr_expect(eq(ri, (int) (red * max_color)));
    cr_expect(eq(gi, (int) (green * max_color)));
    cr_expect(eq(bi, (int) (blue * max_color)));
    cr_expect(eq(ai, (int) (alpha * max_color)));

    g_object_unref(color);
}

Test(color, specific_rgbi_values)
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

    cr_expect(epsilon_eq(rf, red / max_color, epsilon));
    cr_expect(epsilon_eq(gf, green / max_color, epsilon));
    cr_expect(epsilon_eq(bf, blue / max_color, epsilon));
    cr_expect(epsilon_eq(af, alpha / max_color, epsilon));

    cr_expect(eq(ri, red));
    cr_expect(eq(gi, green));
    cr_expect(eq(bi, blue));
    cr_expect(eq(ai, alpha));

    g_object_unref(color);
}
