
#include <psy-config.h>
#include <psylib.h>

bool    g_windowed  = FALSE;
gchar  *g_save_file = NULL;
gdouble g_x         = 100.0;
gdouble g_y         = 100.0;
gdouble g_width     = -1;
gdouble g_height    = -1;

gchar *g_filename = PSY_SOURCE_ROOT "/share/Itálica_Owl.jpg";

// clang-format off
GOptionEntry options[] = {
    {"windowed", 'w',    G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE,   &g_windowed,
        "Run test in a window",NULL},
    {"save", 's', G_OPTION_FLAG_NONE, G_OPTION_ARG_STRING,  &g_save_file,
        "Save png images in to file", "/tmp/some-picture.png"},
    {"x", 'x', G_OPTION_FLAG_NONE, G_OPTION_ARG_DOUBLE, &g_x, "The desired x coordinate", "0"},
    {"y", 'y', G_OPTION_FLAG_NONE, G_OPTION_ARG_DOUBLE, &g_y, "The desired y coordinate", "0"},
    {"width", 'W', G_OPTION_FLAG_NONE, G_OPTION_ARG_DOUBLE, &g_width, "The desired width of the picture", "400"},
    {"height", 'H', G_OPTION_FLAG_NONE, G_OPTION_ARG_DOUBLE, &g_height, "The desired height of the picture", "400"},
    {0,},
};

// clang-format on

static void
run_canvas_test(void)
{

    PsyTimePoint *tnull = psy_time_point_new();
    GError       *error = NULL;

    PsyCanvas         *canvas  = PSY_CANVAS(psy_image_canvas_new(640, 480));
    PsyDrawingContext *context = psy_canvas_get_context(canvas);
    psy_drawing_context_load_files_as_texture(context, &g_filename, 1, &error);
    PsyPicture *pic = psy_picture_new_xy_filename(canvas, g_x, g_y, g_filename);
    PsyTimePoint *tstart
        = psy_time_point_add(tnull, psy_canvas_get_frame_dur(canvas));

    psy_stimulus_play(PSY_STIMULUS(pic), tstart);

    // clang-format off
    g_object_set(
            pic,
            "x", (float) g_x,
            "y", (float) g_y,
            "width", (float) g_width,
            "height", (float) g_height,
            NULL);
    // clang-format on

    psy_image_canvas_iterate(PSY_IMAGE_CANVAS(canvas));

    if (g_save_file) {
        GError   *error = NULL;
        PsyImage *img   = psy_canvas_get_image(canvas);
        psy_image_save_path(img, g_save_file, "png", &error);
        if (error) {
            g_printerr("Unable to save image: %s", error->message);
            g_clear_error(&error);
        }
    }
    psy_time_point_free(tnull);
    psy_time_point_free(tstart);
}

static void
run_gui_test(void)
{
}

int
main(int argc, char **argv)
{
    GError         *error   = NULL;
    GOptionContext *context = g_option_context_new("");
    g_option_context_add_main_entries(context, options, NULL);

    g_option_context_parse(context, &argc, &argv, &error);
    if (error) {
        g_printerr("Unable to parse cmd option %s\n", error->message);
        return EXIT_FAILURE;
    }

    if (!g_windowed) {
        run_canvas_test();
    }
    else {
        run_gui_test();
    }
}
