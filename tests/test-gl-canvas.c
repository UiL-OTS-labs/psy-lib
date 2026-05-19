
#include <math.h>
#include <string.h>

#include <psylib.h>

typedef struct GlCanvasFixture {
    PsyColor *background_color;
    PsyColor *stim_color;
} GlCanvasFixture;

static void
setup_gl_canvas_suite(GlCanvasFixture *fix, gconstpointer unused)
{
    (void) unused;
    fix->background_color = psy_color_new_rgb(.25f, .25f, .25f);
    fix->stim_color       = psy_color_new_rgb(1.0f, 0.0f, 0.0f);
}

static void
tear_down_gl_canvas_suite(GlCanvasFixture *fix, gconstpointer unused)
{
    (void) unused;
    g_clear_object(&fix->background_color);
    g_clear_object(&fix->stim_color);
}

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

static void
test_gl_canvas_create(void)
{
    const gint WIDTH = 1920, HEIGHT = 1080;

    gint width, height;

    PsyGlCanvas *canvas = psy_gl_canvas_new(WIDTH, HEIGHT);

    g_assert_nonnull(canvas);

    width  = psy_canvas_get_width(PSY_CANVAS(canvas));
    height = psy_canvas_get_height(PSY_CANVAS(canvas));

    g_assert_cmpint(width, ==, WIDTH);
    g_assert_cmpint(height, ==, HEIGHT);

    g_object_unref(canvas);
}

static void
test_gl_canvas_iterate(GlCanvasFixture *fix, gconstpointer unused)
{
    (void) unused;
    const gint WIDTH = 640, HEIGHT = 480;

    PsyGlCanvas *canvas
        = psy_gl_canvas_new_full(WIDTH, HEIGHT, FALSE, TRUE, 4, 3);

    g_signal_connect(
        canvas, "debug-message", G_CALLBACK(gl_canvas_debug_message), NULL);

    psy_canvas_set_background_color(PSY_CANVAS(canvas), fix->background_color);

    PsyTimePoint *tp1 = NULL;
    PsyTimePoint *tp0 = psy_image_canvas_get_time(PSY_IMAGE_CANVAS(canvas));
    PsyDuration  *dur = NULL;

    g_assert_nonnull(canvas);

    psy_image_canvas_iterate(PSY_IMAGE_CANVAS(canvas));
    tp1 = psy_image_canvas_get_time(PSY_IMAGE_CANVAS(canvas));
    dur = psy_time_point_subtract(tp1, tp0);

    g_assert_true(
        psy_duration_equal(dur, psy_canvas_get_frame_dur(PSY_CANVAS(canvas))));

    PsyImage *image = psy_canvas_get_image(PSY_CANVAS(canvas));
    g_assert_nonnull(image);

    PsyColor *pixel = psy_image_get_pixel(image, 0, 0);

    g_assert_true(psy_color_equal_eps(pixel, fix->background_color, 1.0 / 255));

    g_object_unref(pixel);
    g_object_unref(image);
    psy_time_point_free(tp0);
    psy_time_point_free(tp1);
    psy_duration_free(dur);
    g_object_unref(canvas);
}

int
main(int argc, char **argv)
{
    g_test_init(&argc, &argv, NULL);

    g_test_add_func("/gl_canvas/create", test_gl_canvas_create);
    g_test_add("/gl_canvas/iterate",
               GlCanvasFixture,
               NULL,
               setup_gl_canvas_suite,
               test_gl_canvas_iterate,
               tear_down_gl_canvas_suite);

    return g_test_run();
}
