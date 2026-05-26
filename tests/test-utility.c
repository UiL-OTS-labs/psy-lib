
#include <psylib.h>

gdouble width  = 600;
gdouble height = 300;

static void
test_utility_c_to_center(void)
{
    // left top
    gdouble xin = 0, yin = 0;
    gdouble xout, yout;

    psy_coordinate_c_to_center(width, height, xin, yin, &xout, &yout);
    g_assert_cmpfloat(xout, ==, -width / 2);
    g_assert_cmpfloat(yout, ==, height / 2);

    // right top
    xin = width;
    yin = 0;
    psy_coordinate_c_to_center(width, height, xin, yin, &xout, &yout);
    g_assert_cmpfloat(xout, ==, width / 2);
    g_assert_cmpfloat(yout, ==, height / 2);

    // center
    xin = width / 2;
    yin = height / 2;
    psy_coordinate_c_to_center(width, height, xin, yin, &xout, &yout);
    g_assert_cmpfloat(xout, ==, 0.0);
    g_assert_cmpfloat(yout, ==, 0.0);

    // left bottom
    xin = 0;
    yin = height;
    psy_coordinate_c_to_center(width, height, xin, yin, &xout, &yout);
    g_assert_cmpfloat(xout, ==, -width / 2);
    g_assert_cmpfloat(yout, ==, -height / 2);

    // right bottom
    xin = width;
    yin = height;
    psy_coordinate_c_to_center(width, height, xin, yin, &xout, &yout);
    g_assert_cmpfloat(xout, ==, width / 2);
    g_assert_cmpfloat(yout, ==, -height / 2);
}

static void
test_utility_center_to_c(void)
{
    // left top
    gdouble xin = -width / 2;
    gdouble yin = height / 2;
    gdouble xout, yout;

    psy_coordinate_center_to_c(width, height, xin, yin, &xout, &yout);
    g_assert_cmpfloat(xout, ==, 0);
    g_assert_cmpfloat(yout, ==, 0);

    // right top
    xin = width / 2;
    yin = height / 2;
    psy_coordinate_center_to_c(width, height, xin, yin, &xout, &yout);
    g_assert_cmpfloat(xout, ==, width);
    g_assert_cmpfloat(yout, ==, 0);

    // center
    xin = 0;
    yin = 0;
    psy_coordinate_center_to_c(width, height, xin, yin, &xout, &yout);
    g_assert_cmpfloat(xout, ==, width / 2);
    g_assert_cmpfloat(yout, ==, height / 2);

    // left bottom
    xin = -width / 2;
    yin = -height / 2;
    psy_coordinate_center_to_c(width, height, xin, yin, &xout, &yout);
    g_assert_cmpfloat(xout, ==, 0);
    g_assert_cmpfloat(yout, ==, height);

    // right bottom
    xin = width / 2;
    yin = -height / 2;
    psy_coordinate_center_to_c(width, height, xin, yin, &xout, &yout);
    g_assert_cmpfloat(xout, ==, width);
    g_assert_cmpfloat(yout, ==, height);
}

int
main(int argc, char **argv)
{
    g_test_init(&argc, &argv, NULL);

    g_test_add_func("/utility/c_to_center", test_utility_c_to_center);
    g_test_add_func("/utility/center_to_c", test_utility_center_to_c);

    return g_test_run();
}
