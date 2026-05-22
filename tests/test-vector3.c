#include <math.h>
#include <psylib.h>

static void
test_vector3_create(void)
{
    PsyVector3 *vec = psy_vector3_new();
    g_assert_nonnull(vec); // vector3 instances can be created
    psy_vector3_free(vec);

    gfloat data[3] = {1, 2, 3};
    gfloat x, y, z;
    vec = psy_vector3_new_data(3, data);
    g_object_get(vec, "x", &x, "y", &y, "z", &z, NULL);

    g_assert_cmpfloat(x, ==, data[0]);
    g_assert_cmpfloat(y, ==, data[1]);
    g_assert_cmpfloat(z, ==, data[2]);

    psy_vector3_free(vec);
}

static void
test_vector_magnitude(void)
{
    gfloat      x = 10, y = 20, z = 40;
    gfloat      values[3] = {x, y, z};
    PsyVector3 *vec       = psy_vector3_new_data(3, values);

    gfloat length, magnitude;
    gfloat expected = sqrtf(x * x + y * y + z * z);
    magnitude       = psy_vector3_get_magnitude(vec);
    g_object_get(vec, "magnitude", &length, NULL);
    g_assert_cmpfloat(magnitude, ==, expected);
    g_assert_cmpfloat(length, ==, magnitude);

    g_object_unref(vec);
}

static void
test_vector3_unit(void)
{
    gfloat      x = 10, y = 20, z = 40;
    gfloat      length;
    PsyVector3 *vec
        = g_object_new(PSY_TYPE_VECTOR3, "x", x, "y", y, "z", z, NULL);
    PsyVector3 *unit = NULL;
    g_object_get(vec, "unit", &unit, NULL);

    length = psy_vector3_get_magnitude(unit);
    g_assert_cmpfloat(length, ==, 1.0f);
    psy_vector3_free(vec);
    psy_vector3_free(unit);

    // "It's not possible to retrieve a unit vector from a null vector
    vec  = psy_vector3_new();
    unit = psy_vector3_unit(vec);
    g_assert_null(unit);

    psy_vector3_free(vec);
}

static void
test_vector3_negate(void)
{
    gfloat      x = 10, y = 20, z = 40;
    gfloat      mx, my, mz;
    PsyVector3 *vec
        = g_object_new(PSY_TYPE_VECTOR3, "x", x, "y", y, "z", z, NULL);

    PsyVector3 *negated = psy_vector3_negate(vec);

    g_object_get(negated, "x", &mx, "y", &my, "z", &mz, NULL);

    psy_vector3_get_magnitude(vec);
    g_assert_cmpfloat(mx, ==, -10);
    g_assert_cmpfloat(my, ==, -20);
    g_assert_cmpfloat(mz, ==, -40);
    g_assert_cmpfloat(
        psy_vector3_get_magnitude(vec), ==, psy_vector3_get_magnitude(negated));

    psy_vector3_free(vec);
    psy_vector3_free(negated);
}

static void
test_vector3_add_scalar(void)
{
    gfloat      x = 10, y = 20, z = 40;
    gfloat      scalar = 2;
    gfloat      rx, ry, rz;
    PsyVector3 *vec
        = g_object_new(PSY_TYPE_VECTOR3, "x", x, "y", y, "z", z, NULL);

    PsyVector3 *result = psy_vector3_add_s(vec, scalar);
    g_object_get(result, "x", &rx, "y", &ry, "z", &rz, NULL);
    g_assert_cmpfloat(rx, ==, x + scalar);
    g_assert_cmpfloat(ry, ==, y + scalar);
    g_assert_cmpfloat(rz, ==, z + scalar);

    psy_vector3_free(vec);
    psy_vector3_free(result);
}

static void
test_vector3_add_vector(void)
{
    gfloat      x = 10, y = 20, z = 40;
    PsyVector3 *v1
        = g_object_new(PSY_TYPE_VECTOR3, "x", x, "y", y, "z", z, NULL);
    PsyVector3 *v2
        = g_object_new(PSY_TYPE_VECTOR3, "x", x, "y", y, "z", z, NULL);
    PsyVector3 *result = psy_vector3_add(v1, v2);
    PsyVector3 *v3     = psy_vector3_mul_s(v1, 2.0f);
    g_assert_true(psy_vector3_equals(result, v3));

    psy_vector3_free(v1);
    psy_vector3_free(v2);
    psy_vector3_free(v3);
    psy_vector3_free(result);
}

static void
test_vector3_subtract_scalar(void)
{
    gfloat      x = 10, y = 20, z = 40;
    gfloat      scalar = 2;
    gfloat      rx, ry, rz;
    PsyVector3 *vec
        = g_object_new(PSY_TYPE_VECTOR3, "x", x, "y", y, "z", z, NULL);
    PsyVector3 *result = psy_vector3_sub_s(vec, scalar);
    g_object_get(result, "x", &rx, "y", &ry, "z", &rz, NULL);
    g_assert_cmpfloat(rx, ==, x - scalar);
    g_assert_cmpfloat(ry, ==, y - scalar);
    g_assert_cmpfloat(rz, ==, z - scalar);

    psy_vector3_free(vec);
    psy_vector3_free(result);
}

static void
test_vector3_sub_vector(void)
{
    gfloat      x = 10, y = 20, z = 40;
    PsyVector3 *v1
        = g_object_new(PSY_TYPE_VECTOR3, "x", x, "y", y, "z", z, NULL);
    PsyVector3 *result = psy_vector3_sub(v1, v1);
    g_assert_true(psy_vector3_is_null(result));

    psy_vector3_free(v1);
    psy_vector3_free(result);
}

static void
test_vector3_multiply_scalar(void)
{
    gfloat      x = 10, y = 20, z = 40;
    gfloat      scalar = 2.0f;
    PsyVector3 *v1
        = g_object_new(PSY_TYPE_VECTOR3, "x", x, "y", y, "z", z, NULL);
    PsyVector3 *result = psy_vector3_mul_s(v1, scalar);

    // clang-format off
    PsyVector3 *expected = g_object_new(PSY_TYPE_VECTOR3,
            "x", x * scalar,
            "y", y * scalar,
            "z", z * scalar,
            NULL);
    // clang-format on

    g_assert_true(psy_vector3_equals(expected, result));
    // The magnitude is scaled
    g_assert_cmpfloat(psy_vector3_get_magnitude(v1) * scalar,
                      ==,
                      psy_vector3_get_magnitude(result));

    psy_vector3_free(v1);
    psy_vector3_free(result);
    psy_vector3_free(expected);
}

static void
test_vector3_dot_product(void)
{
    gfloat      x = 1, y = 1;
    PsyVector3 *v1, *v2;
    v1         = g_object_new(PSY_TYPE_VECTOR3, "x", x, NULL);
    v2         = g_object_new(PSY_TYPE_VECTOR3, "y", y, NULL);
    gfloat cos = psy_vector3_dot(v1, v2);
    g_assert_cmpfloat(cos, ==, 0.0f);

    cos = psy_vector3_dot(v2, v1);
    g_assert_cmpfloat(cos, ==, 0.0f);
    psy_vector3_free(v1);
    psy_vector3_free(v2);
}

int
main(int argc, char **argv)
{
    g_test_init(&argc, &argv, NULL);

    g_test_add_func("/vector3/create", test_vector3_create);
    g_test_add_func("/vector3/magnitude", test_vector_magnitude);
    g_test_add_func("/vector3/unit", test_vector3_unit);
    g_test_add_func("/vector3/negate", test_vector3_negate);
    g_test_add_func("/vector3/add_scalar", test_vector3_add_scalar);
    g_test_add_func("/vector3/add_vector", test_vector3_add_vector);
    g_test_add_func("/vector3/subtract_scalar", test_vector3_subtract_scalar);
    g_test_add_func("/vector3/subtract_vector", test_vector3_sub_vector);
    g_test_add_func("/vector3/multiply_scalar", test_vector3_multiply_scalar);
    g_test_add_func("/vector3/dot", test_vector3_dot_product);

    return g_test_run();
}
