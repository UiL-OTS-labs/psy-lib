
#include <math.h>
#include <psylib.h>

const int WIDTH  = 1920;
const int HEIGHT = 1080;

gfloat g_rotation   = 0.0f;
gint   g_num_frames = 10;

void
debug_message(PsyGlCanvas *self,
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
    (void) source;
    (void) type;
    (void) id;
    (void) severity;
    (void) self;
    (void) user_data;
    g_printerr("%s: message: %s, source: %s, type: %s, severity: %s\n",
               __func__,
               message,
               source_str,
               type_str,
               severity_str);
}

void
update_rectangle(PsyVisualStimulus *stim,
                 PsyTimePoint      *tp,
                 gint64             p0,
                 gpointer           data)
{
    (void) data;
    (void) tp;
    g_print("p0 = %ld\n", p0);
    psy_visual_stimulus_set_rotation(stim, g_rotation);
    g_rotation += 2 * M_PI / g_num_frames;
}

int
main(void)
{
    GError         *error   = NULL;
    PsyDuration    *stimdur = psy_duration_new_ms(1000);
    PsyImageCanvas *icanvas = PSY_IMAGE_CANVAS(
        psy_gl_canvas_new_full(WIDTH, HEIGHT, FALSE, TRUE, 4, 4));
    PsyCanvas *canvas     = PSY_CANVAS(icanvas); // alias
    PsyColor  *stim_color = psy_color_new_rgb(0.25, .5, .75);

    g_signal_connect(canvas, "debug-message", G_CALLBACK(debug_message), NULL);

    PsyTimePoint *tp_null = psy_time_point_new();

    PsyRectangle *rect = psy_rectangle_new_full(
        PSY_CANVAS(canvas), 0, 0, WIDTH / 2.0, HEIGHT / 2.0);

    g_signal_connect(rect, "update", G_CALLBACK(update_rectangle), NULL);

    psy_stimulus_play_for(PSY_STIMULUS(rect), tp_null, stimdur);
    psy_visual_stimulus_set_color(PSY_VISUAL_STIMULUS(rect), stim_color);

    for (int i = 0; i < g_num_frames; i++) {
        char path[128];
        g_snprintf(
            path, sizeof(path), "/%s/some-picture-%d.png", g_get_tmp_dir(), i);
        psy_image_canvas_iterate(icanvas);
        PsyImage *image = psy_canvas_get_image(canvas);
        psy_image_save_path(image, path, "png", &error);
        g_object_unref(image);
    }

    g_object_unref(icanvas);
    psy_time_point_free(tp_null);
    psy_duration_free(stimdur);
#pragma message "Create psy_color_free to free stim color"
    g_object_unref(stim_color);
}
