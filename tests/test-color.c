
#include <CUnit/CUnit.h>
#include <CUnit/TestDB.h>
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

    // Default floating point values should be 0, except alpha channel which
    // should be 1.0.
    CU_ASSERT_DOUBLE_EQUAL(rf, 0.0, 0.0);
    CU_ASSERT_DOUBLE_EQUAL(gf, 0.0, 0.0);
    CU_ASSERT_DOUBLE_EQUAL(bf, 0.0, 0.0);
    CU_ASSERT_DOUBLE_EQUAL(af, 1.0, 0.0);

    // Default integer values should be 0, except alpha channel which
    // should be 255.
    CU_ASSERT_EQUAL(ri, 0);    
    CU_ASSERT_EQUAL(gi, 0);    
    CU_ASSERT_EQUAL(bi, 0);    
    CU_ASSERT_EQUAL(ai, 255);    

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

    CU_ASSERT_DOUBLE_EQUAL(rf, 1.0,  0.0);
    CU_ASSERT_DOUBLE_EQUAL(gf, 0.5,  0.0);
    CU_ASSERT_DOUBLE_EQUAL(bf, 0.25, 0.0);
    CU_ASSERT_DOUBLE_EQUAL(af, 0.5,  0.0);
    
    CU_ASSERT_EQUAL(ri, (int)(red * max_color));
    CU_ASSERT_EQUAL(gi, (int)(green * max_color));
    CU_ASSERT_EQUAL(bi, (int)(blue * max_color));
    CU_ASSERT_EQUAL(ai, (int)(alpha * max_color));

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

    CU_ASSERT_DOUBLE_EQUAL(rf, red / max_color, epsilon);
    CU_ASSERT_DOUBLE_EQUAL(gf, green / max_color, epsilon);
    CU_ASSERT_DOUBLE_EQUAL(bf, blue / max_color, epsilon);
    CU_ASSERT_DOUBLE_EQUAL(af, alpha / max_color, epsilon);
    
    CU_ASSERT_EQUAL(ri, red);
    CU_ASSERT_EQUAL(gi, green);
    CU_ASSERT_EQUAL(bi, blue);
    CU_ASSERT_EQUAL(ai, alpha);

    g_object_unref(color);
}

int
add_color_suite(void)
{
    CU_Suite* suite = CU_add_suite("color tests", NULL, NULL);
    CU_Test* test = NULL;

    if (!suite)
        return 1;

    test = CU_add_test(
            suite,
            "Colors get sensible default values",
            color_default_values
            );
    if (!test)
        return 1;

    test = CU_add_test(
            suite,
            "Colors can get specific rgb values",
            color_specific_rgb_values
            );
    if (!test)
        return 1;

    test = CU_add_test(
            suite,
            "Colors can get specific rgbi values",
            color_specific_rgbi_values
            );
    if (!test)
        return 1;

    return 0;
}

