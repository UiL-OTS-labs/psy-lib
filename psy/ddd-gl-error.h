
#ifndef DDD_GL_ERROR_H
#define DDD_GL_ERROR_H

#include <glib-object.h>
#include <gmodule.h>

G_BEGIN_DECLS

#define DDD_GL_ERROR ddd_gl_error_quark()

typedef enum ddd_gl_error {

    DDD_GL_ERROR_SHADER_COMPILE,
    DDD_GL_ERROR_PROGRAM_LINK,

    DDD_GL_ERROR_INVALID_ENUM,
    DDD_GL_ERROR_INVALID_VALUE,
    DDD_GL_ERROR_INVALID_OPERATION,
    DDD_GL_ERROR_INVALID_FRAMEBUFFER_OPERATION,
    DDD_GL_ERROR_OUT_OF_MEMORY,
    DDD_GL_ERROR_STACK_UNDERFLOW,
    DDD_GL_ERROR_STACK_OVERFLOW

} ddd_gl_error;

G_MODULE_EXPORT GQuark
ddd_gl_error_quark();

G_MODULE_EXPORT gboolean
ddd_gl_check_error(GError** error);

G_END_DECLS

#endif 
