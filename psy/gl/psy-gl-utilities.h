
#pragma once

#include "psy-gl-canvas.h"

G_BEGIN_DECLS

void
psy_gl_canvas_init_default_shaders(PsyCanvas *self, GError **error);

void
psy_gl_canvas_upload_projection_matrices(PsyCanvas *canvas);

const gchar *
psy_egl_strerr(gint error);

G_END_DECLS
