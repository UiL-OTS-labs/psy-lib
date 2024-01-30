
#ifndef PSY_GL_ERROR_H
#define PSY_GL_ERROR_H

#include <glib-object.h>
#include <gmodule.h>

#include "../psy-enums.h"

G_BEGIN_DECLS

#define PSY_GL_ERROR psy_gl_error_quark()

G_MODULE_EXPORT GQuark
psy_gl_error_quark(void);

G_MODULE_EXPORT gboolean
psy_gl_check_error(GError **error);

G_END_DECLS

#endif
