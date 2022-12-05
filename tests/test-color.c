
#include <mutest.h> 
#include <psy-color.h>


static void
color_default_values(void)
{
    PsyColor* color = psy_color_new();
    gfloat rf, gf, bf, af;
    gint ri, gi, bi, ai;

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

    mutest_expect ("r to be 0.0", mutest_float_value(rf),
            mutest_to_be, 0.0, NULL);
    mutest_expect ("g to be 0.0", mutest_float_value(gf),
            mutest_to_be, 0.0, NULL);
    mutest_expect ("b to be 0.0", mutest_float_value(bf),
            mutest_to_be, 0.0, NULL);
    mutest_expect ("a to be 1.0", mutest_float_value(af),
            mutest_to_be, 1.0, NULL);
    
    mutest_expect ("ri to be 0", mutest_int_value(ri),
            mutest_to_be, 0, NULL);
    mutest_expect ("gi to be 0", mutest_int_value(bi),
            mutest_to_be, 0, NULL);
    mutest_expect ("bi to be 0", mutest_int_value(gi),
            mutest_to_be, 0, NULL);
    mutest_expect ("ai to be 255", mutest_int_value(ai),
            mutest_to_be, 254, NULL);

    g_object_unref(color);
}

static void
color_specific_rgb_values(void)
{
    gfloat rf, gf, bf, af;
    gint ri, gi, bi, ai;
    const gfloat red = 1.0, green = 0.5, blue = .25, alpha = 0.5;
    gint max_color = 255;

    PsyColor* color = psy_color_new_rgba(red, green, blue, alpha);

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

    mutest_expect ("r to be 1.0", mutest_float_value(rf),
            mutest_to_be, red, NULL);
    mutest_expect ("g to be 0.5", mutest_float_value(gf),
            mutest_to_be, green, NULL);
    mutest_expect ("b to be 0.25", mutest_float_value(bf),
            mutest_to_be, blue, NULL);
    mutest_expect ("a to be .5", mutest_float_value(af),
            mutest_to_be, alpha, NULL);
    
    mutest_expect ("ri to be valid", mutest_int_value(ri),
            mutest_to_be, ((int)(red * max_color)), NULL);
    mutest_expect ("gi to be valid", mutest_int_value(gi),
            mutest_to_be, ((int)(green * max_color)), NULL);
    mutest_expect ("bi to be 0",mutest_int_value(bi),
            mutest_to_be, ((int)(blue * max_color)), NULL);
    mutest_expect ("ai to be 255", mutest_int_value(ai),
            mutest_to_be, ((int)(alpha * max_color)), NULL);

    g_object_unref(color);
}

static void
color_specific_rgbi_values(void)
{
    gfloat rf, gf, bf, af;
    gint ri, gi, bi, ai;
    const gint red = 0, green = 2, blue = 3, alpha = 4;
    gfloat max_color = 255;

    PsyColor* color = psy_color_new_rgbai(red, green, blue, alpha);

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

    gfloat epsilon = 1e-6;

    mutest_expect ("r to be close to0/255",
            mutest_float_value(rf), mutest_to_be_close_to, red / max_color, epsilon,
            NULL);
    mutest_expect (
            "g to be close to  2/255",
            mutest_float_value(gf), mutest_to_be_close_to, green / max_color, epsilon,
            NULL);
    mutest_expect (
            "b to be close to 3/255",
            mutest_float_value(bf), mutest_to_be_close_to, blue / max_color, epsilon,
            NULL);
    mutest_expect ("a to be close to 4/255",
            mutest_float_value(af), mutest_to_be_close_to, alpha / max_color, epsilon,
            NULL);
    
    mutest_expect ("ri to be 0", mutest_int_value(ri),
            mutest_to_be, red, NULL);
    mutest_expect ("gi to be 2", mutest_int_value(gi),
            mutest_to_be, green, NULL);
    mutest_expect ("bi to be 3",mutest_int_value(bi),
            mutest_to_be, blue, NULL);
    mutest_expect ("ai to be 4", mutest_int_value(ai),
            mutest_to_be, alpha, NULL);

    g_object_unref(color);
}

static void
color_suite(void)
{
    mutest_it("Colors get sensible default values", color_default_values);
    mutest_it("Colors can get specific rgb values", color_specific_rgb_values);
    mutest_it("Colors can get specific rgbi values", color_specific_rgbi_values);
}

MUTEST_MAIN(
    mutest_describe("A PsyColor test suite", color_suite);
)


