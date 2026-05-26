
#include <math.h>

#include "../psy/psy-vector.h"

static void
test_vector_create(void)
{
    PsyVector *vec = psy_vector_new();
    g_assert(vec != NULL);
    gfloat x, y, z;

    psy_vector_free(vec);
    vec = psy_vector_new_x(1.0f);
    g_assert_nonnull(vec);

    g_object_get(vec, "x", &x, "y", &y, "z", &z, NULL);
    g_assert_cmpfloat(x, ==, 1.0f);
    g_assert_cmpfloat(y, ==, 0.0f);
    g_assert_cmpfloat(z, ==, 0.0f);
    psy_vector_free(vec);

    vec = psy_vector_new_xy(10, 10);
    g_assert_nonnull(vec);
    g_object_get(vec, "x", &x, "y", &y, "z", &z, NULL);
    g_assert_cmpfloat(x, ==, 10);
    g_assert_cmpfloat(y, ==, 10);
    g_assert_cmpfloat(z, ==, 0);
    psy_vector_free(vec);

    vec = psy_vector_new_xyz(100, 100, 100);
    g_assert_nonnull(vec);
    g_object_get(vec, "x", &x, "y", &y, "z", &z, NULL);
    g_assert_cmpfloat(x, ==, 100);
    g_assert_cmpfloat(y, ==, 100);
    g_assert_cmpfloat(z, ==, 100);

    psy_vector_free(vec);
}

static void
test_vector_magnitude(void)
{
    gfloat     x = 10, y = 20, z = 40;
    PsyVector *vec = psy_vector_new_xyz(x, y, z);
    gfloat     length, magnitude;
    gfloat     expected = sqrtf(x * x + y * y + z * z);
    magnitude           = psy_vector_magnitude(vec);
    g_object_get(vec, "length", &length, NULL);
    gfloat epsilon = 1e-9f;

    g_assert_cmpfloat_with_epsilon(magnitude, expected, epsilon);
    g_assert_cmpfloat_with_epsilon(length, magnitude, epsilon);

    g_object_unref(vec);
}

static void
test_vector_unit(void)
{
    gfloat     x = 10, y = 20, z = 40;
    gfloat     length;
    gfloat     epsilon = 1e-9f;
    PsyVector *vec     = psy_vector_new_xyz(x, y, z);
    PsyVector *unit    = psy_vector_unit(vec);

    // A unit vector has a length of 1.0
    length = psy_vector_magnitude(unit);
    g_assert_cmpfloat_with_epsilon(length, 1.0f, epsilon);

    psy_vector_free(vec);
    psy_vector_free(unit);

    // null vectors don't have a direction, so no unit vector exists
    vec  = psy_vector_new();
    unit = psy_vector_unit(vec);
    g_assert_null(unit);

    psy_vector_free(vec);
}

static void
test_vector_negate(void)
{
    gfloat     x = 10, y = 20, z = 40;
    gfloat     mx, my, mz;
    PsyVector *vec     = psy_vector_new_xyz(x, y, z);
    PsyVector *negated = psy_vector_negate(vec);

    g_object_get(negated, "x", &mx, "y", &my, "z", &mz, NULL);

    psy_vector_magnitude(vec);
    g_assert_cmpfloat(mx, ==, -10);
    g_assert_cmpfloat(my, ==, -20);
    g_assert_cmpfloat(mz, ==, -40);

    // The negated vector should have the same magnitude
    g_assert_cmpfloat(
        psy_vector_magnitude(vec), ==, psy_vector_magnitude(negated));

    psy_vector_free(vec);
    psy_vector_free(negated);
}

static void
test_vector_add_scalar(void)
{
    gfloat     x = 10, y = 20, z = 40;
    gfloat     scalar = 2;
    gfloat     rx, ry, rz;
    PsyVector *vec    = psy_vector_new_xyz(x, y, z);
    PsyVector *result = psy_vector_add_s(vec, scalar);
    g_object_get(result, "x", &rx, "y", &ry, "z", &rz, NULL);

    g_assert_cmpfloat(rx, ==, 12);
    g_assert_cmpfloat(ry, ==, 22);
    g_assert_cmpfloat(rz, ==, 42);

    psy_vector_free(vec);
    psy_vector_free(result);
}

static void
test_vector_add_vector(void)
{
    gfloat     x = 10, y = 20, z = 40;
    PsyVector *v1     = psy_vector_new_xyz(x, y, z);
    PsyVector *v2     = psy_vector_new_xyz(x, y, z);
    PsyVector *result = psy_vector_add(v1, v2);
    PsyVector *v3     = psy_vector_mul_s(v1, 2.0f);

    // Adding two vectors is the same as multiplying with
    g_assert_true(psy_vector_equals(result, v3));

    psy_vector_free(v1);
    psy_vector_free(v2);
    psy_vector_free(v3);
    psy_vector_free(result);
}

static void
test_vector_subtract_scalar(void)
{
    gfloat     x = 10, y = 20, z = 40;
    gfloat     scalar = 2;
    gfloat     rx, ry, rz;
    PsyVector *vec    = psy_vector_new_xyz(x, y, z);
    PsyVector *result = psy_vector_sub_s(vec, scalar);
    g_object_get(result, "x", &rx, "y", &ry, "z", &rz, NULL);
    g_assert_cmpfloat(rx, ==, x - scalar);
    g_assert_cmpfloat(ry, ==, y - scalar);
    g_assert_cmpfloat(rz, ==, z - scalar);

    psy_vector_free(vec);
    psy_vector_free(result);
}

static void
test_vector_subtract_vector(void)
{
    gfloat     x = 10, y = 20, z = 40;
    PsyVector *v1       = psy_vector_new_xyz(x, y, z);
    PsyVector *v2       = psy_vector_new_xyz(x, y, z);
    PsyVector *result   = psy_vector_sub(v1, v2);
    PsyVector *expected = psy_vector_new();

    // subtracting two the same vectors results in a null vector
    g_assert_true(psy_vector_equals(result, expected));

    psy_vector_free(v1);
    psy_vector_free(v2);
    psy_vector_free(expected);
    psy_vector_free(result);
}

static void
test_vector_mul_scalar(void)
{
    gfloat     x = 10, y = 20, z = 40;
    gfloat     scalar = 2.0f;
    PsyVector *v1     = psy_vector_new_xyz(x, y, z);
    PsyVector *result = psy_vector_mul_s(v1, scalar);
    PsyVector *expected
        = psy_vector_new_xyz(x * scalar, y * scalar, z * scalar);
    g_assert_true(psy_vector_equals(expected, result));
    psy_vector_free(v1);
    psy_vector_free(result);
    psy_vector_free(expected);
}

static void
test_vector_dot(void)
{
    PsyVector *v1, *v2;
    v1         = psy_vector_new_xyz(1, 0, 0);
    v2         = psy_vector_new_xyz(0, 1, 0);
    gfloat cos = psy_vector_dot(v1, v2);
    g_assert_cmpfloat(cos, ==, 0.0f);

    cos = psy_vector_dot(v2, v1);
    g_assert_cmpfloat(cos, ==, 0.0f);
    psy_vector_free(v1);
    psy_vector_free(v2);
}

static void
test_vector_cross_product(void)
{
    PsyVector *v1, *v2, *v3, *result, *result_reversed, *v3min;
    v1    = psy_vector_new_xyz(1, 0, 0);
    v2    = psy_vector_new_xyz(0, 1, 0);
    v3    = psy_vector_new_xyz(0, 0, 1);
    v3min = psy_vector_negate(v3);

    result = psy_vector_cross(v1, v2);
    g_assert_true(psy_vector_equals(v3, result));

    result_reversed = psy_vector_cross(v2, v1);
    g_assert_true(psy_vector_equals(v3min, result_reversed));

    psy_vector_free(v1);
    psy_vector_free(v2);
    psy_vector_free(v3);
    psy_vector_free(v3min);
    psy_vector_free(result);
    psy_vector_free(result_reversed);
}

int
main(int argc, char **argv)
{
    g_test_init(&argc, &argv, NULL);

    g_test_add_func("/vector/create", test_vector_create);
    g_test_add_func("/vector/magnitude", test_vector_magnitude);
    g_test_add_func("/vector/unit", test_vector_unit);
    g_test_add_func("/vector/negate", test_vector_negate);
    g_test_add_func("/vector/add_scalar", test_vector_add_scalar);
    g_test_add_func("/vector/add_vector", test_vector_add_vector);
    g_test_add_func("/vector/subtract_scalar", test_vector_subtract_scalar);
    g_test_add_func("/vector/subtract_vector", test_vector_subtract_vector);
    g_test_add_func("/vector/mul_scalar", test_vector_mul_scalar);
    g_test_add_func("/vector/dot", test_vector_dot);
    g_test_add_func("/vector/cross", test_vector_cross_product);

    return g_test_run();
}
