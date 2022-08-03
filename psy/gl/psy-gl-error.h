
#ifndef PSY_GL_ERROR_H
#define PSY_GL_ERROR_H

#include <glib-object.h>
#include <gmodule.h>

G_BEGIN_DECLS

#define PSY_GL_ERROR psy_gl_error_quark()

/**
 * PsyGlError:
 * @PSY_GL_ERROR_SHADER_COMPILE: Unable to compile shader
 * @PSY_GL_ERROR_PROGRAM_LINK: Unable to link program,
 *
 */
typedef enum  {

    PSY_GL_ERROR_SHADER_COMPILE,
    PSY_GL_ERROR_PROGRAM_LINK,

    PSY_GL_ERROR_INVALID_ENUM,
    PSY_GL_ERROR_INVALID_VALUE,
    PSY_GL_ERROR_INVALID_OPERATION,
    PSY_GL_ERROR_INVALID_FRAMEBUFFER_OPERATION,
    PSY_GL_ERROR_OUT_OF_MEMORY,
    PSY_GL_ERROR_STACK_UNDERFLOW,
    PSY_GL_ERROR_STACK_OVERFLOW

} PsyGlError;

G_MODULE_EXPORT GQuark
psy_gl_error_quark();

G_MODULE_EXPORT gboolean
psy_gl_check_error(GError** error);

G_END_DECLS

#endif 
