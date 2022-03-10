
#include "../psy/ddd-vector.h"
#include <math.h>

static void
test_create(void) {
    DddVector* vec = ddd_vector_new();
    g_assert(vec != NULL);
    gfloat x, y ,z;

    ddd_vector_destroy(vec);
    vec  = ddd_vector_new_x(1.0);
    g_assert(vec);
    g_object_get(vec, "x", &x, "y", &y, "z", &z, NULL);
    g_assert(x == 1);
    g_assert(y == 0);
    g_assert(z == 0);
    ddd_vector_destroy(vec);

    vec = ddd_vector_new_xy(10, 10);
    g_object_get(vec, "x", &x, "y", &y, "z", &z, NULL);
    g_object_get(vec, "x", &x, "y", &y, "z", &z, NULL);
    g_assert(x == 10);
    g_assert(y == 10);
    g_assert(z == 0);
    ddd_vector_destroy(vec);

    vec = ddd_vector_new_xyz(100, 100, 100);
    g_object_get(vec, "x", &x, "y", &y, "z", &z, NULL);
    g_object_get(vec, "x", &x, "y", &y, "z", &z, NULL);
    g_assert(x == 100);
    g_assert(y == 100);
    g_assert(z == 100);

    ddd_vector_destroy(vec);
}

static void
test_magnitude(void)
{
    gfloat x = 10, y = 20, z = 40;
    DddVector* vec = ddd_vector_new_xyz(x, y, z);
    gfloat length, magnitude;
    gfloat expected = sqrtf(x * x + y * y + z * z);
    magnitude = ddd_vector_magnitude(vec);
    g_object_get(vec, "length", &length, NULL);
    g_assert(magnitude == expected);
    g_assert(length == magnitude);

    g_object_unref(vec);
}

static void
test_unit(void) {
    gfloat x = 10, y = 20, z = 40;
    gfloat length;
    DddVector *vec  = ddd_vector_new_xyz(x, y, z);
    DddVector *unit = ddd_vector_unit(vec);

    length = ddd_vector_magnitude(unit);
    g_assert(length == 1.0f);
    ddd_vector_destroy(vec);
    ddd_vector_destroy(unit);

    vec = ddd_vector_new();
    unit = ddd_vector_unit(vec);
    g_assert(unit == NULL);
    ddd_vector_destroy(vec);
}

static void
test_negate(void)
{
    gfloat x = 10, y = 20, z = 40;
    gfloat mx, my, mz;
    DddVector *vec  = ddd_vector_new_xyz(x, y, z);
    DddVector *negated = ddd_vector_negate(vec);

    g_object_get(negated,
                 "x", &mx,
                 "y", &my,
                 "z", &mz,
                 NULL
                 );

    ddd_vector_magnitude(vec);
    g_assert(mx == -x && my == -y && mz == -z);
    g_assert(ddd_vector_magnitude(vec) == ddd_vector_magnitude(negated));

    ddd_vector_destroy(vec);
    ddd_vector_destroy(negated);
}

static void
test_add_scalar(void)
{
    gfloat x = 10, y = 20, z = 40;
    gfloat scalar = 2;
    gfloat rx, ry, rz;
    DddVector *vec  = ddd_vector_new_xyz(x, y, z);
    DddVector *result = ddd_vector_add_s(vec, scalar);
    g_object_get(result,
                 "x", &rx,
                 "y", &ry,
                 "z", &rz,
                 NULL
                 );
    g_assert(rx == x + scalar && ry == y + scalar && rz == z + scalar);

    ddd_vector_destroy(vec);
    ddd_vector_destroy(result);
}

static void
test_add_vector(void)
{
    gfloat x = 10, y = 20, z = 40;
    DddVector *v1  = ddd_vector_new_xyz(x, y, z);
    DddVector *v2  = ddd_vector_new_xyz(x, y, z);
    DddVector *result = ddd_vector_add(v1, v2);
    DddVector *v3  = ddd_vector_mul_s(v1, 2.0f);
    g_assert(ddd_vector_equals(result, v3));

    ddd_vector_destroy(v1);
    ddd_vector_destroy(v2);
    ddd_vector_destroy(v3);
    ddd_vector_destroy(result);
}

static void
test_sub_scalar(void)
{
    gfloat x = 10, y = 20, z = 40;
    gfloat scalar = 2;
    gfloat rx, ry, rz;
    DddVector *vec  = ddd_vector_new_xyz(x, y, z);
    DddVector *result = ddd_vector_sub_s(vec, scalar);
    g_object_get(result,
                 "x", &rx,
                 "y", &ry,
                 "z", &rz,
                 NULL
    );
    g_assert(rx == x - scalar && ry == y - scalar && rz == z - scalar);

    ddd_vector_destroy(vec);
    ddd_vector_destroy(result);
}

static void
test_sub_vector(void)
{
    gfloat x = 10, y = 20, z = 40;
    DddVector *v1  = ddd_vector_new_xyz(x, y, z);
    DddVector *v2  = ddd_vector_new_xyz(x, y, z);
    DddVector *result = ddd_vector_sub(v1, v2);
    DddVector *expected = ddd_vector_new();
    g_assert(ddd_vector_equals(result, expected));

    ddd_vector_destroy(v1);
    ddd_vector_destroy(v2);
    ddd_vector_destroy(expected);
    ddd_vector_destroy(result);
}

static void
test_mul_scalar(void)
{
    gfloat x = 10, y = 20, z = 40;
    gfloat scalar = 2.0;
    DddVector *v1  = ddd_vector_new_xyz(x, y, z);
    DddVector *result = ddd_vector_mul_s(v1, scalar);
    DddVector *expected = ddd_vector_new_xyz(
            x * scalar,
            y * scalar,
            z * scalar);
    g_assert(ddd_vector_equals(expected, result));
    ddd_vector_destroy(v1);
    ddd_vector_destroy(result);
    ddd_vector_destroy(expected);
}

static void
test_vector_dot(void)
{
    DddVector *v1, *v2;
    v1 = ddd_vector_new_xyz(1,0,0);
    v2 = ddd_vector_new_xyz(0,1,0);
    gfloat cos = ddd_vector_dot(v1, v2);
    g_assert(cos == 0.0f);

    cos = ddd_vector_dot(v2, v1);
    g_assert(cos == 0);
    ddd_vector_destroy(v1);
    ddd_vector_destroy(v2);
}

static void
test_vector_cross(void)
{
    DddVector *v1, *v2, *v3, *result, *result_reversed, *v3min;
    v1 = ddd_vector_new_xyz(1,0,0);
    v2 = ddd_vector_new_xyz(0,1,0);
    v3 = ddd_vector_new_xyz(0, 0, 1);
    v3min = ddd_vector_negate(v3);

    result = ddd_vector_cross(v1, v2);
    g_assert(ddd_vector_equals(v3, result));

    result_reversed = ddd_vector_cross(v2, v1);
    g_assert(ddd_vector_equals(v3min, result_reversed));

    ddd_vector_destroy(v1);
    ddd_vector_destroy(v2);
    ddd_vector_destroy(v3);
    ddd_vector_destroy(v3min);
    ddd_vector_destroy(result);
    ddd_vector_destroy(result_reversed);
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
