
#include "../psy/psy-vector.h"
#include <math.h>

static void
test_create(void) {
    PsyVector* vec = psy_vector_new();
    g_assert(vec != NULL);
    gfloat x, y ,z;

    psy_vector_destroy(vec);
    vec  = psy_vector_new_x(1.0);
    g_assert(vec);
    g_object_get(vec, "x", &x, "y", &y, "z", &z, NULL);
    g_assert(x == 1);
    g_assert(y == 0);
    g_assert(z == 0);
    psy_vector_destroy(vec);

    vec = psy_vector_new_xy(10, 10);
    g_object_get(vec, "x", &x, "y", &y, "z", &z, NULL);
    g_object_get(vec, "x", &x, "y", &y, "z", &z, NULL);
    g_assert(x == 10);
    g_assert(y == 10);
    g_assert(z == 0);
    psy_vector_destroy(vec);

    vec = psy_vector_new_xyz(100, 100, 100);
    g_object_get(vec, "x", &x, "y", &y, "z", &z, NULL);
    g_object_get(vec, "x", &x, "y", &y, "z", &z, NULL);
    g_assert(x == 100);
    g_assert(y == 100);
    g_assert(z == 100);

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
    g_assert(magnitude == expected);
    g_assert(length == magnitude);

    g_object_unref(vec);
}

static void
test_unit(void) {
    gfloat x = 10, y = 20, z = 40;
    gfloat length;
    PsyVector *vec  = psy_vector_new_xyz(x, y, z);
    PsyVector *unit = psy_vector_unit(vec);

    length = psy_vector_magnitude(unit);
    g_assert(length == 1.0f);
    psy_vector_destroy(vec);
    psy_vector_destroy(unit);

    vec = psy_vector_new();
    unit = psy_vector_unit(vec);
    g_assert(unit == NULL);
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
    g_assert(psy_vector_magnitude(vec) == psy_vector_magnitude(negated));

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
    g_assert(rx == x + scalar && ry == y + scalar && rz == z + scalar);

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
    g_assert(psy_vector_equals(result, v3));

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
    g_assert(rx == x - scalar && ry == y - scalar && rz == z - scalar);

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
    g_assert(psy_vector_equals(result, expected));

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
    g_assert(psy_vector_equals(expected, result));
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
    g_assert(cos == 0.0f);

    cos = psy_vector_dot(v2, v1);
    g_assert(cos == 0);
    psy_vector_destroy(v1);
    psy_vector_destroy(v2);
}

static void
test_vector_cross(void)
{
    PsyVector *v1, *v2, *v3, *result, *result_reversed, *v3min;
    v1 = psy_vector_new_xyz(1,0,0);
    v2 = psy_vector_new_xyz(0,1,0);
    v3 = psy_vector_new_xyz(0, 0, 1);
    v3min = psy_vector_negate(v3);

    result = psy_vector_cross(v1, v2);
    g_assert(psy_vector_equals(v3, result));

    result_reversed = psy_vector_cross(v2, v1);
    g_assert(psy_vector_equals(v3min, result_reversed));

    psy_vector_destroy(v1);
    psy_vector_destroy(v2);
    psy_vector_destroy(v3);
    psy_vector_destroy(v3min);
    psy_vector_destroy(result);
    psy_vector_destroy(result_reversed);
}

int main() {

    test_create();
    test_magnitude();
    test_unit();
    test_negate();
    test_add_scalar();
    test_add_vector();
    test_sub_scalar();
    test_sub_vector();
    test_mul_scalar();
    test_vector_dot();
    test_vector_cross();

    return EXIT_SUCCESS;
}
