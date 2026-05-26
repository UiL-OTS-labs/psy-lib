
#include <math.h>
#include <psylib.h>

const gint WIDTH  = 640;
const gint HEIGHT = 480;

static void
test_canvas_initialization(void)
{
    // Use PsyGlCanvas as PsyCanvas is abstract
    PsyGlCanvas *canvas = psy_gl_canvas_new(WIDTH, HEIGHT);
    gint         width, height;
    gint         gl_major, gl_minor;
    gboolean     debug, use_es;

    g_assert_nonnull(canvas);

    // clang-format off
    g_object_get(
        canvas,
        "height", &height,
        "width", &width,
        "enable-debug", &debug,
        "use-es", &use_es,
        "gl-major", &gl_major,
        "gl-minor", &gl_minor,
        NULL);
    // clang-format on
    g_assert_cmpint(width, ==, WIDTH);
    g_assert_cmpint(gl_major, ==, 3);
    g_assert_cmpint(gl_minor, ==, 3);
    g_assert_cmpint(height, ==, HEIGHT);
    g_assert_false(debug);
    g_assert_false(use_es);

    psy_gl_canvas_free(canvas);
}

static void
test_canvas_background_color(void)
{
    PsyGlCanvas *canvas     = psy_gl_canvas_new(WIDTH, HEIGHT);
    PsyColor    *default_bg = NULL;
    PsyColor    *new_color  = g_object_new(
        PSY_TYPE_COLOR, "r", 0.0f, "g", 0.0f, "b", 0.0f, "a", 0.0f, NULL);
    gfloat r, g, b;

    g_assert_nonnull(canvas);

    g_object_get(canvas, "background-color", &default_bg, NULL);
    g_assert_nonnull(default_bg);
    g_object_get(default_bg, "r", &r, "g", &g, "b", &b, NULL);

    g_assert_cmpfloat(r, ==, 0.5);
    g_assert_cmpfloat(g, ==, 0.5);
    g_assert_cmpfloat(b, ==, 0.5);

    // Draw to test whether the color is applied
    psy_image_canvas_iterate(PSY_IMAGE_CANVAS(canvas));

    PsyImage *image = psy_canvas_get_image(PSY_CANVAS(canvas));
    PsyColor *probe = psy_image_get_pixel(
        image,
        g_test_rand_int_range(0, (gint) psy_image_get_height(image)) - 1,
        g_test_rand_int_range(0, (gint) psy_image_get_width(image)) - 1);
    g_assert_true(psy_color_equal_eps(default_bg, probe, 1.0 / 255));

    g_clear_object(&image);
    g_clear_object(&probe);

    g_object_set(canvas, "background-color", new_color, NULL);

    // Draw to test whether the color is applied
    psy_image_canvas_iterate(PSY_IMAGE_CANVAS(canvas));

    image = psy_canvas_get_image(PSY_CANVAS(canvas));
    probe = psy_image_get_pixel(
        image,
        g_test_rand_int_range(0, (gint) psy_image_get_height(image)) - 1,
        g_test_rand_int_range(0, (gint) psy_image_get_width(image)) - 1);
    g_assert_true(psy_color_equal_eps(new_color, probe, 1.0 / 255));

    g_clear_object(&image);
    g_clear_object(&probe);

    g_object_unref(new_color);
    g_object_unref(default_bg);
    g_object_unref(canvas);
}

static void
test_canvas_size_vd(void)
{
    gfloat width_vd, height_vd;

    gint width_mm = 640, height_mm = 480, distance_mm = 1000;

    // clang-format off
    PsyCanvas* canvas = g_object_new(
            PSY_TYPE_GL_CANVAS,
            "width", WIDTH,
            "height", HEIGHT,
            "width-mm", width_mm,
            "height-mm", height_mm,
            "distance-mm", distance_mm,
            NULL
            );
    
    g_object_get(
            canvas,
            "width-vd", &width_vd,
            "height-vd", &height_vd,
            NULL);
    // clang-format on

    g_assert_cmpfloat(
        width_vd,
        ==,
        2 * psy_radians_to_degrees(atan(width_mm / 2.0 / distance_mm)));
    g_assert_cmpfloat(
        height_vd,
        ==,
        2 * psy_radians_to_degrees(atan(height_mm / 2.0 / distance_mm)));

    g_object_unref(canvas);
}

int
main(int argc, char **argv)
{
    g_test_init(&argc, &argv, NULL);

    g_test_add_func("/canvas/initialization", test_canvas_initialization);
    g_test_add_func("/canvas/background_color", test_canvas_background_color);
    g_test_add_func("/canvas/size_vd", test_canvas_size_vd);

    return g_test_run();
}
