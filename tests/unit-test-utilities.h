
#pragma once

#include <psylib.h>

G_BEGIN_DECLS
gboolean
init_random(void);

gboolean
init_random_with_seed(guint32 seed);

void
deinitialize_random(void);

// Override default log hander
void
install_log_handler(void);

// remove log hander and associated data
void
remove_log_handler(void);

// set the level threshold default is G_LOG_LEVEL_INFO
void
set_log_handler_level(GLogLevelFlags level);

// set the output file written to /tmp/psy-unit-tests/log/file
void
set_log_handler_file(const gchar *file);

// Capture only this domain, the rest is ignored. if null every domain is
// logged.
void
set_log_handler_domain(const gchar *domain);

GFileOutputStream *
open_log_file(const gchar *name);

guint32
random_seed(void);

gint
random_int(void);

gint
random_int_range(gint lower_inclusive, gint upper_inclusive);

gdouble
random_double(void);

gdouble
random_double_range(gdouble lower_inclusive, gdouble upper_exclusive);

void
set_save_images(gboolean save);

gboolean
save_images(void);

void
save_image_tmp_png(PsyImage *image, const char *name_fmt, ...);

G_END_DECLS
