
#include <psylib.h>

bool    g_windowed    = FALSE;
bool    g_save_images = FALSE;
gdouble g_x           = 100.0;
gdouble g_y           = 100.0;
gdouble g_width       = -1;
gdouble g_height      = -1;

const gchar *g_filename = "../share/Itálica_Owl.jpg";

// clang-format off
GOptionEntry options[] = {
    {"windowed", 'w',    G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE,   &g_windowed,
        "Run test in a window",""},
    {"save", 's', G_OPTION_FLAG_NONE, G_OPTION_ARG_FILENAME,  &g_save_images,
        "Save images in /tmp_folder/psy-unit-tests/", NULL},
    {"x", 'x', G_OPTION_FLAG_NONE, G_OPTION_ARG_DOUBLE, &g_x, "The desired x coordinate", ""},
    {"y", 'y', G_OPTION_FLAG_NONE, G_OPTION_ARG_DOUBLE, &g_y, "The desired y coordinate", ""},
    {"width", 'W', G_OPTION_FLAG_NONE, G_OPTION_ARG_DOUBLE, &g_width, "The desired width of the picture", ""},
    {"height", 'H', G_OPTION_FLAG_NONE, G_OPTION_ARG_DOUBLE, &g_height, "The desired height of the picture", ""},
    {0,},
};

static void
run_canvas_test() {

    PsyTimePoint* tnull = psy_time_point_new();

    PsyCanvas *canvas = PSY_CANVAS(psy_image_canvas_new(640, 480));
    PsyPicture* pic = psy_picture_new_xy_filename(canvas, g_x, g_y, g_filename);
    PsyTimePoint* tstart = psy_time_point_add(tnull, psy_canvas_get_frame_dur(canvas));
    
    psy_stimulus_play(PSY_STIMULUS(pic), tstart);

    psy_image_canvas_iterate(PSY_IMAGE_CANVAS(canvas));

    if (g_save_images) {
        PsyImage* img = psy_canvas_get_image(canvas);
        //psy_image_save();
    }
}

static void
run_gui_test() {
}

// clang-format on
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

    if (argc < 2)
        g_print("No picture specified");

    if (!g_windowed) {
        run_canvas_test();
    }
    else {
        run_gui_test();
    }
}
