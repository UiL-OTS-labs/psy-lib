
#include "../psy/psy-vector4.h"
#include "psy-vector.h"
#include <linux/rtnetlink.h>
#include <math.h>
#include <string.h>

static void
test_create(void) {
    PsyVector4* vec = psy_vector4_new();
    g_assert(vec != NULL);
    psy_vector4_destroy(vec);

    gdouble data[4] = {1, 2, 3, 4};
    gdouble x, y ,z, w;
    vec = psy_vector4_new_data(4, data);
    g_assert(vec);
    g_object_get(vec, "x", &x, "y", &y, "z", &z, "w", &w, NULL);
    g_assert(x == data[0]);
    g_assert(y == data[1]);
    g_assert(z == data[2]);
    g_assert(w == data[3]);

    psy_vector4_destroy(vec);
}

static void
test_magnitude(void)
{
    gdouble x = 10, y = 20, z = 40, w = 20;
    gdouble values[4] = {x, y, z, w};
    PsyVector4* vec = psy_vector4_new_data(4 , values);

    gdouble length, magnitude;
    gdouble expected = sqrt(x * x + y * y + z * z + w * w);
    magnitude = psy_vector4_get_magnitude(vec);
    g_object_get(vec, "magnitude", &length, NULL);
    g_assert(magnitude == expected);
    g_assert(length == magnitude);

    g_object_unref(vec);
}

static void
test_unit(void) {
    gdouble x = 10, y = 20, z = 40;
    gdouble length;
    PsyVector4 *vec  = g_object_new(PSY_TYPE_VECTOR4,
                                    "x", x,
                                    "y", y,
                                    "z", z,
                                    NULL
                                    );
    PsyVector4 *unit = NULL;
    g_object_get(vec,
                 "unit", &unit,
                 NULL);
    g_assert(PSY_VECTOR4(unit));

    length = psy_vector4_get_magnitude(unit);
    g_assert(length == 1.0f);
    psy_vector4_destroy(vec);
    psy_vector4_destroy(unit);

    vec = psy_vector4_new();
    unit = psy_vector4_unit(vec);
    g_assert(unit == NULL);

    psy_vector4_destroy(vec);
}

static void
test_negate(void)
{
    gdouble x = 10, y = 20, z = 40, w = 1;
    gdouble mx, my, mz, mw;
    PsyVector4 *vec  = g_object_new(PSY_TYPE_VECTOR4,
                                    "x", x,
                                    "y", y,
                                    "z", z,
                                    "w", w,
                                    NULL);

    PsyVector4 *negated = psy_vector4_negate(vec);

    g_object_get(negated,
                 "x", &mx,
                 "y", &my,
                 "z", &mz,
                 "w", &mw,
                 NULL
                 );

    psy_vector4_get_magnitude(vec);
    g_assert(mx == -x && my == -y && mz == -z);
    g_assert(psy_vector4_get_magnitude(vec) == psy_vector4_get_magnitude(negated));

    psy_vector4_destroy(vec);
    psy_vector4_destroy(negated);
}

static void
test_add_scalar(void)
{
    gdouble x = 10, y = 20, z = 40, w = -50;
    gdouble scalar = 2;
    gdouble rx, ry, rz, rw;
    PsyVector4 *vec  = g_object_new(PSY_TYPE_VECTOR4,
                                    "x", x,
                                    "y", y,
                                    "z", z,
                                    "w", w,
                                    NULL);

    PsyVector4 *result = psy_vector4_add_s(vec, scalar);
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

    psy_vector4_destroy(vec);
    psy_vector4_destroy(result);
}

static void
test_add_vector(void)
{
    gdouble x = 10, y = 20, z = 40, w = -50;
    PsyVector4 *v1  = g_object_new(PSY_TYPE_VECTOR4,
                                   "x", x,
                                   "y", y,
                                   "z", z,
                                   "w", w,
                                   NULL);
    PsyVector4 *v2  = g_object_new(PSY_TYPE_VECTOR4,
                                   "x", x,
                                   "y", y,
                                   "z", z,
                                   "w", w,
                                   NULL
                                   );
    PsyVector4 *result = psy_vector4_add(v1, v2);
    PsyVector4 *v3  = psy_vector4_mul_s(v1, 2.0);
    g_assert(psy_vector4_equals(result, v3));

    psy_vector4_destroy(v1);
    psy_vector4_destroy(v2);
    psy_vector4_destroy(v3);
    psy_vector4_destroy(result);
}

static void
test_sub_scalar(void)
{
    gdouble x = 10, y = 20, z = 40, w = -50;
    gdouble scalar = 2;
    gdouble rx, ry, rz, rw;
    PsyVector4 *vec  = g_object_new(PSY_TYPE_VECTOR4,
                                    "x", x,
                                    "y", y,
                                    "z", z,
                                    "w", w,
                                    NULL);
    PsyVector4 *result = psy_vector4_sub_s(vec, scalar);
    g_object_get(result,
                 "x", &rx,
                 "y", &ry,
                 "z", &rz,
                 "w", &rw,
                 NULL
                 );
    g_assert(rx == x - scalar && ry == y - scalar &&
             rz == z - scalar && rw == w - scalar);

    psy_vector4_destroy(vec);
    psy_vector4_destroy(result);
}

static void
test_sub_vector(void)
{
    gdouble x = 10, y = 20, z = 40, w = -40;
    PsyVector4 *v1  = g_object_new(PSY_TYPE_VECTOR4,
                                   "x", x,
                                   "y", y,
                                   "z", z,
                                   "w", w,
                                   NULL);
    PsyVector4 *result = psy_vector4_sub(v1, v1);
    g_assert(psy_vector4_is_null(result));

    psy_vector4_destroy(v1);
    psy_vector4_destroy(result);
}

static void
test_mul_scalar(void)
{
    gdouble x = 10, y = 20, z = 40, w = -50;
    gdouble scalar = 2.0;
    PsyVector4 *v1  = g_object_new(PSY_TYPE_VECTOR4,
                                   "x", x,
                                   "y", y,
                                   "z", z,
                                   "w", w,
                                   NULL);
    PsyVector4 *result = psy_vector4_mul_s(v1, scalar);
    PsyVector4 *expected = g_object_new(PSY_TYPE_VECTOR4,
            "x", x * scalar,
            "y", y * scalar,
            "z", z * scalar,
            "w", w * scalar,
            NULL);
    g_assert(psy_vector4_equals(expected, result));
    psy_vector4_destroy(v1);
    psy_vector4_destroy(result);
    psy_vector4_destroy(expected);
}

static void
test_vector_dot(void)
{
    gdouble x = 1, y = 1;
    PsyVector4 *v1, *v2;
    v1 = g_object_new(PSY_TYPE_VECTOR4,"x", x, NULL);
    v2 = g_object_new(PSY_TYPE_VECTOR4,"y", y, NULL);
    gdouble cos = psy_vector4_dot(v1, v2);
    g_assert(cos == 0.0f);

    cos = psy_vector4_dot(v2, v1);
    g_assert(cos == 0);
    psy_vector4_destroy(v1);
    psy_vector4_destroy(v2);
}

//static void
//test_vector_property_values(void)
//{
//    GArray *copy = NULL, *list = g_array_sized_new(FALSE, FALSE, sizeof(gdouble), 4);
//    double values[4] =  {0.0, 2.0, 3.0, 4.0};
//    g_array_append_vals(list, values, 4);
//
//    PsyVector4* vec = g_object_new(
//            PSY_TYPE_VECTOR4,
//            "values", list,
//            NULL);
//    PsyVector4* similar = psy_vector4_new_data(4, values);
//
//    gboolean the_same = psy_vector4_equals(vec, similar);
//    g_assert(the_same);
//
//    g_object_get(vec, "values", &copy, NULL);
//
//    the_same = memcmp(copy->data, values, 4) == 0;
//    g_assert(the_same);
//    g_array_unref(copy);
//    g_array_unref(list);
//
//    psy_vector4_destroy(vec);
//    psy_vector4_destroy(similar);
//}


int
main(void) {

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
//    test_vector_property_values();

    return EXIT_SUCCESS;
}
