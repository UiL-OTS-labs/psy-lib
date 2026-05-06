
#include <criterion/criterion.h>
#include <criterion/new/assert.h>
#include <math.h>

#include "../psy/psy-vector.h"

Test(vector, create)
{
    PsyVector *vec = psy_vector_new();
    g_assert(vec != NULL);
    gfloat x, y, z;

    psy_vector_free(vec);
    vec = psy_vector_new_x(1.0f);
    cr_assert(ne(vec, NULL), "Vectors can be created");

    g_object_get(vec, "x", &x, "y", &y, "z", &z, NULL);
    cr_expect(eq(x, 1.0f));
    cr_expect(eq(y, 0.0f));
    cr_expect(eq(z, 0.0f));
    psy_vector_free(vec);

    vec = psy_vector_new_xy(10, 10);
    cr_assert(vec != NULL);
    g_object_get(vec, "x", &x, "y", &y, "z", &z, NULL);
    cr_assert(eq(x, 10));
    cr_assert(eq(y, 10));
    cr_assert(eq(z, 0));
    psy_vector_free(vec);

    vec = psy_vector_new_xyz(100, 100, 100);
    cr_assert(ne(vec, NULL));
    g_object_get(vec, "x", &x, "y", &y, "z", &z, NULL);
    cr_expect(eq(x, 100));
    cr_expect(eq(y, 100));
    cr_expect(eq(z, 100));

    psy_vector_free(vec);
}

Test(vector, magnitude)
{
    gfloat     x = 10, y = 20, z = 40;
    PsyVector *vec = psy_vector_new_xyz(x, y, z);
    gfloat     length, magnitude;
    gfloat     expected = sqrtf(x * x + y * y + z * z);
    magnitude           = psy_vector_magnitude(vec);
    g_object_get(vec, "length", &length, NULL);
    gfloat epsilon = 1e-6f;

    cr_expect(epsilon_eq(magnitude, expected, epsilon));
    cr_expect(epsilon_eq(length, magnitude, 0.0));

    g_object_unref(vec);
}

Test(vector, unit)
{
    gfloat     x = 10, y = 20, z = 40;
    gfloat     length;
    gfloat     epsilon = 1e-9f;
    PsyVector *vec     = psy_vector_new_xyz(x, y, z);
    PsyVector *unit    = psy_vector_unit(vec);

    length = psy_vector_magnitude(unit);
    cr_expect(epsilon_eq(length, 1.0f, epsilon),
              "A unit vector has a length of 1.0");
    psy_vector_free(vec);
    psy_vector_free(unit);

    vec  = psy_vector_new();
    unit = psy_vector_unit(vec);
    cr_expect(eq(unit, NULL),
              "null vectors don't have a direction, so no unit exists");
    psy_vector_free(vec);
}

Test(vector, negate)
{
    gfloat     x = 10, y = 20, z = 40;
    gfloat     mx, my, mz;
    PsyVector *vec     = psy_vector_new_xyz(x, y, z);
    PsyVector *negated = psy_vector_negate(vec);

    g_object_get(negated, "x", &mx, "y", &my, "z", &mz, NULL);

    psy_vector_magnitude(vec);
    cr_expect(eq(mx, -10));
    cr_expect(eq(my, -20));
    cr_expect(eq(mz, -40));

    cr_expect(eq(psy_vector_magnitude(vec), psy_vector_magnitude(negated)),
              "The negated vector should have the same magnitude");

    psy_vector_free(vec);
    psy_vector_free(negated);
}

Test(vector, add_scalar)
{
    gfloat     x = 10, y = 20, z = 40;
    gfloat     scalar = 2;
    gfloat     rx, ry, rz;
    PsyVector *vec    = psy_vector_new_xyz(x, y, z);
    PsyVector *result = psy_vector_add_s(vec, scalar);
    g_object_get(result, "x", &rx, "y", &ry, "z", &rz, NULL);

    cr_expect(eq(rx, 12));
    cr_expect(eq(ry, 22));
    cr_expect(eq(rz, 42));

    psy_vector_free(vec);
    psy_vector_free(result);
}

Test(vector, add_vector)
{
    gfloat     x = 10, y = 20, z = 40;
    PsyVector *v1     = psy_vector_new_xyz(x, y, z);
    PsyVector *v2     = psy_vector_new_xyz(x, y, z);
    PsyVector *result = psy_vector_add(v1, v2);
    PsyVector *v3     = psy_vector_mul_s(v1, 2.0f);

    cr_expect(all(psy_vector_equals(result, v3)));

    psy_vector_free(v1);
    psy_vector_free(v2);
    psy_vector_free(v3);
    psy_vector_free(result);
}

Test(vector, subtract_scalar)
{
    gfloat     x = 10, y = 20, z = 40;
    gfloat     scalar = 2;
    gfloat     rx, ry, rz;
    PsyVector *vec    = psy_vector_new_xyz(x, y, z);
    PsyVector *result = psy_vector_sub_s(vec, scalar);
    g_object_get(result, "x", &rx, "y", &ry, "z", &rz, NULL);
    cr_expect(eq(rx, x - scalar));
    cr_expect(eq(ry, y - scalar));
    cr_expect(eq(rz, z - scalar));

    psy_vector_free(vec);
    psy_vector_free(result);
}

Test(vector, subtract_vector)
{
    gfloat     x = 10, y = 20, z = 40;
    PsyVector *v1       = psy_vector_new_xyz(x, y, z);
    PsyVector *v2       = psy_vector_new_xyz(x, y, z);
    PsyVector *result   = psy_vector_sub(v1, v2);
    PsyVector *expected = psy_vector_new();

    cr_expect(all(psy_vector_equals(result, expected)));

    psy_vector_free(v1);
    psy_vector_free(v2);
    psy_vector_free(expected);
    psy_vector_free(result);
}

Test(vector, mul_scalar)
{
    gfloat     x = 10, y = 20, z = 40;
    gfloat     scalar = 2.0f;
    PsyVector *v1     = psy_vector_new_xyz(x, y, z);
    PsyVector *result = psy_vector_mul_s(v1, scalar);
    PsyVector *expected
        = psy_vector_new_xyz(x * scalar, y * scalar, z * scalar);
    cr_expect(all(psy_vector_equals(expected, result)));
    psy_vector_free(v1);
    psy_vector_free(result);
    psy_vector_free(expected);
}

Test(vector, dot)
{
    PsyVector *v1, *v2;
    v1         = psy_vector_new_xyz(1, 0, 0);
    v2         = psy_vector_new_xyz(0, 1, 0);
    gfloat cos = psy_vector_dot(v1, v2);
    cr_expect(epsilon_eq(cos, 0.0f, 0.0));

    cos = psy_vector_dot(v2, v1);
    cr_expect(eq(cos, 0.0f));
    psy_vector_free(v1);
    psy_vector_free(v2);
}

Test(vector, cross_product)
{
    PsyVector *v1, *v2, *v3, *result, *result_reversed, *v3min;
    v1    = psy_vector_new_xyz(1, 0, 0);
    v2    = psy_vector_new_xyz(0, 1, 0);
    v3    = psy_vector_new_xyz(0, 0, 1);
    v3min = psy_vector_negate(v3);

    result = psy_vector_cross(v1, v2);
    cr_expect(all(psy_vector_equals(v3, result)));

    result_reversed = psy_vector_cross(v2, v1);
    cr_expect(all(psy_vector_equals(v3min, result_reversed)));

    psy_vector_free(v1);
    psy_vector_free(v2);
    psy_vector_free(v3);
    psy_vector_free(v3min);
    psy_vector_free(result);
    psy_vector_free(result_reversed);
}
