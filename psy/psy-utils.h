
#pragma once

#include <gmodule.h>

G_BEGIN_DECLS

G_MODULE_EXPORT void
psy_coordinate_c_to_center(gdouble  width,
                           gdouble  height,
                           gdouble  x_in,
                           gdouble  y_in,
                           gdouble *x_out,
                           gdouble *y_out);

G_MODULE_EXPORT void
psy_coordinate_center_to_c(gdouble  width,
                           gdouble  height,
                           gdouble  x_in,
                           gdouble  y_in,
                           gdouble *x_out,
                           gdouble *y_out);

G_MODULE_EXPORT void
psy_coordinate_c_to_center_i(
    gint width, gint height, gint x_in, gint y_in, gint *x_out, gint *y_out);

G_MODULE_EXPORT void
psy_coordinate_center_to_c_i(
    gint width, gint height, gint x_in, gint y_in, gint *x_out, gint *y_out);

G_END_DECLS
