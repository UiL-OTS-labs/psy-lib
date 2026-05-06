
#include <math.h>
#include <string.h>

#include <criterion/criterion.h>
#include <criterion/new/assert.h>

#include "psylib.h"

Test(vector3, create)
{
    PsyVector3 *vec = psy_vector3_new();
    cr_expect(ne(vec, NULL), "vector3 instances can be created");
    psy_vector3_free(vec);

    gfloat data[3] = {1, 2, 3};
    gfloat x, y, z;
    vec = psy_vector3_new_data(3, data);
    g_object_get(vec, "x", &x, "y", &y, "z", &z, NULL);
    cr_expect(eq(x, data[0]));
    cr_expect(eq(y, data[1]));
    cr_expect(eq(z, data[2]));

    psy_vector3_free(vec);
}

Test(vector, magnitude)
{
    gfloat      x = 10, y = 20, z = 40;
    gfloat      values[3] = {x, y, z};
    PsyVector3 *vec       = psy_vector3_new_data(3, values);

    gfloat length, magnitude;
    gfloat expected = sqrt(x * x + y * y + z * z);
    magnitude       = psy_vector3_get_magnitude(vec);
    g_object_get(vec, "magnitude", &length, NULL);
    cr_expect(eq(magnitude, expected));
    cr_expect(eq(length, magnitude));

    g_object_unref(vec);
}

Test(vector3, unit)
{
    gfloat      x = 10, y = 20, z = 40;
    gfloat      length;
    PsyVector3 *vec
        = g_object_new(PSY_TYPE_VECTOR3, "x", x, "y", y, "z", z, NULL);
    PsyVector3 *unit = NULL;
    g_object_get(vec, "unit", &unit, NULL);

    length = psy_vector3_get_magnitude(unit);
    cr_expect(eq(length, 1.0f));
    psy_vector3_free(vec);
    psy_vector3_free(unit);

    vec  = psy_vector3_new();
    unit = psy_vector3_unit(vec);
    cr_expect(eq(unit, NULL),
              "It's not possible to retrieve a unit vector from a null vector");

    psy_vector3_free(vec);
}

Test(vector3, negate)
{
    gfloat      x = 10, y = 20, z = 40;
    gfloat      mx, my, mz;
    PsyVector3 *vec
        = g_object_new(PSY_TYPE_VECTOR3, "x", x, "y", y, "z", z, NULL);

    PsyVector3 *negated = psy_vector3_negate(vec);

    g_object_get(negated, "x", &mx, "y", &my, "z", &mz, NULL);

    psy_vector3_get_magnitude(vec);
    cr_expect(eq(mx, -10));
    cr_expect(eq(my, -20));
    cr_expect(eq(mz, -40));
    cr_expect(
        eq(psy_vector3_get_magnitude(vec), psy_vector3_get_magnitude(negated)));

    psy_vector3_free(vec);
    psy_vector3_free(negated);
}

Test(vector3, add_scalar)
{
    gfloat      x = 10, y = 20, z = 40;
    gfloat      scalar = 2;
    gfloat      rx, ry, rz;
    PsyVector3 *vec
        = g_object_new(PSY_TYPE_VECTOR3, "x", x, "y", y, "z", z, NULL);

    PsyVector3 *result = psy_vector3_add_s(vec, scalar);
    g_object_get(result, "x", &rx, "y", &ry, "z", &rz, NULL);
    cr_expect(eq(rx, x + scalar));
    cr_expect(eq(ry, y + scalar));
    cr_expect(eq(rz, z + scalar));

    psy_vector3_free(vec);
    psy_vector3_free(result);
}

Test(vector3, add_vector)
{
    gfloat      x = 10, y = 20, z = 40;
    PsyVector3 *v1
        = g_object_new(PSY_TYPE_VECTOR3, "x", x, "y", y, "z", z, NULL);
    PsyVector3 *v2
        = g_object_new(PSY_TYPE_VECTOR3, "x", x, "y", y, "z", z, NULL);
    PsyVector3 *result = psy_vector3_add(v1, v2);
    PsyVector3 *v3     = psy_vector3_mul_s(v1, 2.0);
    cr_expect(eq(psy_vector3_equals(result, v3), TRUE));

    psy_vector3_free(v1);
    psy_vector3_free(v2);
    psy_vector3_free(v3);
    psy_vector3_free(result);
}

Test(vector3, subtract_scalar)
{
    gfloat      x = 10, y = 20, z = 40;
    gfloat      scalar = 2;
    gfloat      rx, ry, rz;
    PsyVector3 *vec
        = g_object_new(PSY_TYPE_VECTOR3, "x", x, "y", y, "z", z, NULL);
    PsyVector3 *result = psy_vector3_sub_s(vec, scalar);
    g_object_get(result, "x", &rx, "y", &ry, "z", &rz, NULL);
    cr_expect(eq(rx, x - scalar));
    cr_expect(eq(ry, y - scalar));
    cr_expect(eq(rz, z - scalar));

    psy_vector3_free(vec);
    psy_vector3_free(result);
}

Test(vector3, sub_vector)
{
    gfloat      x = 10, y = 20, z = 40;
    PsyVector3 *v1
        = g_object_new(PSY_TYPE_VECTOR3, "x", x, "y", y, "z", z, NULL);
    PsyVector3 *result = psy_vector3_sub(v1, v1);
    cr_expect(eq(psy_vector3_is_null(result), TRUE));

    psy_vector3_free(v1);
    psy_vector3_free(result);
}

Test(vector3, multiply_scalar)
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

    cr_expect(eq(psy_vector3_equals(expected, result), TRUE));
    cr_expect(eq(psy_vector3_get_magnitude(v1) * scalar,
                 psy_vector3_get_magnitude(result)),
              "The magnitude is scaled");

    psy_vector3_free(v1);
    psy_vector3_free(result);
    psy_vector3_free(expected);
}

Test(vector3, vector3_dot_product)
{
    gfloat      x = 1, y = 1;
    PsyVector3 *v1, *v2;
    v1         = g_object_new(PSY_TYPE_VECTOR3, "x", x, NULL);
    v2         = g_object_new(PSY_TYPE_VECTOR3, "y", y, NULL);
    gfloat cos = psy_vector3_dot(v1, v2);
    cr_expect(eq(cos, 0.0f));

    cos = psy_vector3_dot(v2, v1);
    cr_expect(eq(cos, 0.0f));
    psy_vector3_free(v1);
    psy_vector3_free(v2);
}
