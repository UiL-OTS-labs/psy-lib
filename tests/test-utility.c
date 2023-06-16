
#include <CUnit/CUnit.h>

#include <psylib.h>

gdouble width  = 600;
gdouble height = 300;

static void
test_c_to_center(void)
{
    // left top
    gdouble xin = 0, yin = 0;
    gdouble xout, yout;

    psy_coordinate_c_to_center(width, height, xin, yin, &xout, &yout);
    CU_ASSERT_EQUAL(xout, -width / 2);
    CU_ASSERT_EQUAL(yout, height / 2);

    // right top
    xin = width;
    yin = 0;
    psy_coordinate_c_to_center(width, height, xin, yin, &xout, &yout);
    CU_ASSERT_EQUAL(xout, width / 2);
    CU_ASSERT_EQUAL(yout, height / 2);

    // center
    xin = width / 2;
    yin = height / 2;
    psy_coordinate_c_to_center(width, height, xin, yin, &xout, &yout);
    CU_ASSERT_EQUAL(xout, 0.0);
    CU_ASSERT_EQUAL(yout, 0.0);

    // left bottom
    xin = 0;
    yin = height;
    psy_coordinate_c_to_center(width, height, xin, yin, &xout, &yout);
    CU_ASSERT_EQUAL(xout, -width / 2);
    CU_ASSERT_EQUAL(yout, -height / 2);

    // right bottom
    xin = width;
    yin = height;
    psy_coordinate_c_to_center(width, height, xin, yin, &xout, &yout);
    CU_ASSERT_EQUAL(xout, width / 2);
    CU_ASSERT_EQUAL(yout, -height / 2);
}

static void
test_center_to_c(void)
{
    // left top
    gdouble xin = -width / 2;
    gdouble yin = height / 2;
    gdouble xout, yout;

    psy_coordinate_center_to_c(width, height, xin, yin, &xout, &yout);
    CU_ASSERT_EQUAL(xout, 0);
    CU_ASSERT_EQUAL(yout, 0);

    // right top
    xin = width / 2;
    yin = height / 2;
    psy_coordinate_center_to_c(width, height, xin, yin, &xout, &yout);
    CU_ASSERT_EQUAL(xout, width);
    CU_ASSERT_EQUAL(yout, 0);

    // center
    xin = 0;
    yin = 0;
    psy_coordinate_center_to_c(width, height, xin, yin, &xout, &yout);
    CU_ASSERT_EQUAL(xout, width / 2);
    CU_ASSERT_EQUAL(yout, height / 2);

    // left bottom
    xin = -width / 2;
    yin = -height / 2;
    psy_coordinate_center_to_c(width, height, xin, yin, &xout, &yout);
    CU_ASSERT_EQUAL(xout, 0);
    CU_ASSERT_EQUAL(yout, height);

    // right bottom
    xin = width / 2;
    yin = -height / 2;
    psy_coordinate_center_to_c(width, height, xin, yin, &xout, &yout);
    CU_ASSERT_EQUAL(xout, width);
    CU_ASSERT_EQUAL(yout, height);
}

int
add_utility_suite(void)
{
    CU_Suite *suite = CU_add_suite("utility tests", NULL, NULL);
    CU_Test  *test  = NULL;

    if (!suite)
        return 1;

    test = CU_ADD_TEST(suite, test_c_to_center);
    if (!test)
        return 1;

    test = CU_ADD_TEST(suite, test_center_to_c);
    if (!test)
        return 1;

    return 0;
}
