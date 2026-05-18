
#include <math.h>
#include <psylib.h>

#include "unit-test-utilities.h"

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

    g_assert_nonnull(img);

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

    g_assert_cmpuint(width, ==, WIDTH);
    g_assert_cmpuint(height, ==, HEIGHT);
    g_assert_cmpuint(nchannels, ==, NCHANNELS);
    g_assert_cmpuint(nbytes, ==, WIDTH * HEIGHT * NCHANNELS);
    g_assert_cmpuint(stride, ==, WIDTH * NCHANNELS);

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

    g_assert_nonnull(img);

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

    g_assert_cmpuint(width, ==, WIDTH);
    g_assert_cmpuint(height, ==, HEIGHT);
    g_assert_cmpuint(nchannels, ==, NCHANNELS);
    g_assert_cmpuint(nbytes, ==, (guint64) WIDTH * HEIGHT * NCHANNELS);
    g_assert_cmpuint(stride, ==, WIDTH * NCHANNELS);
    g_assert_cmpuint(format_out, ==, format);

    g_object_unref(img);
}

static void
test_image_change_format(void)
{
    const guint WIDTH = 1280, HEIGHT = 640;

    PsyImage *img = psy_image_new(WIDTH, HEIGHT, PSY_IMAGE_FORMAT_RGB);

    g_assert_nonnull(img);

    g_assert_cmpuint(psy_image_pixel_num_bytes(img), ==, 3u);
    g_assert_cmpuint(psy_image_get_num_bytes(img),
                     ==,
                     WIDTH * HEIGHT * psy_image_pixel_num_bytes(img));

    psy_image_set_format(img, PSY_IMAGE_FORMAT_RGBA);

    g_assert_cmpuint(psy_image_pixel_num_bytes(img), ==, 4u);
    g_assert_cmpuint(psy_image_get_num_bytes(img),
                     ==,
                     WIDTH * HEIGHT * psy_image_pixel_num_bytes(img));

    g_object_unref(img);
}

static void
test_image_clear(void)
{
    const guint WIDTH = 100, HEIGHT = 100;

    PsyImage *img = psy_image_new(WIDTH, HEIGHT, PSY_IMAGE_FORMAT_RGB);
    g_assert_nonnull(img);

    PsyColor *bg = psy_color_new_rgb((float) g_test_rand_double_range(0, 1),
                                     (float) g_test_rand_double_range(0, 1),
                                     (float) g_test_rand_double_range(0, 1));
    g_assert_nonnull(bg);

    psy_image_clear(img, bg);

    PsyColor *probe
        = psy_image_get_pixel(img,
                              g_test_rand_int_range(0, (gint) WIDTH),
                              g_test_rand_int_range(0, (gint) HEIGHT));

    g_assert_true(psy_color_equal_eps(probe, bg, 1 / 255.0));
    psy_color_free(probe);

    // clang-format off
    g_object_set(bg,
            "r", (float)g_test_rand_double_range(0, 1),
            "b", (float)g_test_rand_double_range(0, 1),
            "g", (float)g_test_rand_double_range(0, 1),
            "a", (float)g_test_rand_double_range(0, 1),
            NULL);
    // clang-format on
    g_object_set(img, "format", PSY_IMAGE_FORMAT_RGBA, NULL);
    psy_image_clear(img, bg);

    probe
        = psy_image_get_pixel(img,
                              (guint) g_test_rand_int_range(0, (gint) WIDTH),
                              (guint) g_test_rand_int_range(0, (gint) HEIGHT));
    g_assert_true(psy_color_equal_eps(probe, bg, 1 / 255.0));

    psy_color_free(bg);
    psy_color_free(probe);
    psy_image_free(img);
}

static void
test_image_set_pixel(void)
{
    const gint WIDTH = 100, HEIGHT = 100;

    PsyImage *img   = psy_image_new(WIDTH, HEIGHT, PSY_IMAGE_FORMAT_RGB);
    PsyColor *color = psy_color_new_rgb((float) g_test_rand_double_range(0, 1),
                                        (float) g_test_rand_double_range(0, 1),
                                        (float) g_test_rand_double_range(0, 1));
    gint      row   = g_test_rand_int_range(0, (gint) HEIGHT);
    gint      col   = g_test_rand_int_range(0, (gint) WIDTH);
    psy_image_set_pixel(img, row, col, color);

    PsyColor *probe = psy_image_get_pixel(img, row, col);

    g_assert_true(psy_color_equal_eps(color, probe, 1.0 / 255));

    psy_color_free(probe);
    psy_color_free(color);
    psy_image_free(img);
}

static void
test_image_get_bytes(void)
{
    const gint WIDTH = 100, HEIGHT = 100;

    PsyImage *img   = psy_image_new(WIDTH, HEIGHT, PSY_IMAGE_FORMAT_RGB);
    PsyColor *color = psy_color_new_rgb((float) g_test_rand_double_range(0, 1),
                                        (float) g_test_rand_double_range(0, 1),
                                        (float) g_test_rand_double_range(0, 1));
    psy_image_clear(img, color);

    gsize bytes_size;
    gsize img_size = psy_image_get_num_bytes(img);

    GBytes       *bytes     = psy_image_get_bytes(img);
    const guint8 *bytes_ptr = g_bytes_get_data(bytes, &bytes_size);
    const guint8 *img_ptr   = psy_image_get_ptr(img);

    g_assert_cmpuint(img_size, ==, bytes_size);
    // memcmp should return 0 when memory is equal... like strcmp.
    g_assert_cmpmem(img_ptr, bytes_size, bytes_ptr, bytes_size);
    // A deep copy should be made
    g_assert_true(img_ptr != bytes_ptr);

    psy_color_free(color);
    psy_image_free(img);
    g_bytes_unref(bytes);
}

int
main(int argc, char **argv)
{
    set_log_handler_file("test-image.txt");

    g_test_init(&argc, &argv, NULL);

    g_test_add_func("/image/create1", test_image_create1);
    g_test_add_func("/image/create2", test_image_create2);
    g_test_add_func("/image/change_format", test_image_change_format);
    g_test_add_func("/image/clear", test_image_clear);
    g_test_add_func("/image/set_pixel", test_image_set_pixel);
    g_test_add_func("/image/get_bytes", test_image_get_bytes);

    int save = g_test_run();

    set_log_handler_file(NULL);

    return save;
}
