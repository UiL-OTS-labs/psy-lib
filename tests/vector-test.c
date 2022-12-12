
#include <math.h>
#include <CUnit/CUnit.h>

#include "../psy/psy-vector.h"

static void
test_create(void) {
    PsyVector* vec = psy_vector_new();
    g_assert(vec != NULL);
    gfloat x, y ,z;

    psy_vector_destroy(vec);
    vec  = psy_vector_new_x(1.0);
    CU_ASSERT_PTR_NOT_NULL_FATAL(vec);
    g_object_get(vec, "x", &x, "y", &y, "z", &z, NULL);
    CU_ASSERT_DOUBLE_EQUAL(x, 1, 0.0);
    CU_ASSERT_DOUBLE_EQUAL(y, 0, 0.0);
    CU_ASSERT_DOUBLE_EQUAL(z, 0, 0.0);
    psy_vector_destroy(vec);

    vec = psy_vector_new_xy(10, 10);
    CU_ASSERT_PTR_NOT_NULL_FATAL(vec);
    g_object_get(vec, "x", &x, "y", &y, "z", &z, NULL);
    CU_ASSERT_DOUBLE_EQUAL(x, 10, 0);
    CU_ASSERT_DOUBLE_EQUAL(y, 10, 0);
    CU_ASSERT_DOUBLE_EQUAL(z, 0, 0);
    psy_vector_destroy(vec);

    vec = psy_vector_new_xyz(100, 100, 100);
    CU_ASSERT_PTR_NOT_NULL_FATAL(vec);
    g_object_get(vec, "x", &x, "y", &y, "z", &z, NULL);
    CU_ASSERT_DOUBLE_EQUAL(x, 100, 0.0);
    CU_ASSERT_DOUBLE_EQUAL(y, 100, 0.0);
    CU_ASSERT_DOUBLE_EQUAL(z, 100, 0.0);

    psy_vector_destroy(vec);
}

static void
test_magnitude(void)
{
    gfloat x = 10, y = 20, z = 40;
    PsyVector* vec = psy_vector_new_xyz(x, y, z);
    gfloat length, magnitude;
    gfloat expected = sqrtf(x * x + y * y + z * z);
    magnitude = psy_vector_magnitude(vec);
    g_object_get(vec, "length", &length, NULL);
    gfloat epsilon = 1e-6;

    CU_ASSERT_DOUBLE_EQUAL(magnitude, expected, epsilon);
    CU_ASSERT_DOUBLE_EQUAL(length, magnitude, 0.0);

    g_object_unref(vec);
}

static void
test_unit(void) {
    gfloat x = 10, y = 20, z = 40;
    gfloat length;
    gfloat epsilon = 1e-9;
    PsyVector *vec  = psy_vector_new_xyz(x, y, z);
    PsyVector *unit = psy_vector_unit(vec);

    length = psy_vector_magnitude(unit);
    CU_ASSERT_DOUBLE_EQUAL(length, 1.0f, epsilon);
    psy_vector_destroy(vec);
    psy_vector_destroy(unit);

    vec = psy_vector_new();
    unit = psy_vector_unit(vec);
    CU_ASSERT_PTR_NULL(unit);
    psy_vector_destroy(vec);
}

static void
test_negate(void)
{
    gfloat x = 10, y = 20, z = 40;
    gfloat mx, my, mz;
    PsyVector *vec  = psy_vector_new_xyz(x, y, z);
    PsyVector *negated = psy_vector_negate(vec);

    g_object_get(negated,
                 "x", &mx,
                 "y", &my,
                 "z", &mz,
                 NULL
                 );

    psy_vector_magnitude(vec);
    g_assert(mx == -x && my == -y && mz == -z);
    CU_ASSERT_DOUBLE_EQUAL(mx, -x, 0);
    CU_ASSERT_DOUBLE_EQUAL(my, -y, 0);
    CU_ASSERT_DOUBLE_EQUAL(mz, -z, 0);

    CU_ASSERT_EQUAL(psy_vector_magnitude(vec), psy_vector_magnitude(negated));

    psy_vector_destroy(vec);
    psy_vector_destroy(negated);
}

static void
test_add_scalar(void)
{
    gfloat x = 10, y = 20, z = 40;
    gfloat scalar = 2;
    gfloat rx, ry, rz;
    PsyVector *vec  = psy_vector_new_xyz(x, y, z);
    PsyVector *result = psy_vector_add_s(vec, scalar);
    g_object_get(result,
                 "x", &rx,
                 "y", &ry,
                 "z", &rz,
                 NULL
                 );

    CU_ASSERT_EQUAL(rx, x + scalar);
    CU_ASSERT_EQUAL(ry, y + scalar);
    CU_ASSERT_EQUAL(rz, z + scalar);

    psy_vector_destroy(vec);
    psy_vector_destroy(result);
}

static void
test_add_vector(void)
{
    gfloat x = 10, y = 20, z = 40;
    PsyVector *v1  = psy_vector_new_xyz(x, y, z);
    PsyVector *v2  = psy_vector_new_xyz(x, y, z);
    PsyVector *result = psy_vector_add(v1, v2);
    PsyVector *v3  = psy_vector_mul_s(v1, 2.0f);

    CU_ASSERT_TRUE(psy_vector_equals(result, v3));

    psy_vector_destroy(v1);
    psy_vector_destroy(v2);
    psy_vector_destroy(v3);
    psy_vector_destroy(result);
}

static void
test_sub_scalar(void)
{
    gfloat x = 10, y = 20, z = 40;
    gfloat scalar = 2;
    gfloat rx, ry, rz;
    PsyVector *vec  = psy_vector_new_xyz(x, y, z);
    PsyVector *result = psy_vector_sub_s(vec, scalar);
    g_object_get(result,
                 "x", &rx,
                 "y", &ry,
                 "z", &rz,
                 NULL
    );
    CU_ASSERT_EQUAL(rx ,x - scalar);
    CU_ASSERT_EQUAL(ry ,y - scalar);
    CU_ASSERT_EQUAL(rz ,z - scalar);

    psy_vector_destroy(vec);
    psy_vector_destroy(result);
}

static void
test_sub_vector(void)
{
    gfloat x = 10, y = 20, z = 40;
    PsyVector *v1  = psy_vector_new_xyz(x, y, z);
    PsyVector *v2  = psy_vector_new_xyz(x, y, z);
    PsyVector *result = psy_vector_sub(v1, v2);
    PsyVector *expected = psy_vector_new();

    CU_ASSERT_TRUE(psy_vector_equals(result, expected));

    psy_vector_destroy(v1);
    psy_vector_destroy(v2);
    psy_vector_destroy(expected);
    psy_vector_destroy(result);
}

static void
test_mul_scalar(void)
{
    gfloat x = 10, y = 20, z = 40;
    gfloat scalar = 2.0;
    PsyVector *v1  = psy_vector_new_xyz(x, y, z);
    PsyVector *result = psy_vector_mul_s(v1, scalar);
    PsyVector *expected = psy_vector_new_xyz(
            x * scalar,
            y * scalar,
            z * scalar);
    CU_ASSERT_TRUE(psy_vector_equals(expected, result));
    psy_vector_destroy(v1);
    psy_vector_destroy(result);
    psy_vector_destroy(expected);
}

static void
test_vector_dot(void)
{
    PsyVector *v1, *v2;
    v1 = psy_vector_new_xyz(1,0,0);
    v2 = psy_vector_new_xyz(0,1,0);
    gfloat cos = psy_vector_dot(v1, v2);
    CU_ASSERT_DOUBLE_EQUAL(cos, 0.0f, 0.0);

    cos = psy_vector_dot(v2, v1);
    g_assert(cos == 0);
    psy_vector_destroy(v1);
    psy_vector_destroy(v2);
}

static void
test_vector_cross(void)
{
    PsyVector *v1, *v2, *v3, *result, *result_reversed, *v3min;
    v1 = psy_vector_new_xyz(1, 0, 0);
    v2 = psy_vector_new_xyz(0, 1, 0);
    v3 = psy_vector_new_xyz(0, 0, 1);
    v3min = psy_vector_negate(v3);

    result = psy_vector_cross(v1, v2);
    CU_ASSERT_TRUE(psy_vector_equals(v3, result));

    result_reversed = psy_vector_cross(v2, v1);
    CU_ASSERT_TRUE(psy_vector_equals(v3min, result_reversed));

    psy_vector_destroy(v1);
    psy_vector_destroy(v2);
    psy_vector_destroy(v3);
    psy_vector_destroy(v3min);
    psy_vector_destroy(result);
    psy_vector_destroy(result_reversed);
}

int add_vector_suite(void) {
    
    CU_Suite* suite = CU_add_suite("test reference count", NULL, NULL);
    CU_Test* test = NULL;
    
    if (!suite)
        return 1;

    test = CU_add_test(suite, "Vectors are created with sensible values", test_create);
    if (!test)
        return 1;

    test = CU_add_test(suite, "Vectors have a correct magnitude", test_magnitude);
    if (!test)
        return 1;

    test = CU_add_test(suite, "Vectors can create their own unit vector", test_unit);
    if (!test)
        return 1;

    test = CU_add_test(suite, "Vectors can be negated", test_negate);
    if (!test)
        return 1;

    test = CU_add_test(suite, "Test whether vector scalar addition works", test_add_scalar);
    if (!test)
        return 1;

    test = CU_add_test(suite, "Test whether vector addition works", test_add_vector);
    if (!test)
        return 1;
    
    test = CU_add_test(suite, "Test whether vector scalar subtraction works", test_sub_scalar);
    if (!test)
        return 1;

    test = CU_add_test(suite, "Test whether vector subtractions works", test_sub_vector);
    if (!test)
        return 1;
    
    test = CU_add_test(suite, "Test whether vector scalar multiplication works", test_mul_scalar);
    if (!test)
        return 1;
    
    test = CU_add_test(suite, "Test whether vector dot product works", test_vector_dot);
    if (!test)
        return 1;
    
    test = CU_add_test(suite, "Test whether vector cross product works", test_vector_cross);
    if (!test)
        return 1;

    return EXIT_SUCCESS;
}

