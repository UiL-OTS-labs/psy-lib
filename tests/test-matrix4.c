
#include <math.h>
#include <string.h>

#include <criterion/criterion.h>
#include <criterion/new/assert.h>

#include <psylib.h>

Test(matrix4, create)
{
    PsyMatrix4 *mat = psy_matrix4_new();

    gboolean is_null;
    gboolean is_identity;

    // clang-format off
    g_object_get(mat,
            "is-null", &is_null,
            "is-identity", &is_identity,
            NULL);
    // clang-format on

    cr_assert(is_null);
    cr_assert(ne(is_identity, TRUE));

    psy_matrix4_free(mat);
}

Test(matrix4, create_identity)
{
    PsyMatrix4 *mat = psy_matrix4_new_identity();

    gboolean is_null;
    gboolean is_identity;

    // clang-format off
    g_object_get(mat,
            "is-null", &is_null,
            "is-identity", &is_identity,
            NULL);
    // clang-format on

    cr_expect(zero(is_null));
    cr_expect(eq(is_identity, TRUE));

    psy_matrix4_free(mat);
}

Test(matrix4, setable_props)
{
    PsyMatrix4 *mat = psy_matrix4_new();

    gboolean is_identity;
    gboolean is_null;

    g_object_set(mat, "is-identity", TRUE, NULL);
    // clang-format off
    g_object_get(mat,
            "is-identity", &is_identity,
            "is-null", &is_null,
            NULL);
    // clang-format on

    cr_expect(eq(is_identity, TRUE));
    cr_expect(eq(is_null, FALSE));

    g_object_set(mat, "is-null", TRUE, NULL);
    // clang-format off
    g_object_get(mat,
            "is-identity", &is_identity,
            "is-null", &is_null,
            NULL);
    // clang-format off
    
    cr_expect(eq(is_identity, FALSE));
    cr_expect(eq(is_null, TRUE));

    psy_matrix4_free(mat);
}
