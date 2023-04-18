

#include <psylib.h>
#include <stdarg.h>

#include "unit-test-utilities.h"

// globals

static GRand  *g_random_dev;
static guint32 g_seed;

// folder in platform specific temp dir
static const gchar *g_unit_test_tmp_dir = "psy-unit-tests/";

gboolean g_save_images = FALSE;

// Random functions

gboolean
init_random(void)
{
    if (g_random_dev) {
        g_warning("Random device already initialized");
        return TRUE;
    }

    g_seed = psy_random_uint32();

    g_random_dev = g_rand_new_with_seed(g_seed);
    if (G_LIKELY(g_random_dev))
        return TRUE;
    return FALSE;
}

gboolean
init_random_with_seed(guint32 seed)
{
    if (g_random_dev) {
        g_warning("Random device already initialized");
        return TRUE;
    }

    g_seed = seed;

    g_random_dev = g_rand_new_with_seed(seed);
    if (G_LIKELY(g_random_dev))
        return TRUE;
    return FALSE;
}

void
deinitialize_random(void)
{
    g_clear_pointer(&g_random_dev, g_rand_free);
}

guint
random_seed(void)
{
    return g_seed;
}

gint
random_int(void)
{
    if (G_UNLIKELY(!g_random_dev))
        g_critical("g_random = %p, have you initialized the random library?",
                   (gpointer) g_random_dev);

    return g_rand_int(g_random_dev);
}

gint
random_int_range(gint lower, gint upper)
{
    if (G_UNLIKELY(!g_random_dev))
        g_critical("g_random = %p, have you initialized the random library?",
                   (gpointer) g_random_dev);

    return g_rand_int_range(g_random_dev, lower, upper);
}

gdouble
random_double(void)
{
    if (G_UNLIKELY(!g_random_dev))
        g_critical("g_random = %p, have you initialized the random library?",
                   (gpointer) g_random_dev);

    return g_rand_double(g_random_dev);
}

gdouble
random_double_range(gdouble lower, gdouble upper)
{
    if (G_UNLIKELY(!g_random_dev))
        g_critical("g_random = %p, have you initialized the random library?",
                   (gpointer) g_random_dev);

    return g_rand_double_range(g_random_dev, lower, upper);
}

gboolean
random_boolean(void)
{
    if (G_UNLIKELY(!g_random_dev))
        g_critical("g_random = %p, have you initialized the random library?",
                   (gpointer) g_random_dev);

    return g_rand_boolean(g_random_dev);
}

// saving images

void
set_save_images(gboolean save)
{
    g_save_images = save;
}

gboolean
save_images(void)
{
    return g_save_images;
}

static void
save_image_tmp_png_priv(PsyImage *image, const char *name)
{
    GError *error = NULL;
    if (!psy_image_save_path(image, name, "png", &error)) {
        if (g_error_matches(error, G_FILE_ERROR, G_FILE_ERROR_NOENT)) {
            // create dir and retry
            GError *error2 = NULL;
            gchar   path[1024];
            g_snprintf(path,
                       sizeof(path),
                       "%s/%s",
                       g_get_tmp_dir(),
                       g_unit_test_tmp_dir);

            GFile *tmp_dir = g_file_new_for_path(path);
            g_file_make_directory(tmp_dir, NULL, &error2);
            if (error2) {
                g_critical("Unable to create tmp dir: %s", error->message);
                g_clear_error(&error2);
            }
            else {
                psy_image_save_path(image, name, "png", &error2);
                if (error2) {
                    g_critical("unable to save image: %s", error->message);
                    g_clear_error(&error2);
                }
            }
            g_object_unref(tmp_dir);
        }
        else {
            g_critical("Unable to save image: %s", error->message);
        }
        g_clear_error(&error);
    }
}

static void
save_image_tmp_png_v(PsyImage *image, const gchar *fmt, va_list args)
{
    GString *tmp_path = g_string_new("");

    // append our test dir in the os temp folder
    g_string_append_printf(
        tmp_path, "%s/%s", g_get_tmp_dir(), g_unit_test_tmp_dir);

    // append specific file name
    g_string_append_vprintf(tmp_path, fmt, args);

    save_image_tmp_png_priv(image, tmp_path->str);

    g_string_free(tmp_path, TRUE);
}

/**
 * save_image_tmp_png:
 * @image: an image to save in the temp dir
 * @name_fmt: the printf compatible format
 * @...: the parameter for @name_fmt
 *
 * This function saves an image in the temp directory for saving images.
 * These images may be inspected for post hoc analisis of the unit test.
 *
 * The final format of the filename in pseudocode is:
 * get_tmp_dir() + "psy-unit-tests/" + printf(name_fmt, @...)
 *
 * Stability: private
 */
void
save_image_tmp_png(PsyImage *image, const char *name_fmt, ...)
{
    va_list args;
    va_start(args, name_fmt);
    save_image_tmp_png_v(image, name_fmt, args);
    va_end(args);
}
