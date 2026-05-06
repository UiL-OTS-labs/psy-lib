
#include "psy-vector4.h"
#include <math.h>
#include <string.h>

#include <criterion/criterion.h>
#include <criterion/new/assert.h>

#include <psylib.h>

Test(vector4, create)
{
    PsyVector4 *vec = psy_vector4_new();
    cr_assert(ne(vec, NULL), "PsyVector4 can be instantiated");
    psy_vector4_free(vec);

    gfloat data[4] = {1, 2, 3, 4};
    gfloat x, y, z, w;
    vec = psy_vector4_new_data(4, data);
    g_object_get(vec, "x", &x, "y", &y, "z", &z, "w", &w, NULL);
    cr_expect(eq(x, data[0]));
    cr_expect(eq(y, data[1]));
    cr_expect(eq(z, data[2]));
    cr_expect(eq(w, data[3]));

    psy_vector4_free(vec);
}

Test(vector4, magnitude)
{
    gfloat      x = 10, y = 20, z = 40, w = 20;
    gfloat      values[4] = {x, y, z, w};
    PsyVector4 *vec       = psy_vector4_new_data(4, values);

    gfloat length, magnitude;
    gfloat expected = sqrtf(x * x + y * y + z * z + w * w);
    magnitude       = psy_vector4_get_magnitude(vec);
    g_object_get(vec, "magnitude", &length, NULL);
    cr_expect(eq(magnitude, expected));
    cr_expect(eq(length, magnitude));

    g_object_unref(vec);
}

Test(vector4, unit)
{
    gfloat      x = 10, y = 20, z = 40;
    gfloat      length;
    PsyVector4 *vec
        = g_object_new(PSY_TYPE_VECTOR4, "x", x, "y", y, "z", z, NULL);
    PsyVector4 *unit = NULL;
    g_object_get(vec, "unit", &unit, NULL);

    length = psy_vector4_get_magnitude(unit);
    cr_expect(eq(length, 1.0f));
    psy_vector4_free(vec);
    psy_vector4_free(unit);

    vec  = psy_vector4_new();
    unit = psy_vector4_unit(vec);
    cr_expect(zero(unit),
              "A null vectors doesn't have a direction, so no unit vector is "
              "possible");

    psy_vector4_free(vec);
}

Test(vector4, negate)
{
    gfloat      x = 10, y = 20, z = 40, w = 1;
    gfloat      mx, my, mz, mw;
    PsyVector4 *vec
        = g_object_new(PSY_TYPE_VECTOR4, "x", x, "y", y, "z", z, "w", w, NULL);

    PsyVector4 *negated = psy_vector4_negate(vec);

    g_object_get(negated, "x", &mx, "y", &my, "z", &mz, "w", &mw, NULL);

    psy_vector4_get_magnitude(vec);
    cr_expect(eq(mx, -x));
    cr_expect(eq(my, -y));
    cr_expect(eq(mz, -z));
    cr_expect(
        eq(psy_vector4_get_magnitude(vec), psy_vector4_get_magnitude(negated)));

    psy_vector4_free(vec);
    psy_vector4_free(negated);
}

Test(vector4, add_scalar)
{
    gfloat      x = 10, y = 20, z = 40, w = -50;
    gfloat      scalar = 2;
    gfloat      rx, ry, rz, rw;
    PsyVector4 *vec
        = g_object_new(PSY_TYPE_VECTOR4, "x", x, "y", y, "z", z, "w", w, NULL);

    PsyVector4 *result = psy_vector4_add_s(vec, scalar);
    g_object_get(result, "x", &rx, "y", &ry, "z", &rz, "w", &rw, NULL);
    cr_expect(eq(rx, x + scalar));
    cr_expect(eq(ry, y + scalar));
    cr_expect(eq(rz, z + scalar));
    cr_expect(eq(rw, w + scalar));

    psy_vector4_free(vec);
    psy_vector4_free(result);
}

Test(vector4, add_vector)
{
    gfloat      x = 10, y = 20, z = 40, w = -50;
    PsyVector4 *v1
        = g_object_new(PSY_TYPE_VECTOR4, "x", x, "y", y, "z", z, "w", w, NULL);
    PsyVector4 *v2
        = g_object_new(PSY_TYPE_VECTOR4, "x", x, "y", y, "z", z, "w", w, NULL);
    PsyVector4 *result = psy_vector4_add(v1, v2);
    PsyVector4 *v3     = psy_vector4_mul_s(v1, 2.0f);
    cr_expect(eq(psy_vector4_equals(result, v3), TRUE));

    psy_vector4_free(v1);
    psy_vector4_free(v2);
    psy_vector4_free(v3);
    psy_vector4_free(result);
}

Test(vector4, subtract_scalar)
{
    gfloat      x = 10, y = 20, z = 40, w = -50;
    gfloat      scalar = 2;
    gfloat      rx, ry, rz, rw;
    PsyVector4 *vec
        = g_object_new(PSY_TYPE_VECTOR4, "x", x, "y", y, "z", z, "w", w, NULL);
    PsyVector4 *result = psy_vector4_sub_s(vec, scalar);
    g_object_get(result, "x", &rx, "y", &ry, "z", &rz, "w", &rw, NULL);
    cr_expect(eq(rx, x - scalar));
    cr_expect(eq(ry, y - scalar));
    cr_expect(eq(rz, z - scalar));
    cr_expect(eq(rw, w - scalar));

    psy_vector4_free(vec);
    psy_vector4_free(result);
}

Test(vector4, sub_vector)
{
    gfloat      x = 10, y = 20, z = 40, w = -40;
    PsyVector4 *v1
        = g_object_new(PSY_TYPE_VECTOR4, "x", x, "y", y, "z", z, "w", w, NULL);
    PsyVector4 *result = psy_vector4_sub(v1, v1);
    cr_expect(eq(psy_vector4_is_null(result), TRUE));

    psy_vector4_free(v1);
    psy_vector4_free(result);
}

Test(vector4, mul_scalar)
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

    cr_expect(eq(psy_vector4_equals(expected, result), TRUE));
    cr_expect(eq(flt,
                 psy_vector4_get_magnitude(v1) * 2.0f,
                 psy_vector4_get_magnitude(result)));

    psy_vector4_free(v1);
    psy_vector4_free(result);
    psy_vector4_free(expected);
}

Test(vector4, vector_dot_product)
{
    gfloat      x = 1, y = 1;
    PsyVector4 *v1, *v2;
    v1         = g_object_new(PSY_TYPE_VECTOR4, "x", x, NULL);
    v2         = g_object_new(PSY_TYPE_VECTOR4, "y", y, NULL);
    gfloat cos = psy_vector4_dot(v1, v2);
    cr_expect(eq(cos, 0.0f));

    cos = psy_vector4_dot(v2, v1);
    cr_expect(eq(cos, 0.0f));
    psy_vector4_free(v1);
    psy_vector4_free(v2);
}
