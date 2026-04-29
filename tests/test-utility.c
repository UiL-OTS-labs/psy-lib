
#include <criterion/criterion.h>
#include <criterion/new/assert.h>

#include <psylib.h>

gdouble width  = 600;
gdouble height = 300;

Test(utility, c_to_center)
{
    // left top
    gdouble xin = 0, yin = 0;
    gdouble xout, yout;

    psy_coordinate_c_to_center(width, height, xin, yin, &xout, &yout);
    cr_expect(eq(xout, -width / 2));
    cr_expect(eq(yout, height / 2));

    // right top
    xin = width;
    yin = 0;
    psy_coordinate_c_to_center(width, height, xin, yin, &xout, &yout);
    cr_expect(eq(xout, width / 2));
    cr_expect(eq(yout, height / 2));

    // center
    xin = width / 2;
    yin = height / 2;
    psy_coordinate_c_to_center(width, height, xin, yin, &xout, &yout);
    cr_expect(eq(xout, 0.0));
    cr_expect(eq(yout, 0.0));

    // left bottom
    xin = 0;
    yin = height;
    psy_coordinate_c_to_center(width, height, xin, yin, &xout, &yout);
    cr_expect(eq(xout, -width / 2));
    cr_expect(eq(yout, -height / 2));

    // right bottom
    xin = width;
    yin = height;
    psy_coordinate_c_to_center(width, height, xin, yin, &xout, &yout);
    cr_assert(eq(xout, width / 2));
    cr_assert(eq(yout, -height / 2));
}

Test(utility, center_to_c)
{
    // left top
    gdouble xin = -width / 2;
    gdouble yin = height / 2;
    gdouble xout, yout;

    psy_coordinate_center_to_c(width, height, xin, yin, &xout, &yout);
    cr_expect(eq(xout, 0));
    cr_expect(eq(yout, 0));

    // right top
    xin = width / 2;
    yin = height / 2;
    psy_coordinate_center_to_c(width, height, xin, yin, &xout, &yout);
    cr_expect(eq(xout, width));
    cr_expect(eq(yout, 0));

    // center
    xin = 0;
    yin = 0;
    psy_coordinate_center_to_c(width, height, xin, yin, &xout, &yout);
    cr_expect(eq(xout, width / 2));
    cr_expect(eq(yout, height / 2));

    // left bottom
    xin = -width / 2;
    yin = -height / 2;
    psy_coordinate_center_to_c(width, height, xin, yin, &xout, &yout);
    cr_expect(eq(xout, 0));
    cr_expect(eq(yout, height));

    // right bottom
    xin = width / 2;
    yin = -height / 2;
    psy_coordinate_center_to_c(width, height, xin, yin, &xout, &yout);
    cr_expect(eq(xout, width));
    cr_expect(eq(yout, height));
}
