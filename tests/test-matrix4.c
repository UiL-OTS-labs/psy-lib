
#include <math.h>
#include <string.h>

#include <psylib.h>

static void
test_matrix4_create(void)
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

    g_assert_true(is_null);
    g_assert_false(is_identity);

    psy_matrix4_free(mat);
}

static void
test_matrix4_create_identity(void)
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

    g_assert_false(is_null);
    g_assert_true(is_identity);

    psy_matrix4_free(mat);
}

static void
test_matrix4_setable_props(void)
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

    g_assert_true(is_identity);
    g_assert_false(is_null);

    g_object_set(mat, "is-null", TRUE, NULL);
    // clang-format off
    g_object_get(mat,
            "is-identity", &is_identity,
            "is-null", &is_null,
            NULL);
    // clang-format off
    
    g_assert_false(is_identity);
    g_assert_true(is_null);

    psy_matrix4_free(mat);
}

int main(int argc, char** argv) {
    g_test_init(&argc, &argv, NULL);

    g_test_add_func("/matrix4/test_matrix4", test_matrix4_create);
    g_test_add_func("/matrix4/create_itentity", test_matrix4_create_identity);
    g_test_add_func("/matrix4/setable_props", test_matrix4_setable_props);

    return g_test_run();
}
