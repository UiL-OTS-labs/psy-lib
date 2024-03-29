
#include <math.h>
#include <string.h>

#include "unit-test-utilities.h"
#include <CUnit/CUnit.h>

#include <psylib.h>

static int
image_setup(void)
{
    set_log_handler_file("test-image.txt");
    return 0;
}

static int
image_teardown(void)
{
    set_log_handler_file(NULL);
    return 0;
}

static void
test_image_create1(void)
{
    const guint          WIDTH = 1920, HEIGHT = 1080, NCHANNELS = 4;
    const PsyImageFormat format = PSY_IMAGE_FORMAT_RGBA;

    guint          width, height, nchannels;
    gsize          nbytes;
    guint          stride;
    PsyImageFormat format_out;

    PsyImage *img = psy_image_new(WIDTH, HEIGHT, format);

    CU_ASSERT_PTR_NOT_NULL_FATAL(img);

    // clang-format off
    g_object_get(img,
            "width", &width,
            "height", &height,
            "num-channels", &nchannels,
            "num-bytes", &nbytes,
            "stride", &stride,
            "format", &format_out,
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
    const guint          WIDTH = 1280, HEIGHT = 640, NCHANNELS = 3;
    const PsyImageFormat format = PSY_IMAGE_FORMAT_RGB;

    guint          width, height, nchannels;
    gsize          nbytes;
    guint          stride;
    PsyImageFormat format_out;

    PsyImage *img = psy_image_new(WIDTH, HEIGHT, format);

    CU_ASSERT_PTR_NOT_NULL_FATAL(img);

    // clang-format off
    g_object_get(img,
            "width", &width,
            "height", &height,
            "num-channels", &nchannels,
            "num-bytes", &nbytes,
            "stride", &stride,
            "format", &format_out,
            NULL
            );
    // clang-format on

    CU_ASSERT_EQUAL(width, WIDTH);
    CU_ASSERT_EQUAL(height, HEIGHT);
    CU_ASSERT_EQUAL(nchannels, NCHANNELS);
    CU_ASSERT_EQUAL(nbytes, WIDTH * HEIGHT * NCHANNELS);
    CU_ASSERT_EQUAL(stride, WIDTH * NCHANNELS);
    CU_ASSERT_EQUAL(format_out, format);

    g_object_unref(img);
}

static void
test_image_change_format(void)
{
    const guint WIDTH = 1280, HEIGHT = 640;

    PsyImage *img = psy_image_new(WIDTH, HEIGHT, PSY_IMAGE_FORMAT_RGB);

    CU_ASSERT_PTR_NOT_NULL_FATAL(img);

    CU_ASSERT_EQUAL(psy_image_pixel_num_bytes(img), 3);
    CU_ASSERT_EQUAL(psy_image_get_num_bytes(img),
                    WIDTH * HEIGHT * psy_image_pixel_num_bytes(img));

    psy_image_set_format(img, PSY_IMAGE_FORMAT_RGBA);

    CU_ASSERT_EQUAL(psy_image_pixel_num_bytes(img), 4);
    CU_ASSERT_EQUAL(psy_image_get_num_bytes(img),
                    WIDTH * HEIGHT * psy_image_pixel_num_bytes(img));

    g_object_unref(img);
}

int
add_image_suite(void)
{
    CU_Suite *suite
        = CU_add_suite("PsyImage suite", image_setup, image_teardown);
    CU_Test *test;
    if (!suite)
        return 1;

    test = CU_add_test(suite, "Image Create1", test_image_create1);
    if (!test)
        return 1;

    test = CU_add_test(suite, "Image Create2", test_image_create2);
    if (!test)
        return 1;

    test = CU_ADD_TEST(suite, test_image_change_format);
    if (!test)
        return 1;

    return 0;
}
