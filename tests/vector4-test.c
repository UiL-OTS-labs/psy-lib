
#include <math.h>
#include <string.h>

#include <CUnit/CUnit.h>

#include "../psy/psy-vector4.h"
#include "psy-vector.h"

static void
test_create(void)
{
    PsyVector4 *vec = psy_vector4_new();
    CU_ASSERT_PTR_NOT_NULL_FATAL(vec);
    psy_vector4_free(vec);

    gfloat data[4] = {1, 2, 3, 4};
    gfloat x, y, z, w;
    vec = psy_vector4_new_data(4, data);
    g_object_get(vec, "x", &x, "y", &y, "z", &z, "w", &w, NULL);
    CU_ASSERT_EQUAL(x, data[0]);
    CU_ASSERT_EQUAL(y, data[1]);
    CU_ASSERT_EQUAL(z, data[2]);
    CU_ASSERT_EQUAL(w, data[3]);

    psy_vector4_free(vec);
}

static void
test_magnitude(void)
{
    gfloat      x = 10, y = 20, z = 40, w = 20;
    gfloat      values[4] = {x, y, z, w};
    PsyVector4 *vec       = psy_vector4_new_data(4, values);

    gfloat length, magnitude;
    gfloat expected = sqrt(x * x + y * y + z * z + w * w);
    magnitude       = psy_vector4_get_magnitude(vec);
    g_object_get(vec, "magnitude", &length, NULL);
    CU_ASSERT_EQUAL(magnitude, expected);
    CU_ASSERT_EQUAL(length, magnitude);

    g_object_unref(vec);
}

static void
test_unit(void)
{
    gfloat      x = 10, y = 20, z = 40;
    gfloat      length;
    PsyVector4 *vec
        = g_object_new(PSY_TYPE_VECTOR4, "x", x, "y", y, "z", z, NULL);
    PsyVector4 *unit = NULL;
    g_object_get(vec, "unit", &unit, NULL);

    length = psy_vector4_get_magnitude(unit);
    CU_ASSERT_EQUAL(length, 1.0f);
    psy_vector4_free(vec);
    psy_vector4_free(unit);

    vec  = psy_vector4_new();
    unit = psy_vector4_unit(vec);
    CU_ASSERT_PTR_NULL(unit);

    psy_vector4_free(vec);
}

static void
test_negate(void)
{
    gfloat      x = 10, y = 20, z = 40, w = 1;
    gfloat      mx, my, mz, mw;
    PsyVector4 *vec
        = g_object_new(PSY_TYPE_VECTOR4, "x", x, "y", y, "z", z, "w", w, NULL);

    PsyVector4 *negated = psy_vector4_negate(vec);

    g_object_get(negated, "x", &mx, "y", &my, "z", &mz, "w", &mw, NULL);

    psy_vector4_get_magnitude(vec);
    CU_ASSERT_EQUAL(mx, -x);
    CU_ASSERT_EQUAL(my, -y);
    CU_ASSERT_EQUAL(mz, -z);
    CU_ASSERT_EQUAL(psy_vector4_get_magnitude(vec),
                    psy_vector4_get_magnitude(negated));

    psy_vector4_free(vec);
    psy_vector4_free(negated);
}

static void
test_add_scalar(void)
{
    gfloat      x = 10, y = 20, z = 40, w = -50;
    gfloat      scalar = 2;
    gfloat      rx, ry, rz, rw;
    PsyVector4 *vec
        = g_object_new(PSY_TYPE_VECTOR4, "x", x, "y", y, "z", z, "w", w, NULL);

    PsyVector4 *result = psy_vector4_add_s(vec, scalar);
    g_object_get(result, "x", &rx, "y", &ry, "z", &rz, "w", &rw, NULL);
    CU_ASSERT_EQUAL(rx, x + scalar);
    CU_ASSERT_EQUAL(ry, y + scalar);
    CU_ASSERT_EQUAL(rz, z + scalar);
    CU_ASSERT_EQUAL(rw, w + scalar);

    psy_vector4_free(vec);
    psy_vector4_free(result);
}

static void
test_add_vector(void)
{
    gfloat      x = 10, y = 20, z = 40, w = -50;
    PsyVector4 *v1
        = g_object_new(PSY_TYPE_VECTOR4, "x", x, "y", y, "z", z, "w", w, NULL);
    PsyVector4 *v2
        = g_object_new(PSY_TYPE_VECTOR4, "x", x, "y", y, "z", z, "w", w, NULL);
    PsyVector4 *result = psy_vector4_add(v1, v2);
    PsyVector4 *v3     = psy_vector4_mul_s(v1, 2.0);
    CU_ASSERT_TRUE(psy_vector4_equals(result, v3));

    psy_vector4_free(v1);
    psy_vector4_free(v2);
    psy_vector4_free(v3);
    psy_vector4_free(result);
}

static void
test_sub_scalar(void)
{
    gfloat      x = 10, y = 20, z = 40, w = -50;
    gfloat      scalar = 2;
    gfloat      rx, ry, rz, rw;
    PsyVector4 *vec
        = g_object_new(PSY_TYPE_VECTOR4, "x", x, "y", y, "z", z, "w", w, NULL);
    PsyVector4 *result = psy_vector4_sub_s(vec, scalar);
    g_object_get(result, "x", &rx, "y", &ry, "z", &rz, "w", &rw, NULL);
    CU_ASSERT_EQUAL(rx, x - scalar);
    CU_ASSERT_EQUAL(ry, y - scalar);
    CU_ASSERT_EQUAL(rz, z - scalar);
    CU_ASSERT_EQUAL(rw, w - scalar);

    psy_vector4_free(vec);
    psy_vector4_free(result);
}

static void
test_sub_vector(void)
{
    gfloat      x = 10, y = 20, z = 40, w = -40;
    PsyVector4 *v1
        = g_object_new(PSY_TYPE_VECTOR4, "x", x, "y", y, "z", z, "w", w, NULL);
    PsyVector4 *result = psy_vector4_sub(v1, v1);
    CU_ASSERT_TRUE(psy_vector4_is_null(result));

    psy_vector4_free(v1);
    psy_vector4_free(result);
}

static void
test_mul_scalar(void)
{
    gfloat      x = 10, y = 20, z = 40, w = -50;
    gfloat      scalar = 2.0;
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

    CU_ASSERT_TRUE(psy_vector4_equals(expected, result));

    psy_vector4_free(v1);
    psy_vector4_free(result);
    psy_vector4_free(expected);
}

static void
test_vector_dot(void)
{
    gfloat      x = 1, y = 1;
    PsyVector4 *v1, *v2;
    v1         = g_object_new(PSY_TYPE_VECTOR4, "x", x, NULL);
    v2         = g_object_new(PSY_TYPE_VECTOR4, "y", y, NULL);
    gfloat cos = psy_vector4_dot(v1, v2);
    CU_ASSERT_EQUAL(cos, 0.0f);

    cos = psy_vector4_dot(v2, v1);
    CU_ASSERT_EQUAL(cos, 0.0f);
    psy_vector4_free(v1);
    psy_vector4_free(v2);
}

int
add_vector4_suite(void)
{
    CU_Suite *suite = CU_add_suite("PsyVector4 suite", NULL, NULL);
    CU_Test  *test;
    if (!suite)
        return 1;

    test = CU_add_test(suite, "Vector4 Create", test_create);
    if (!test)
        return 1;

    test = CU_add_test(suite, "Vector4 magnitude", test_magnitude);
    if (!test)
        return 1;

    test = CU_add_test(suite, "Vector4 unit", test_unit);
    if (!test)
        return 1;

    test = CU_add_test(suite, "Vector4 create", test_create);
    if (!test)
        return 1;

    test = CU_add_test(suite, "Vector4 negate", test_negate);
    if (!test)
        return 1;

    test = CU_add_test(suite, "Vector4 add scalar", test_add_scalar);
    if (!test)
        return 1;

    test = CU_add_test(suite, "Vector4 add vector", test_add_vector);
    if (!test)
        return 1;

    test = CU_add_test(suite, "Vector4 subtract scalar", test_sub_scalar);
    if (!test)
        return 1;

    test = CU_add_test(suite, "Vector4 subtract vector", test_sub_vector);
    if (!test)
        return 1;

    test = CU_add_test(suite, "Vector4 scale", test_mul_scalar);
    if (!test)
        return 1;

    test = CU_add_test(suite, "Vector4 dot product", test_vector_dot);
    if (!test)
        return 1;

    return 0;
}
