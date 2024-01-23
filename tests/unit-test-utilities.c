

#include <psylib.h>
#include <stdarg.h>
#include <string.h>

#include "unit-test-utilities.h"

// globals

static GRand  *g_random_dev;
static guint32 g_seed;

static GMutex log_mutex;

static struct log_data {
    GFileOutputStream *stream; // for specific test
    GFileOutputStream *main;   // for all data
    GLogLevelFlags     level;
    gchar             *domain; // domains starting with domain are logged.
} g_log_data;

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

static void
write_to_output_file(GOutputStream *stream, const char *line)
{
    GError *error       = NULL;
    gchar  *line_ending = "\n"; // Perhaps add line endings for other platforms
    gsize   num_bytes_tot = 0;
    gsize   num_bytes_written;
    gsize   len = strlen(line);

    while (num_bytes_tot < len && error == NULL) {
        g_output_stream_write_all(G_OUTPUT_STREAM(stream),
                                  &line[num_bytes_tot],
                                  len,
                                  &num_bytes_written,
                                  NULL,
                                  &error);
        num_bytes_tot += num_bytes_written;
    }

    if (error) {
        g_error("Unable to write to log file: %s",
                error->message); // terminates
    }

    len               = strlen(line_ending);
    num_bytes_written = 0;
    num_bytes_tot     = 0;

    while (num_bytes_tot < len && error == NULL) {
        g_output_stream_write_all(G_OUTPUT_STREAM(stream),
                                  &line_ending[num_bytes_written],
                                  strlen(line_ending),
                                  &num_bytes_written,
                                  NULL,
                                  &error);
        num_bytes_tot += num_bytes_written;
    }

    if (error) {
        g_error("Unable to write to log file: %s",
                error->message); // terminates
    }
}

static GLogWriterOutput
psy_unit_test_logger(GLogLevelFlags   log_level,
                     const GLogField *fields,
                     gsize            num_fields,
                     gpointer         data)
{
    (void) data;
    const GLogField *field = NULL;
    g_mutex_lock(&log_mutex);

    if (g_log_data.level < log_level)
        goto exit;

    for (guint i = 0; i < num_fields; i++) { // extract log_domain
        field = &fields[i];
        if (g_strcmp0(field->key, "GLIB_DOMAIN") == 0)
            break;
    }
    g_assert(field);

    if (g_log_data.domain) {
        size_t len = strlen(g_log_data.domain);

        if (strncmp(g_log_data.domain, field->value, len) != 0) {
            goto exit; // drop it
        }
    }

    gchar *line // line for output
        = g_log_writer_format_fields(log_level, fields, num_fields, FALSE);

    if (g_log_data.stream) {
        write_to_output_file(G_OUTPUT_STREAM(g_log_data.stream), line);
    }
    if (g_log_data.main) {
        write_to_output_file(G_OUTPUT_STREAM(g_log_data.main), line);
    }

exit:

    g_mutex_unlock(&log_mutex);

    return G_LOG_WRITER_HANDLED;
}

void
install_log_handler(void)
{
    g_clear_object(&g_log_data.stream);
    g_clear_pointer(&g_log_data.domain, g_free);

    g_log_data.level = G_LOG_LEVEL_INFO;

    if (!g_log_data.main) {
        g_log_data.main = open_log_file("main.txt");
    }

    g_log_set_writer_func(psy_unit_test_logger, NULL, NULL);
}

void
remove_log_handler(void)
{
    g_clear_object(&g_log_data.stream);
    g_clear_object(&g_log_data.main);
    g_clear_pointer(&g_log_data.domain, g_free);

    g_log_set_writer_func(g_log_writer_default, NULL, NULL);
}

void
set_log_handler_level(GLogLevelFlags flags)
{
    g_log_data.level = flags;
}

void
set_log_handler_file(const gchar *filename)
{
    g_clear_object(&g_log_data.stream);
    if (filename) {
        g_log_data.stream = open_log_file(filename);
    }
}

void
set_log_handler_domain(const gchar *domain)
{
    g_clear_pointer(&g_log_data.domain, g_free);
    if (domain) {
        g_log_data.domain = g_strdup(domain);
    }
}

/**
 * Returns an g_file_output_stream, that should be closed by the client.
 */
GFileOutputStream *
open_log_file(const gchar *filename)
{
    GError *error    = NULL;
    GFile  *tmp_file = g_file_new_build_filename(
        g_get_tmp_dir(), g_unit_test_tmp_dir, "log", filename, NULL);
    GFile *dir = g_file_get_parent(tmp_file);

    if (g_file_query_exists(dir, NULL)) {
        GFileType type
            = g_file_query_file_type(dir, G_FILE_QUERY_INFO_NONE, NULL);
        if (type != G_FILE_TYPE_DIRECTORY) {
            g_error("Oops \"%s\" exits and it ain't no directory",
                    g_file_get_path(dir));
        }
    }
    else {
        g_file_make_directory_with_parents(dir, NULL, &error);
        if (error) {
            char *path = g_file_get_path(dir);
            g_error("Unable to create dir %s", error->message);
            g_free(path);
            return NULL;
        }
    }
    g_object_unref(dir);

    if (!tmp_file) {
        char *path = g_file_get_path(tmp_file);
        g_error("Unable to create logfile %s", path);
        g_free(path);
        return NULL;
    }

    GFileOutputStream *ret = g_file_replace(
        tmp_file, NULL, FALSE, G_FILE_CREATE_REPLACE_DESTINATION, NULL, &error);

    if (error) {
        g_error("Unable to open/create file: %s", error->message);
        g_clear_error(&error);
    }

    g_object_unref(tmp_file);

    return ret;
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
