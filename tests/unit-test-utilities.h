
#pragma once

#include <glib.h>

G_BEGIN_DECLS
gboolean
init_random(void);

gboolean
init_random_with_seed(guint32 seed);

gint
random_int(void);

gint
random_int_range(gint lower_inclusive, gint upper_inclusive);

gdouble
random_double(void);

gdouble
random_double_range(gdouble lower_inclusive, gdouble upper_exclusive);

G_END_DECLS
