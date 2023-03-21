
#include <math.h>
#include <string.h>

#include <CUnit/CUnit.h>

#include <psylib.h>

static void
test_image_create1(void)
{
    const guint WIDTH = 1920, HEIGHT = 1080, NCHANNELS = 4;

    guint width, height, nchannels;
    gsize nbytes;
    guint stride;

    PsyImage *img = psy_image_new(WIDTH, HEIGHT, NCHANNELS);

    CU_ASSERT_PTR_NOT_NULL_FATAL(img);

    // clang-format off
    g_object_get(img,
            "width", &width,
            "height", &height,
            "num-channels", &nchannels,
            "num-bytes", &nbytes,
            "stride", &stride,
            NULL
            );
    // clang-format on

    CU_ASSERT_EQUAL(width, WIDTH);
    CU_ASSERT_EQUAL(height, HEIGHT);
    CU_ASSERT_EQUAL(nchannels, NCHANNELS);
    CU_ASSERT_EQUAL(nbytes, WIDTH * HEIGHT * NCHANNELS);
    CU_ASSERT_EQUAL(stride, WIDTH * NCHANNELS);

    g_object_unref(img);
}

static void
test_image_create2(void)
{
    const guint WIDTH = 1280, HEIGHT = 640, NCHANNELS = 3;

    guint width, height, nchannels;
    gsize nbytes;
    guint stride;

    PsyImage *img = psy_image_new(WIDTH, HEIGHT, NCHANNELS);

    CU_ASSERT_PTR_NOT_NULL_FATAL(img);

    // clang-format off
    g_object_get(img,
            "width", &width,
            "height", &height,
            "num-channels", &nchannels,
            "num-bytes", &nbytes,
            "stride", &stride,
            NULL
            );
    // clang-format on

    CU_ASSERT_EQUAL(width, WIDTH);
    CU_ASSERT_EQUAL(height, HEIGHT);
    CU_ASSERT_EQUAL(nchannels, NCHANNELS);
    CU_ASSERT_EQUAL(nbytes, WIDTH * HEIGHT * NCHANNELS);
    CU_ASSERT_EQUAL(stride, WIDTH * NCHANNELS);

    g_object_unref(img);
}

int
add_image_suite(void)
{
    CU_Suite *suite = CU_add_suite("PsyImage suite", NULL, NULL);
    CU_Test  *test;
    if (!suite)
        return 1;

    test = CU_add_test(suite, "Image Create1", test_image_create1);
    if (!test)
        return 1;

    test = CU_add_test(suite, "Image Create2", test_image_create2);
    if (!test)
        return 1;

    return 0;
}
