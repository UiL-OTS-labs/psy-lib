#include <math.h>
#include <psylib.h>

static void
test_vector4_create(void)
{
    PsyVector4 *vec = psy_vector4_new();
    g_assert_nonnull(vec); // PsyVector4 can be instantiated
    psy_vector4_free(vec);

    gfloat data[4] = {1, 2, 3, 4};
    gfloat x, y, z, w;
    vec = psy_vector4_new_data(4, data);
    g_object_get(vec, "x", &x, "y", &y, "z", &z, "w", &w, NULL);

    g_assert_cmpfloat(x, ==, data[0]);
    g_assert_cmpfloat(y, ==, data[1]);
    g_assert_cmpfloat(z, ==, data[2]);
    g_assert_cmpfloat(w, ==, data[3]);

    psy_vector4_free(vec);
}

static void
test_vector4_magnitude(void)
{
    gfloat      x = 10, y = 20, z = 40, w = 20;
    gfloat      values[4] = {x, y, z, w};
    PsyVector4 *vec       = psy_vector4_new_data(4, values);

    gfloat length, magnitude;
    gfloat expected = sqrtf(x * x + y * y + z * z + w * w);
    magnitude       = psy_vector4_get_magnitude(vec);
    g_object_get(vec, "magnitude", &length, NULL);

    g_assert_cmpfloat(magnitude, ==, expected);
    g_assert_cmpfloat(length, ==, magnitude);

    g_object_unref(vec);
}

static void
test_vector4_unit(void)
{
    gfloat      x = 10, y = 20, z = 40;
    gfloat      length;
    PsyVector4 *vec
        = g_object_new(PSY_TYPE_VECTOR4, "x", x, "y", y, "z", z, NULL);
    PsyVector4 *unit = NULL;
    g_object_get(vec, "unit", &unit, NULL);

    length = psy_vector4_get_magnitude(unit);
    g_assert_cmpfloat(length, ==, 1.0f);
    psy_vector4_free(vec);
    psy_vector4_free(unit);

    vec  = psy_vector4_new();
    unit = psy_vector4_unit(vec);
    // Null vectors have no direction, so no unit vector is possible
    g_assert_null(unit);

    psy_vector4_free(vec);
}

static void
test_vector4_negate(void)
{
    gfloat      x = 10, y = 20, z = 40, w = 1;
    gfloat      mx, my, mz, mw;
    PsyVector4 *vec
        = g_object_new(PSY_TYPE_VECTOR4, "x", x, "y", y, "z", z, "w", w, NULL);

    PsyVector4 *negated = psy_vector4_negate(vec);

    g_object_get(negated, "x", &mx, "y", &my, "z", &mz, "w", &mw, NULL);

    psy_vector4_get_magnitude(vec);
    g_assert_cmpfloat(mx, ==, -x);
    g_assert_cmpfloat(my, ==, -y);
    g_assert_cmpfloat(mz, ==, -z);
    g_assert_cmpfloat(mw, ==, -w);
    // they still should have an equal length
    g_assert_cmpfloat(
        psy_vector4_get_magnitude(vec), ==, psy_vector4_get_magnitude(negated));

    psy_vector4_free(vec);
    psy_vector4_free(negated);
}

static void
test_vector4_add_scalar(void)
{
    gfloat      x = 10, y = 20, z = 40, w = -50;
    gfloat      scalar = 2;
    gfloat      rx, ry, rz, rw;
    PsyVector4 *vec
        = g_object_new(PSY_TYPE_VECTOR4, "x", x, "y", y, "z", z, "w", w, NULL);

    PsyVector4 *result = psy_vector4_add_s(vec, scalar);
    g_object_get(result, "x", &rx, "y", &ry, "z", &rz, "w", &rw, NULL);
    g_assert_cmpfloat(rx, ==, x + scalar);
    g_assert_cmpfloat(ry, ==, y + scalar);
    g_assert_cmpfloat(rz, ==, z + scalar);
    g_assert_cmpfloat(rw, ==, w + scalar);

    psy_vector4_free(vec);
    psy_vector4_free(result);
}

static void
test_vector4_add_vector(void)
{
    gfloat      x = 10, y = 20, z = 40, w = -50;
    PsyVector4 *v1
        = g_object_new(PSY_TYPE_VECTOR4, "x", x, "y", y, "z", z, "w", w, NULL);
    PsyVector4 *v2
        = g_object_new(PSY_TYPE_VECTOR4, "x", x, "y", y, "z", z, "w", w, NULL);
    PsyVector4 *result = psy_vector4_add(v1, v2);
    PsyVector4 *v3     = psy_vector4_mul_s(v1, 2.0f);
    g_assert_true(psy_vector4_equals(result, v3));

    psy_vector4_free(v1);
    psy_vector4_free(v2);
    psy_vector4_free(v3);
    psy_vector4_free(result);
}

static void
test_vector4_subtract_scalar(void)
{
    gfloat      x = 10, y = 20, z = 40, w = -50;
    gfloat      scalar = 2;
    gfloat      rx, ry, rz, rw;
    PsyVector4 *vec
        = g_object_new(PSY_TYPE_VECTOR4, "x", x, "y", y, "z", z, "w", w, NULL);
    PsyVector4 *result = psy_vector4_sub_s(vec, scalar);
    g_object_get(result, "x", &rx, "y", &ry, "z", &rz, "w", &rw, NULL);
    g_assert_cmpfloat(rx, ==, x - scalar);
    g_assert_cmpfloat(ry, ==, y - scalar);
    g_assert_cmpfloat(rz, ==, z - scalar);
    g_assert_cmpfloat(rw, ==, w - scalar);

    psy_vector4_free(vec);
    psy_vector4_free(result);
}

static void
test_vector4_sub_vector(void)
{
    gfloat      x = 10, y = 20, z = 40, w = -40;
    PsyVector4 *v1
        = g_object_new(PSY_TYPE_VECTOR4, "x", x, "y", y, "z", z, "w", w, NULL);
    PsyVector4 *result = psy_vector4_sub(v1, v1);
    g_assert_true(psy_vector4_is_null(result));

    psy_vector4_free(v1);
    psy_vector4_free(result);
}

static void
test_vector4_mul_scalar(void)
{
    gfloat      x = 10, y = 20, z = 40, w = -50;
    gfloat      scalar = 2.0f;
    PsyVector4 *v1
        = g_object_new(PSY_TYPE_VECTOR4, "x", x, "y", y, "z", z, "w", w, NULL);
    PsyVector4 *result = psy_vector4_mul_s(v1, scalar);

    // clang-format off
    PsyVector4 *expected = g_object_new(PSY_TYPE_VECTOR4,
            "x", x * scalar,
            "y", y * scalar,
            "z", z * scalar,
            "w", w * scalar,
            NULL);
    // clang-format on

    g_assert_true(psy_vector4_equals(expected, result));
    // scaling by two doubles the magnitude
    g_assert_cmpfloat(psy_vector4_get_magnitude(v1) * 2.0f,
                      ==,
                      psy_vector4_get_magnitude(result));

    psy_vector4_free(v1);
    psy_vector4_free(result);
    psy_vector4_free(expected);
}

static void
test_vector4_dot_product(void)
{
    gfloat      x = 1, y = 1;
    PsyVector4 *v1, *v2;
    v1         = g_object_new(PSY_TYPE_VECTOR4, "x", x, NULL);
    v2         = g_object_new(PSY_TYPE_VECTOR4, "y", y, NULL);
    gfloat cos = psy_vector4_dot(v1, v2);
    g_assert_cmpfloat(cos, ==, 0.0f);

    cos = psy_vector4_dot(v2, v1);
    g_assert_cmpfloat(cos, ==, 0.0f);
    psy_vector4_free(v1);
    psy_vector4_free(v2);
}

int
main(int argc, char **argv)
{
    g_test_init(&argc, &argv, NULL);

    g_test_add_func("/vector4/create", test_vector4_create);
    g_test_add_func("/vector4/magnitude", test_vector4_magnitude);
    g_test_add_func("/vector4/unit", test_vector4_unit);
    g_test_add_func("/vector4/negate", test_vector4_negate);
    g_test_add_func("/vector4/add_scalar", test_vector4_add_scalar);
    g_test_add_func("/vector4/add_vector", test_vector4_add_vector);
    g_test_add_func("/vector4/subtract_scalar", test_vector4_subtract_scalar);
    g_test_add_func("/vector4/subtract_vector", test_vector4_sub_vector);
    g_test_add_func("/vector4/multiply_scalar", test_vector4_mul_scalar);
    g_test_add_func("/vector4/dot", test_vector4_dot_product);

    return g_test_run();
}
