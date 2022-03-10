
#include "../psy/ddd-vector4.h"
#include <math.h>

static void
test_create(void) {
    DddVector4* vec = ddd_vector4_new();
    g_assert(vec != NULL);
    ddd_vector4_destroy(vec);

    gdouble data[4] = {1, 2, 3, 4};
    gdouble x, y ,z, w;
    vec = ddd_vector4_new_data(4, data);
    g_assert(vec);
    g_object_get(vec, "x", &x, "y", &y, "z", &z, "w", &w, NULL);
    g_assert(x == data[0]);
    g_assert(y == data[1]);
    g_assert(z == data[2]);
    g_assert(w == data[3]);

    ddd_vector4_destroy(vec);
}

static void
test_magnitude(void)
{
    gdouble x = 10, y = 20, z = 40, w = 20;
    gdouble values[4] = {x, y, z, w};
    DddVector4* vec = ddd_vector4_new_data(4 , values);

    gdouble length, magnitude;
    gdouble expected = sqrt(x * x + y * y + z * z + w * w);
    magnitude = ddd_vector4_get_magnitude(vec);
    g_object_get(vec, "magnitude", &length, NULL);
    g_assert(magnitude == expected);
    g_assert(length == magnitude);

    g_object_unref(vec);
}

static void
test_unit(void) {
    gdouble x = 10, y = 20, z = 40;
    gdouble length;
    DddVector4 *vec  = g_object_new(DDD_TYPE_VECTOR4,
                                    "x", x,
                                    "y", y,
                                    "z", z,
                                    NULL
                                    );
    DddVector4 *unit = NULL;
    g_object_get(vec,
                 "unit", &unit,
                 NULL);
    g_assert(DDD_VECTOR4(unit));

    length = ddd_vector4_get_magnitude(unit);
    g_assert(length == 1.0f);
    ddd_vector4_destroy(vec);
    ddd_vector4_destroy(unit);

    vec = ddd_vector4_new();
    unit = ddd_vector4_normalize(vec);
    g_assert(unit == NULL);

    ddd_vector4_destroy(vec);
}

static void
test_negate(void)
{
    gdouble x = 10, y = 20, z = 40, w = 1;
    gdouble mx, my, mz, mw;
    DddVector4 *vec  = g_object_new(DDD_TYPE_VECTOR4,
                                    "x", x,
                                    "y", y,
                                    "z", z,
                                    "w", w,
                                    NULL);

    DddVector4 *negated = ddd_vector4_negate(vec);

    g_object_get(negated,
                 "x", &mx,
                 "y", &my,
                 "z", &mz,
                 "w", &mw,
                 NULL
                 );

    ddd_vector4_get_magnitude(vec);
    g_assert(mx == -x && my == -y && mz == -z);
    g_assert(ddd_vector4_get_magnitude(vec) == ddd_vector4_get_magnitude(negated));

    ddd_vector4_destroy(vec);
    ddd_vector4_destroy(negated);
}

static void
test_add_scalar(void)
{
    gdouble x = 10, y = 20, z = 40, w = -50;
    gdouble scalar = 2;
    gdouble rx, ry, rz, rw;
    DddVector4 *vec  = g_object_new(DDD_TYPE_VECTOR4,
                                    "x", x,
                                    "y", y,
                                    "z", z,
                                    "w", w,
                                    NULL);

    DddVector4 *result = ddd_vector4_add_s(vec, scalar);
    g_object_get(result,
                 "x", &rx,
                 "y", &ry,
                 "z", &rz,
                 "w", &rw,
                 NULL
                 );
    g_assert(rx == x + scalar && ry == y + scalar &&
             rz == z + scalar && rw == w + scalar
             );

    ddd_vector4_destroy(vec);
    ddd_vector4_destroy(result);
}

static void
test_add_vector(void)
{
    gdouble x = 10, y = 20, z = 40, w = -50;
    DddVector4 *v1  = g_object_new(DDD_TYPE_VECTOR4,
                                   "x", x,
                                   "y", y,
                                   "z", z,
                                   "w", w,
                                   NULL);
    DddVector4 *v2  = g_object_new(DDD_TYPE_VECTOR4,
                                   "x", x,
                                   "y", y,
                                   "z", z,
                                   "w", w,
                                   NULL
                                   );
    DddVector4 *result = ddd_vector4_add(v1, v2);
    DddVector4 *v3  = ddd_vector4_mul_s(v1, 2.0);
    g_assert(ddd_vector4_equals(result, v3));

    ddd_vector4_destroy(v1);
    ddd_vector4_destroy(v2);
    ddd_vector4_destroy(v3);
    ddd_vector4_destroy(result);
}

static void
test_sub_scalar(void)
{
    gdouble x = 10, y = 20, z = 40, w = -50;
    gdouble scalar = 2;
    gdouble rx, ry, rz, rw;
    DddVector4 *vec  = g_object_new(DDD_TYPE_VECTOR4,
                                    "x", x,
                                    "y", y,
                                    "z", z,
                                    "w", w,
                                    NULL);
    DddVector4 *result = ddd_vector4_sub_s(vec, scalar);
    g_object_get(result,
                 "x", &rx,
                 "y", &ry,
                 "z", &rz,
                 "w", &rw,
                 NULL
                 );
    g_assert(rx == x - scalar && ry == y - scalar &&
             rz == z - scalar && rw == w - scalar);

    ddd_vector4_destroy(vec);
    ddd_vector4_destroy(result);
}

static void
test_sub_vector(void)
{
    gdouble x = 10, y = 20, z = 40, w = -40;
    DddVector4 *v1  = g_object_new(DDD_TYPE_VECTOR4,
                                   "x", x,
                                   "y", y,
                                   "z", z,
                                   "w", w,
                                   NULL);
    DddVector4 *result = ddd_vector4_sub(v1, v1);
    g_assert(ddd_vector4_is_null(result));

    ddd_vector4_destroy(v1);
    ddd_vector4_destroy(result);
}

static void
test_mul_scalar(void)
{
    gdouble x = 10, y = 20, z = 40, w = -50;
    gdouble scalar = 2.0;
    DddVector4 *v1  = g_object_new(DDD_TYPE_VECTOR4,
                                   "x", x,
                                   "y", y,
                                   "z", z,
                                   "w", w,
                                   NULL);
    DddVector4 *result = ddd_vector4_mul_s(v1, scalar);
    DddVector4 *expected = g_object_new(DDD_TYPE_VECTOR4,
            "x", x * scalar,
            "y", y * scalar,
            "z", z * scalar,
            "w", w * scalar,
            NULL);
    g_assert(ddd_vector4_equals(expected, result));
    ddd_vector4_destroy(v1);
    ddd_vector4_destroy(result);
    ddd_vector4_destroy(expected);
}

static void
test_vector_dot(void)
{
    gdouble x = 1, y = 1;
    DddVector4 *v1, *v2;
    v1 = g_object_new(DDD_TYPE_VECTOR4,"x", x, NULL);
    v2 = g_object_new(DDD_TYPE_VECTOR4,"y", y, NULL);
    gdouble cos = ddd_vector4_dot(v1, v2);
    g_assert(cos == 0.0f);

    cos = ddd_vector4_dot(v2, v1);
    g_assert(cos == 0);
    ddd_vector4_destroy(v1);
    ddd_vector4_destroy(v2);
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

    return EXIT_SUCCESS;
}
