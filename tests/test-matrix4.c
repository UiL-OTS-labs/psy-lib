
#include <math.h>
#include <string.h>

#include <CUnit/CUnit.h>

#include <psylib.h>

static void
matrix4_create(void)
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

    CU_ASSERT_TRUE(is_null);
    CU_ASSERT_FALSE(is_identity);

    psy_matrix4_free(mat);
}

static void
matrix4_create_identity(void)
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

    CU_ASSERT_FALSE(is_null);
    CU_ASSERT_TRUE(is_identity);

    psy_matrix4_free(mat);
}

static void
matrix4_setable_props(void)
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

    CU_ASSERT_TRUE(is_identity);
    CU_ASSERT_FALSE(is_null);

    g_object_set(mat, "is-null", TRUE, NULL);
    // clang-format off
    g_object_get(mat,
            "is-identity", &is_identity,
            "is-null", &is_null,
            NULL);
    // clang-format off
    
    CU_ASSERT_FALSE(is_identity);
    CU_ASSERT_TRUE(is_null);

    psy_matrix4_free(mat);
}

int
add_matrix4_suite(void)
{
    CU_Suite *suite = CU_add_suite("PsyMatrix4 suite", NULL, NULL);
    printf("Yes, it's added");
    CU_Test  *test;
    if (!suite)
        return 1;

    test = CU_ADD_TEST(suite, matrix4_create);
    if (!test)
        return 1;

    test = CU_ADD_TEST(suite, matrix4_create_identity);
    if (!test)
        return 1;
    
    test = CU_ADD_TEST(suite, matrix4_setable_props);
    if (!test)
        return 1;

    return 0;
}
