
#include <math.h>
#include <string.h>

#include "unit-test-utilities.h"
#include <criterion/criterion.h>
#include <criterion/new/assert.h>

#include <psylib.h>

static void
image_setup(void)
{
    set_log_handler_file("test-image.txt");
    init_random();
}

static void
image_teardown(void)
{
    deinitialize_random();
    set_log_handler_file(NULL);
}

TestSuite(image, .init = image_setup, .fini = image_teardown);

Test(image, create1)
{
    const guint          WIDTH = 1920, HEIGHT = 1080, NCHANNELS = 4;
    const PsyImageFormat format = PSY_IMAGE_FORMAT_RGBA;

    guint          width, height, nchannels;
    gsize          nbytes;
    guint          stride;
    PsyImageFormat format_out;

    PsyImage *img = psy_image_new(WIDTH, HEIGHT, format);

    cr_assert(ne(img, NULL));

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

    cr_expect(eq(width, WIDTH));
    cr_expect(eq(height, HEIGHT));
    cr_expect(eq(nchannels, NCHANNELS));
    cr_expect(eq(nbytes, WIDTH * HEIGHT * NCHANNELS));
    cr_expect(eq(stride, WIDTH * NCHANNELS));

    g_object_unref(img);
}

Test(image, create2)
{
    const guint          WIDTH = 1280, HEIGHT = 640, NCHANNELS = 3;
    const PsyImageFormat format = PSY_IMAGE_FORMAT_RGB;

    guint          width, height, nchannels;
    gsize          nbytes;
    guint          stride;
    PsyImageFormat format_out;

    PsyImage *img = psy_image_new(WIDTH, HEIGHT, format);

    cr_assert(ne(img, NULL));

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

    cr_expect(eq(width, WIDTH));
    cr_expect(eq(height, HEIGHT));
    cr_expect(eq(nchannels, NCHANNELS));
    cr_expect(eq(nbytes, WIDTH * HEIGHT * NCHANNELS));
    cr_expect(eq(stride, WIDTH * NCHANNELS));
    cr_expect(eq(format_out, format));

    g_object_unref(img);
}

Test(image, change_format)
{
    const guint WIDTH = 1280, HEIGHT = 640;

    PsyImage *img = psy_image_new(WIDTH, HEIGHT, PSY_IMAGE_FORMAT_RGB);

    cr_assert(ne(img, NULL));

    cr_expect(eq(psy_image_pixel_num_bytes(img), 3u));
    cr_expect(eq(psy_image_get_num_bytes(img),
                 WIDTH * HEIGHT * psy_image_pixel_num_bytes(img)));

    psy_image_set_format(img, PSY_IMAGE_FORMAT_RGBA);

    cr_expect(eq(psy_image_pixel_num_bytes(img), 4u));
    cr_expect(eq(psy_image_get_num_bytes(img),
                 WIDTH * HEIGHT * psy_image_pixel_num_bytes(img)));

    g_object_unref(img);
}

Test(image, clear)
{
    const guint WIDTH = 100, HEIGHT = 100;

    PsyImage *img = psy_image_new(WIDTH, HEIGHT, PSY_IMAGE_FORMAT_RGB);
    cr_assert(ne(img, NULL));

    PsyColor *bg = psy_color_new_rgb(random_double_range(0, 1),
                                     random_double_range(0, 1),
                                     random_double_range(0, 1));
    cr_assert(ne(bg, NULL));

    psy_image_clear(img, bg);

    PsyColor *probe = psy_image_get_pixel(
        img, random_int_range(0, WIDTH), random_int_range(0, HEIGHT));

    cr_assert(psy_color_equal_eps(probe, bg, 1 / 255.0));
    psy_color_free(probe);

    // clang-format off
    g_object_set(bg,
            "r", random_double_range(0, 1),
            "b", random_double_range(0, 1),
            "g", random_double_range(0, 1),
            "a", random_double_range(0, 1),
            NULL);
    // clang-format on
    g_object_set(img, "format", PSY_IMAGE_FORMAT_RGBA, NULL);
    psy_image_clear(img, bg);

    probe = psy_image_get_pixel(
        img, random_int_range(0, WIDTH), random_int_range(0, HEIGHT));
    cr_assert(psy_color_equal_eps(probe, bg, 1 / 255.0));

    psy_color_free(bg);
    psy_color_free(probe);
    psy_image_free(img);
}

Test(image, set_pixel)
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

    cr_assert(psy_color_equal_eps(color, probe, 1.0 / 255));

    psy_color_free(probe);
    psy_color_free(color);
    psy_image_free(img);
}

Test(image, get_bytes)
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

    cr_assert(eq(img_size, bytes_size));
    // memcmp should return 0 when memory is equal... like strcmp.
    cr_assert(eq(memcmp(img_ptr, bytes_ptr, bytes_size), 0));
    // A deep copy should be made
    cr_assert(ne(img_ptr, bytes_ptr));

    psy_color_free(color);
    psy_image_free(img);
    g_bytes_unref(bytes);
}
