
#include <math.h>
#include <string.h>

#include <CUnit/CUnit.h>

#include "psylib.h"

static void
test_create(void)
{
    PsyVector3 *vec = psy_vector3_new();
    CU_ASSERT_PTR_NOT_NULL_FATAL(vec);
    psy_vector3_destroy(vec);

    gfloat data[3] = {1, 2, 3};
    gfloat x, y, z;
    vec = psy_vector3_new_data(3, data);
    g_object_get(vec, "x", &x, "y", &y, "z", &z, NULL);
    CU_ASSERT_EQUAL(x, data[0]);
    CU_ASSERT_EQUAL(y, data[1]);
    CU_ASSERT_EQUAL(z, data[2]);

    psy_vector3_destroy(vec);
}

static void
test_magnitude(void)
{
    gfloat      x = 10, y = 20, z = 40;
    gfloat      values[3] = {x, y, z};
    PsyVector3 *vec       = psy_vector3_new_data(3, values);

    gfloat length, magnitude;
    gfloat expected = sqrt(x * x + y * y + z * z);
    magnitude       = psy_vector3_get_magnitude(vec);
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
    PsyVector3 *vec
        = g_object_new(PSY_TYPE_VECTOR3, "x", x, "y", y, "z", z, NULL);
    PsyVector3 *unit = NULL;
    g_object_get(vec, "unit", &unit, NULL);

    length = psy_vector3_get_magnitude(unit);
    CU_ASSERT_EQUAL(length, 1.0f);
    psy_vector3_destroy(vec);
    psy_vector3_destroy(unit);

    vec  = psy_vector3_new();
    unit = psy_vector3_unit(vec);
    CU_ASSERT_PTR_NULL(unit);

    psy_vector3_destroy(vec);
}

static void
test_negate(void)
{
    gfloat      x = 10, y = 20, z = 40;
    gfloat      mx, my, mz;
    PsyVector3 *vec
        = g_object_new(PSY_TYPE_VECTOR3, "x", x, "y", y, "z", z, NULL);

    PsyVector3 *negated = psy_vector3_negate(vec);

    g_object_get(negated, "x", &mx, "y", &my, "z", &mz, NULL);

    psy_vector3_get_magnitude(vec);
    CU_ASSERT_EQUAL(mx, -x);
    CU_ASSERT_EQUAL(my, -y);
    CU_ASSERT_EQUAL(mz, -z);
    CU_ASSERT_EQUAL(psy_vector3_get_magnitude(vec),
                    psy_vector3_get_magnitude(negated));

    psy_vector3_destroy(vec);
    psy_vector3_destroy(negated);
}

static void
test_add_scalar(void)
{
    gfloat      x = 10, y = 20, z = 40;
    gfloat      scalar = 2;
    gfloat      rx, ry, rz;
    PsyVector3 *vec
        = g_object_new(PSY_TYPE_VECTOR3, "x", x, "y", y, "z", z, NULL);

    PsyVector3 *result = psy_vector3_add_s(vec, scalar);
    g_object_get(result, "x", &rx, "y", &ry, "z", &rz, NULL);
    CU_ASSERT_EQUAL(rx, x + scalar);
    CU_ASSERT_EQUAL(ry, y + scalar);
    CU_ASSERT_EQUAL(rz, z + scalar);

    psy_vector3_destroy(vec);
    psy_vector3_destroy(result);
}

static void
test_add_vector(void)
{
    gfloat      x = 10, y = 20, z = 40;
    PsyVector3 *v1
        = g_object_new(PSY_TYPE_VECTOR3, "x", x, "y", y, "z", z, NULL);
    PsyVector3 *v2
        = g_object_new(PSY_TYPE_VECTOR3, "x", x, "y", y, "z", z, NULL);
    PsyVector3 *result = psy_vector3_add(v1, v2);
    PsyVector3 *v3     = psy_vector3_mul_s(v1, 2.0);
    CU_ASSERT_TRUE(psy_vector3_equals(result, v3));

    psy_vector3_destroy(v1);
    psy_vector3_destroy(v2);
    psy_vector3_destroy(v3);
    psy_vector3_destroy(result);
}

static void
test_sub_scalar(void)
{
    gfloat      x = 10, y = 20, z = 40;
    gfloat      scalar = 2;
    gfloat      rx, ry, rz;
    PsyVector3 *vec
        = g_object_new(PSY_TYPE_VECTOR3, "x", x, "y", y, "z", z, NULL);
    PsyVector3 *result = psy_vector3_sub_s(vec, scalar);
    g_object_get(result, "x", &rx, "y", &ry, "z", &rz, NULL);
    CU_ASSERT_EQUAL(rx, x - scalar);
    CU_ASSERT_EQUAL(ry, y - scalar);
    CU_ASSERT_EQUAL(rz, z - scalar);

    psy_vector3_destroy(vec);
    psy_vector3_destroy(result);
}

static void
test_sub_vector(void)
{
    gfloat      x = 10, y = 20, z = 40;
    PsyVector3 *v1
        = g_object_new(PSY_TYPE_VECTOR3, "x", x, "y", y, "z", z, NULL);
    PsyVector3 *result = psy_vector3_sub(v1, v1);
    CU_ASSERT_TRUE(psy_vector3_is_null(result));

    psy_vector3_destroy(v1);
    psy_vector3_destroy(result);
}

static void
test_mul_scalar(void)
{
    gfloat      x = 10, y = 20, z = 40;
    gfloat      scalar = 2.0;
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

    CU_ASSERT_TRUE(psy_vector3_equals(expected, result));

    psy_vector3_destroy(v1);
    psy_vector3_destroy(result);
    psy_vector3_destroy(expected);
}

static void
test_vector_dot(void)
{
    gfloat      x = 1, y = 1;
    PsyVector3 *v1, *v2;
    v1         = g_object_new(PSY_TYPE_VECTOR3, "x", x, NULL);
    v2         = g_object_new(PSY_TYPE_VECTOR3, "y", y, NULL);
    gfloat cos = psy_vector3_dot(v1, v2);
    CU_ASSERT_EQUAL(cos, 0.0f);

    cos = psy_vector3_dot(v2, v1);
    CU_ASSERT_EQUAL(cos, 0.0f);
    psy_vector3_destroy(v1);
    psy_vector3_destroy(v2);
}

int
add_vector3_suite(void)
{
    CU_Suite *suite = CU_add_suite("PsyVector3 suite", NULL, NULL);
    CU_Test  *test;
    if (!suite)
        return 1;

    test = CU_add_test(suite, "Vector3 Create", test_create);
    if (!test)
        return 1;

    test = CU_add_test(suite, "Vector3 magnitude", test_magnitude);
    if (!test)
        return 1;

    test = CU_add_test(suite, "Vector3 unit", test_unit);
    if (!test)
        return 1;

    test = CU_add_test(suite, "Vector3 create", test_create);
    if (!test)
        return 1;

    test = CU_add_test(suite, "Vector3 negate", test_negate);
    if (!test)
        return 1;

    test = CU_add_test(suite, "Vector3 add scalar", test_add_scalar);
    if (!test)
        return 1;

    test = CU_add_test(suite, "Vector3 add vector", test_add_vector);
    if (!test)
        return 1;

    test = CU_add_test(suite, "Vector3 subtract scalar", test_sub_scalar);
    if (!test)
        return 1;

    test = CU_add_test(suite, "Vector3 subtract vector", test_sub_vector);
    if (!test)
        return 1;

    test = CU_add_test(suite, "Vector3 scale", test_mul_scalar);
    if (!test)
        return 1;

    test = CU_add_test(suite, "Vector3 dot product", test_vector_dot);
    if (!test)
        return 1;

    return 0;
}
