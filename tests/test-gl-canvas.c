
#include <math.h>
#include <string.h>

#include <criterion/criterion.h>
#include <criterion/new/assert.h>

#include <psylib.h>

static PsyColor *g_background_color;
static PsyColor *g_stim_color;

static void
setup_gl_canvas_suite(void)
{
    g_background_color = psy_color_new_rgb(.25, .25, .25);
    g_stim_color       = psy_color_new_rgb(1.0, 0, 0);
}

static void
tear_down_gl_canvas_suite(void)
{
    g_object_unref(g_background_color);
    g_object_unref(g_stim_color);
}

TestSuite(gl_canvas,
          .init = setup_gl_canvas_suite,
          .fini = tear_down_gl_canvas_suite);

static void
gl_canvas_debug_message(PsyGlCanvas *self,
                        guint        source,
                        guint        type,
                        guint        id,
                        guint        severity,
                        gchar       *message,
                        gchar       *source_str,
                        gchar       *type_str,
                        gchar       *severity_str,
                        gpointer     user_data)
{
    (void) self;
    (void) source;
    (void) type;
    (void) id;
    (void) severity;

    g_critical(
        "%s: OpenGl debug message: %s, source='%s', type='%s', severity='%s'",
        (const char *) user_data,
        message,
        source_str,
        type_str,
        severity_str);
}

Test(gl_canvas, create)
{
    const gint WIDTH = 1920, HEIGHT = 1080;

    gint width, height;

    PsyGlCanvas *canvas = psy_gl_canvas_new(WIDTH, HEIGHT);

    cr_assert(ne(canvas, NULL));

    width  = psy_canvas_get_width(PSY_CANVAS(canvas));
    height = psy_canvas_get_height(PSY_CANVAS(canvas));

    cr_expect(eq(width, WIDTH));
    cr_expect(eq(height, HEIGHT));

    g_object_unref(canvas);
}

Test(gl_canvas, iterate)
{
    const gint WIDTH = 640, HEIGHT = 480;

    PsyGlCanvas *canvas
        = psy_gl_canvas_new_full(WIDTH, HEIGHT, FALSE, TRUE, 4, 3);
    g_signal_connect(
        canvas, "debug-message", G_CALLBACK(gl_canvas_debug_message), NULL);

    psy_canvas_set_background_color(PSY_CANVAS(canvas), g_background_color);

    PsyTimePoint *tp1 = NULL;
    PsyTimePoint *tp0 = psy_image_canvas_get_time(PSY_IMAGE_CANVAS(canvas));
    PsyDuration  *dur = NULL;

    cr_assert(ne(canvas, NULL));

    psy_image_canvas_iterate(PSY_IMAGE_CANVAS(canvas));
    tp1 = psy_image_canvas_get_time(PSY_IMAGE_CANVAS(canvas));
    dur = psy_time_point_subtract(tp1, tp0);

    cr_assert(
        psy_duration_equal(dur, psy_canvas_get_frame_dur(PSY_CANVAS(canvas))));

    PsyImage *image = psy_canvas_get_image(PSY_CANVAS(canvas));
    cr_assert(ne(image, NULL));

    PsyColor *pixel = psy_image_get_pixel(image, 0, 0);

    cr_assert(psy_color_equal_eps(pixel, g_background_color, 1.0 / 255));

    g_object_unref(pixel);
    g_object_unref(image);
    psy_time_point_free(tp0);
    psy_time_point_free(tp1);
    psy_duration_free(dur);
    g_object_unref(canvas);
}
