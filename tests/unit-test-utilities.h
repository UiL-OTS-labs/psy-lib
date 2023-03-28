
#pragma once

#include <psylib.h>

G_BEGIN_DECLS
gboolean
init_random(void);

gboolean
init_random_with_seed(guint32 seed);

void
deinitialize_random(void);

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
