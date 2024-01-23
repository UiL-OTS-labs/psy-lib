
#include <math.h>
#include <string.h>

#include <CUnit/CUnit.h>

#include <psylib.h>

static PsyColor *g_background_color;
static PsyColor *g_stim_color;

static int
setup_gl_cavans_suite(void)
{
    g_background_color = psy_color_new_rgb(.25, .25, .25);
    g_stim_color       = psy_color_new_rgb(1.0, 0, 0);

    return 0;
}

static int
tear_down_gl_canvas_suite(void)
{
    g_object_unref(g_background_color);
    g_object_unref(g_stim_color);

    return 0;
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
    const guint WIDTH = 1920, HEIGHT = 1080;

    gint width, height;

    PsyGlCanvas *canvas = psy_gl_canvas_new(WIDTH, HEIGHT);

    CU_ASSERT_PTR_NOT_NULL_FATAL(canvas);

    width  = psy_canvas_get_width(PSY_CANVAS(canvas));
    height = psy_canvas_get_height(PSY_CANVAS(canvas));

    CU_ASSERT_EQUAL(width, WIDTH);
    CU_ASSERT_EQUAL(height, HEIGHT);

    g_object_unref(canvas);
}

static void
test_gl_canvas_iterate(void)
{
    const guint WIDTH = 640, HEIGHT = 480;

    PsyGlCanvas *canvas
        = psy_gl_canvas_new_full(WIDTH, HEIGHT, FALSE, TRUE, 4, 3);
    g_signal_connect(
        canvas, "debug-message", G_CALLBACK(gl_canvas_debug_message), NULL);

    psy_canvas_set_background_color(PSY_CANVAS(canvas), g_background_color);

    PsyTimePoint *tp1 = NULL;
    PsyTimePoint *tp0 = psy_image_canvas_get_time(PSY_IMAGE_CANVAS(canvas));
    PsyDuration  *dur = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL(canvas);

    psy_image_canvas_iterate(PSY_IMAGE_CANVAS(canvas));
    tp1 = psy_image_canvas_get_time(PSY_IMAGE_CANVAS(canvas));
    dur = psy_time_point_subtract(tp1, tp0);

    CU_ASSERT_TRUE(
        psy_duration_equal(dur, psy_canvas_get_frame_dur(PSY_CANVAS(canvas))));

    PsyImage *image = psy_canvas_get_image(PSY_CANVAS(canvas));
    CU_ASSERT_PTR_NOT_NULL_FATAL(image);

    PsyColor *pixel = psy_image_get_pixel(image, 0, 0);

    CU_ASSERT_TRUE(psy_color_equal_eps(pixel, g_background_color, 1.0 / 255));

    g_object_unref(pixel);
    g_object_unref(image);
    g_object_unref(tp0);
    g_object_unref(tp1);
    psy_duration_free(dur);
    g_object_unref(canvas);
}

int
add_gl_canvas_suite(void)
{
    CU_Suite *suite = CU_add_suite(
        "PsyGlCanvas suite", setup_gl_cavans_suite, tear_down_gl_canvas_suite);
    CU_Test *test;
    if (!suite)
        return 1;

    test = CU_ADD_TEST(suite, test_gl_canvas_create);
    if (!test)
        return 1;

    test = CU_ADD_TEST(suite, test_gl_canvas_iterate);
    if (!test)
        return 1;

    return 0;
}
