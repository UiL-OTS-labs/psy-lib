
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

static void
test_image_clear(void)
{
    const guint WIDTH = 100, HEIGHT = 100;

    PsyImage *img = psy_image_new(WIDTH, HEIGHT, PSY_IMAGE_FORMAT_RGB);
    CU_ASSERT_PTR_NOT_NULL_FATAL(img);
    PsyColor *bg = psy_color_new_rgb(random_double_range(0, 1),
                                     random_double_range(0, 1),
                                     random_double_range(0, 1));
    CU_ASSERT_PTR_NOT_NULL_FATAL(bg);

    psy_image_clear(img, bg);

    PsyColor *probe = psy_image_get_pixel(
        img, random_int_range(0, WIDTH), random_int_range(0, HEIGHT));

    CU_ASSERT_TRUE(psy_color_equal_eps(probe, bg, 1 / 255.0));
    psy_color_free(probe);

    // clang-format off
    g_object_set(bg,
            "red", random_double_range(0, 1),
            "blue", random_double_range(0,1),
            "green", random_double_range(0, 1),
            "alpha", random_double_range(0, 1),
            NULL);
    // clang-format on
    g_object_set(img, "format", PSY_IMAGE_FORMAT_RGBA, NULL);
    psy_image_clear(img, bg);

    probe = psy_image_get_pixel(
        img, random_int_range(0, WIDTH), random_int_range(0, HEIGHT));
    CU_ASSERT_TRUE(psy_color_equal_eps(probe, bg, 1 / 255.0));

    psy_color_free(bg);
    psy_color_free(probe);
    psy_image_free(img);
}

static void
test_image_set_pixel(void)
{
    const gint WIDTH = 100, HEIGHT = 100;

    PsyImage *img   = psy_image_new(WIDTH, HEIGHT, PSY_IMAGE_FORMAT_RGB);
    PsyColor *color = psy_color_new_rgb((float) random_double_range(0, 1),
                                        (float) random_double_range(0, 1),
                                        (float) random_double_range(0, 1));
    gint      row   = random_int_range(0, HEIGHT);
    gint      col   = random_int_range(0, WIDTH);
    psy_image_set_pixel(img, row, col, color);

    PsyColor *probe = psy_image_get_pixel(img, row, col);

    CU_ASSERT_TRUE(psy_color_equal_eps(color, probe, 1.0 / 255));

    psy_color_free(probe);
    psy_color_free(color);
    psy_image_free(img);
}

static void
test_image_get_bytes(void)
{
    const gint WIDTH = 100, HEIGHT = 100;

    PsyImage *img   = psy_image_new(WIDTH, HEIGHT, PSY_IMAGE_FORMAT_RGB);
    PsyColor *color = psy_color_new_rgb((float) random_double_range(0, 1),
                                        (float) random_double_range(0, 1),
                                        (float) random_double_range(0, 1));
    psy_image_clear(img, color);

    gsize bytes_size;
    gsize img_size = psy_image_get_num_bytes(img);

    GBytes       *bytes     = psy_image_get_bytes(img);
    const guint8 *bytes_ptr = g_bytes_get_data(bytes, &bytes_size);
    const guint8 *img_ptr   = psy_image_get_ptr(img);

    CU_ASSERT_EQUAL(img_size, bytes_size);
    // memcmp should return 0 when memory is equal... like strcmp.
    CU_ASSERT_EQUAL(memcmp(img_ptr, bytes_ptr, bytes_size), 0);
    // A deep copy should be made
    CU_ASSERT_PTR_NOT_EQUAL(img_ptr, bytes_ptr);

    psy_color_free(color);
    psy_image_free(img);
    g_bytes_unref(bytes);
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

    test = CU_ADD_TEST(suite, test_image_clear);
    if (!test)
        return 1;

    test = CU_ADD_TEST(suite, test_image_set_pixel);
    if (!test)
        return 1;

    test = CU_ADD_TEST(suite, test_image_get_bytes);
    if (!test)
        return 1;

    return 0;
}
