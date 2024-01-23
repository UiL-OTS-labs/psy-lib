
#include <math.h>
#include <psylib.h>
#include <stdlib.h>

// Global variable
PsyClock     *clk;
PsyTimePoint *g_tstart = NULL;
PsyTimePoint *g_tstop  = NULL;

const gchar *g_markup_text = "<span font_desc=\"Dejavu Sans Mono 25\">"
                             "Hello, "
                             "</span>"
                             "<span foreground=\"red\">"
                             "World<span font_desc=\"Dejavu Sans 32\" "
                             "background=\"yellow\">!<i>!</i>!</span>"
                             "</span> "
                             "And this <b>text</b> continues for a bit";

const char *g_no_markup_text = "Hello, World! We are checking whether "
                               "this line wraps.";

// related to option parsing
gint    n_monitor;
gdouble g_duration   = 4.0f;
int     g_nvertices  = 10;
gdouble g_radius     = 50;
gdouble g_amplitude  = 25;
gdouble g_frequency  = 0.5;
gdouble g_x          = 0.0;
gdouble g_y          = 0.0;
gdouble g_z          = 0.0;
gchar  *g_texture_fn = "./share/Itálica_Owl.jpg";

char    *g_origin       = "center";
char    *g_units        = "pixels";
gboolean g_circle_first = FALSE;
gboolean g_opengl_debug = FALSE;
gboolean g_use_markup   = FALSE;

// clang-format off
static GOptionEntry entries[] = {
    {"monitor-number", 'n', G_OPTION_FLAG_NONE, G_OPTION_ARG_INT, &n_monitor, "The number of the desired monitor", "N"},
    {"duration", 'd', G_OPTION_FLAG_NONE, G_OPTION_ARG_DOUBLE, &g_duration, "The duration of the stimulus in seconds", "seconds"},
    {"radius", 'r', G_OPTION_FLAG_NONE, G_OPTION_ARG_DOUBLE, &g_radius, "The base radius of the circle", "units"},
    {"amplitude", 'a', G_OPTION_FLAG_NONE, G_OPTION_ARG_DOUBLE, &g_amplitude, "The base amplitude of the pulsing of the circle", "units"},
    {"frequency", 'f', G_OPTION_FLAG_NONE, G_OPTION_ARG_DOUBLE, &g_frequency, "The base frequency of the pulsing of the circle", "seconds"},
    {"vertices", 'v', G_OPTION_FLAG_NONE, G_OPTION_ARG_INT, &g_nvertices, "The number of vertices of the circle", "number"},
    {"origin", 'o', G_OPTION_FLAG_NONE, G_OPTION_ARG_STRING, &g_origin, "The center C style or center", "c|center"},
    {"units", 'u', G_OPTION_FLAG_NONE, G_OPTION_ARG_STRING, &g_units, "The units of the coordinate system", "pix|m|mm|visdeg"},
    {"x", 'x', G_OPTION_FLAG_NONE, G_OPTION_ARG_DOUBLE, &g_x, "The x-coordinate of the circle", "units depends on projection"},
    {"y", 'y', G_OPTION_FLAG_NONE, G_OPTION_ARG_DOUBLE, &g_y, "The y-coordinate of the circle", "units depends on projection"},
    {"z", 'z', G_OPTION_FLAG_NONE, G_OPTION_ARG_DOUBLE, &g_z, "The z-coordinate of the circle", "units depends on projection"},
    {"circle-first", 'c', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &g_circle_first, "Whether or not to present the circle first", NULL},
    {"texture-fn", 't', G_OPTION_FLAG_NONE, G_OPTION_ARG_STRING, &g_texture_fn, "The filename of the texture", "utf8"},
    {"debug", 'D', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &g_opengl_debug, "Use a add extra OpenGL debugging calls", NULL},
    {"use-markup", 'M', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &g_use_markup, "Use markup for the text stimulus.", NULL},
    {0}
};

// clang-format on

void
update_circle(PsyVisualStimulus *stim,
              PsyTimePoint      *tp,
              gint64             nth_frame,
              gpointer           data)
{
    (void) data;
    (void) tp;
    (void) nth_frame;
    PsyCircle   *circle = PSY_CIRCLE(stim);
    gfloat       radius;
    PsyDuration *dur = NULL;
    if (g_tstart)
        dur = psy_time_point_subtract(tp, g_tstart);
    else
        dur = psy_time_point_subtract(tp, tp);

    radius = g_radius
             + sin(psy_duration_get_seconds(dur) * g_frequency * 2 * M_PI)
                   * g_amplitude;

    psy_circle_set_radius(circle, radius);
    psy_duration_free(dur);
}

void
update_rect(PsyVisualStimulus *stim,
            PsyTimePoint      *tp,
            gint64             nth_frame,
            gpointer           data)
{
    (void) nth_frame;
    PsyTimePoint *start = data;
    PsyDuration  *dur   = psy_time_point_subtract(tp, start);

    gdouble seconds = psy_duration_get_seconds(dur);
    psy_visual_stimulus_set_rotation(stim, seconds);
    psy_visual_stimulus_set_scale_y(stim, 4 + sin(seconds * 2) * 2);
    psy_duration_free(dur);
}

void
circle_started(PsyCircle *circle, PsyTimePoint *tstart, gpointer data)
{
    (void) circle;
    PsyTimePoint *tzero = data;
    PsyDuration  *dur   = psy_time_point_subtract(tstart, tzero);
    g_print("Circle started after %lf seconds\n",
            psy_duration_get_seconds(dur));
    g_tstart = psy_time_point_dup(tstart);
    psy_duration_free(dur);
}

void
circle_stopped(PsyCircle *circle, PsyTimePoint *tstop, gpointer data)
{
    (void) circle;
    PsyTimePoint *tzero = data;
    g_tstop             = psy_time_point_dup(tstop);
    PsyDuration *dur    = psy_time_point_subtract(tstop, tzero);
    g_print("Circle stopped after %lf seconds\n",
            psy_duration_get_seconds(dur));
    psy_duration_free(dur);
}

void
on_picture_auto_size(PsyPicture *picture,
                     gfloat      width,
                     gfloat      height,
                     gpointer    data)
{
    (void) data;
    gfloat new_width = width / 8.0, new_height = height / 8.0;
    psy_rectangle_set_size(PSY_RECTANGLE(picture), new_width, new_height);
    g_print("Initial picture size is %d*%d, ", (int) width, (int) height);
    g_print("after picture custom resize it is %f*%f\n", new_width, new_height);
}

void
stop_loop(PsyCircle *circle, PsyTimePoint *tp, gpointer data)
{
    (void) circle;
    (void) tp;
    GMainLoop *loop = data;
    g_main_loop_quit(loop);
}

gint
get_window_style(void)
{
    const char *center = "center";
    const char *c      = "c";

    const char *pixels = "pixels";
    const char *m      = "m";
    const char *mm     = "mm";
    const char *visdeg = "visual degrees";

    gint style = 0;
    if (g_strcmp0(g_origin, center) == 0)
        style |= PSY_CANVAS_PROJECTION_STYLE_CENTER;
    else if (g_strcmp0(g_origin, c) == 0)
        style |= PSY_CANVAS_PROJECTION_STYLE_C;
    else {
        g_warning(
            "The origin wasn't %s nor %s, defaulting to %s", center, c, center);
        style |= PSY_CANVAS_PROJECTION_STYLE_CENTER;
    }

    if (g_strcmp0(g_units, pixels) == 0)
        style |= PSY_CANVAS_PROJECTION_STYLE_PIXELS;
    else if (g_strcmp0(g_units, m) == 0)
        style |= PSY_CANVAS_PROJECTION_STYLE_METER;
    else if (g_strcmp0(g_units, mm) == 0)
        style |= PSY_CANVAS_PROJECTION_STYLE_MILLIMETER;
    else if (g_strcmp0(g_units, visdeg) == 0)
        style |= PSY_CANVAS_PROJECTION_STYLE_VISUAL_DEGREES;
    else {
        g_warning("The units wasn't one of: %s, %s or %s, %s, defaulting to %s",
                  pixels,
                  m,
                  mm,
                  visdeg,
                  pixels);
        style |= PSY_CANVAS_PROJECTION_STYLE_PIXELS;
    }

    return style;
}

static void
open_gl_error_cb(PsyGtkWindow *self,
                 guint         source,
                 guint         type,
                 guint         id,
                 guint         severity,
                 gchar        *message,
                 gchar        *source_str,
                 gchar        *type_str,
                 gchar        *severity_str,
                 gpointer      user_data)
{
    (void) self;
    (void) source;
    (void) severity;
    (void) type;
    GMainLoop *loop = user_data;

    g_printerr("-----------------------------\n");
    g_printerr("OpenGL error id = %u\n", id);
    g_printerr("OpenGL error source = %s\n", source_str);
    g_printerr("OpenGL error type = %s\n", type_str);
    g_printerr("OpenGL error severity = %s\n", severity_str);
    g_printerr("OpenGL error message = \"%s\"\n", message);
    g_printerr("-----------------------------\n\n");

    (void) loop;
    // g_printerr("Exiting");
    // g_main_loop_quit(loop);
}

int
main(int argc, char **argv)
{

    int           ret = EXIT_SUCCESS;
    PsyTimePoint *tp, *start;
    GError       *error = NULL;
    gint          window_style;
    const gchar  *text_content  = NULL;
    PsyColor     *circle_color  = psy_color_new_rgb(1.0, 0, 0);
    PsyColor     *cross_color   = psy_color_new_rgb(1.0, 1.0, 0);
    PsyColor     *rect_color    = psy_color_new_rgb(0.0, 1.0, 0.5);
    PsyColor     *font_color    = psy_color_new_rgb(1.0, 1.0, 1.0);
    PsyColor     *text_bg_color = psy_color_new_rgb(0.2, 0.2, 0.2);

    GOptionContext *context = g_option_context_new("");
    g_option_context_add_main_entries(context, entries, NULL);

    if (!g_option_context_parse(context, &argc, &argv, &error)) {
        g_printerr("Unable to parse options: %s\n", error->message);
        g_option_context_free(context);
        return EXIT_FAILURE;
    }
    g_option_context_free(context);

    text_content = g_use_markup ? g_markup_text : g_no_markup_text;

    clk = psy_clock_new();
    tp  = psy_clock_now(clk);

    PsyDuration *start_dur = psy_duration_new_ms(500);
    PsyDuration *dur       = psy_duration_new(g_duration);

    GMainLoop *loop = g_main_loop_new(NULL, FALSE);

    // clang-format off
    PsyGtkWindow *window = g_object_new(
            PSY_TYPE_GTK_WINDOW,
            "n-monitor", n_monitor,
            "enable-debug", g_opengl_debug,
            NULL);
    // clang-format on

    g_signal_connect(
        window, "debug-message", G_CALLBACK(open_gl_error_cb), loop);

    window_style = get_window_style();
    psy_canvas_set_projection_style(PSY_CANVAS(window), window_style);

    PsyCircle *circle = psy_circle_new_full(
        PSY_CANVAS(window), g_x, g_y, g_radius, g_nvertices);
    PsyCross     *cross = psy_cross_new_full(PSY_CANVAS(window), 0, 0, 200, 10);
    PsyRectangle *rect
        = psy_rectangle_new_full(PSY_CANVAS(window), 200, 200, 50, 50);
    PsyPicture *picture
        = psy_picture_new_xy_filename(PSY_CANVAS(window), 300, 0, g_texture_fn);
    PsyText *text_stim = psy_text_new_full(
        PSY_CANVAS(window), -300, 300, 200, 200, text_content, TRUE);

    // clang-format off
    g_object_set(text_stim,
                 "color", text_bg_color,
                 "font-color", font_color,
                 "use-markup", g_use_markup,
                 NULL);
    // clang-format on

    g_object_unref(text_bg_color);
    g_object_unref(font_color);

    PsyDrawingContext *drawing_context
        = psy_canvas_get_context(PSY_CANVAS(window));
    psy_drawing_context_load_files_as_texture(
        drawing_context, &g_texture_fn, 1, NULL);

    psy_visual_stimulus_set_color(PSY_VISUAL_STIMULUS(circle), circle_color);
    g_object_set(cross, "color", cross_color, NULL);
    g_object_set(rect, "color", rect_color, NULL);
    g_object_unref(rect_color);

    g_signal_connect(circle, "update", G_CALLBACK(update_circle), tp);
    g_signal_connect(circle, "started", G_CALLBACK(circle_started), tp);
    g_signal_connect(circle, "stopped", G_CALLBACK(circle_stopped), tp);
    g_signal_connect(
        circle, "stopped", G_CALLBACK(stop_loop), loop); // stop the loop
    g_signal_connect_after(
        picture, "auto-resize", G_CALLBACK(on_picture_auto_size), NULL);

    start = psy_time_point_add(tp, start_dur);

    g_signal_connect(rect, "update", G_CALLBACK(update_rect), start);

    if (g_circle_first) {
        psy_stimulus_play_for(PSY_STIMULUS(circle), start, dur);
        psy_stimulus_play_for(PSY_STIMULUS(cross), start, dur);
    }
    else {
        psy_stimulus_play_for(PSY_STIMULUS(cross), start, dur);
        psy_stimulus_play_for(PSY_STIMULUS(circle), start, dur);
    }
    psy_stimulus_play_for(PSY_STIMULUS(rect), start, dur);
    psy_stimulus_play_for(PSY_STIMULUS(picture), start, dur);
    psy_stimulus_play_for(PSY_STIMULUS(text_stim), start, dur);

    g_main_loop_run(loop);

    PsyDuration *diff = NULL;

    if (g_tstop && g_tstart)
        diff = psy_time_point_subtract(g_tstop, g_tstart);

    g_print("The width = %d mm and height = %d mm\n",
            psy_canvas_get_width_mm(PSY_CANVAS(window)),
            psy_canvas_get_height_mm(PSY_CANVAS(window)));

    g_print("circle->num_frames = %ld\n",
            psy_visual_stimulus_get_num_frames(PSY_VISUAL_STIMULUS(circle)));
    g_print("cavas->num_frames = %ld, num_frames_missed  = %ld, num_frames_tot "
            "=  %ld\n",
            psy_canvas_get_num_frames(PSY_CANVAS(window)),
            psy_canvas_get_num_frames_missed(PSY_CANVAS(window)),
            psy_canvas_get_num_frames_total(PSY_CANVAS(window)));

    if (diff)
        g_print("Difference between start and stop = %lf\n",
                psy_duration_get_seconds(diff));

    g_main_loop_unref(loop);
    g_object_unref(window);
    g_object_unref(clk);
    psy_duration_free(dur);
    psy_duration_free(start_dur);
    if (diff)
        psy_duration_free(diff);
    g_object_unref(start);

    return ret;
}
