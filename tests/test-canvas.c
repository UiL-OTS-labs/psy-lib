#include <criterion/criterion.h>
#include <criterion/new/assert.h>

#include "unit-test-utilities.h"
#include <gl/psy-gl-canvas.h>

const gint WIDTH  = 640;
const gint HEIGHT = 480;

static void
canvas_suite_init(void)
{
    init_random();
}

static void
canvas_suite_fini(void)
{
    deinitialize_random();
}

TestSuite(canvas, .init = canvas_suite_init, .fini = canvas_suite_fini);

Test(canvas, initialization)
{
    // Use PsyGlCanvas as PsyCanvas is abstract
    PsyGlCanvas *canvas = psy_gl_canvas_new(WIDTH, HEIGHT);
    gint         width, height;
    gint         gl_major, gl_minor;
    gboolean     debug, use_es;

    cr_assert(ne(canvas, NULL));

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
    cr_expect(eq(width, WIDTH));
    cr_expect(eq(gl_major, 3));
    cr_expect(eq(gl_minor, 3));
    cr_expect(eq(height, HEIGHT));
    cr_expect(none(debug));
    cr_expect(none(use_es));

    psy_gl_canvas_free(canvas);
}

Test(canvas, background_color)
{
    PsyGlCanvas *canvas     = psy_gl_canvas_new(WIDTH, HEIGHT);
    PsyColor    *default_bg = NULL;
    PsyColor    *new_color  = g_object_new(
        PSY_TYPE_COLOR, "r", 0.0f, "g", 0.0f, "b", 0.0f, "a", 0.0f, NULL);
    gfloat r, g, b;

    cr_assert(ne(canvas, NULL));

    g_object_get(canvas, "background-color", &default_bg, NULL);
    cr_assert(ne(default_bg, NULL));
    g_object_get(default_bg, "r", &r, "g", &g, "b", &b, NULL);

    cr_expect(eq(r, 0.5));
    cr_expect(eq(g, 0.5));
    cr_expect(eq(b, 0.5));

    // Draw to test whether the color is applied
    psy_image_canvas_iterate(PSY_IMAGE_CANVAS(canvas));

    PsyImage *image = psy_canvas_get_image(PSY_CANVAS(canvas));
    PsyColor *probe = psy_image_get_pixel(
        image,
        random_int_range(0, (gint) psy_image_get_height(image)) - 1,
        random_int_range(0, (gint) psy_image_get_width(image)) - 1);
    cr_assert(psy_color_equal_eps(default_bg, probe, 1.0 / 255));

    g_clear_object(&image);
    g_clear_object(&probe);

    g_object_set(canvas, "background-color", new_color, NULL);

    // Draw to test whether the color is applied
    psy_image_canvas_iterate(PSY_IMAGE_CANVAS(canvas));

    image = psy_canvas_get_image(PSY_CANVAS(canvas));
    probe = psy_image_get_pixel(
        image,
        random_int_range(0, (gint) psy_image_get_height(image)) - 1,
        random_int_range(0, (gint) psy_image_get_width(image)) - 1);
    cr_assert(psy_color_equal_eps(new_color, probe, 1.0 / 255));

    g_clear_object(&image);
    g_clear_object(&probe);

    g_object_unref(new_color);
    g_object_unref(default_bg);
    g_object_unref(canvas);
}

Test(canvas, size_vd)
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

    cr_assert(
        eq(width_vd,
           2 * psy_radians_to_degrees(atan(width_mm / 2.0 / distance_mm))));
    cr_assert(
        eq(height_vd,
           2 * psy_radians_to_degrees(atan(height_mm / 2.0 / distance_mm))));

    g_object_unref(canvas);
}
