
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
 * @PSY_GL_ERROR_INVALID_ENUM: glError returned GL_INVALID_ENUM
 * @PSY_GL_ERROR_INVALID_VALUE: glError returned GL_INVALID_VALUE
 * @PSY_GL_ERROR_INVALID_OPERATION: glError returned GL_INVALID_OPERATION
 * @PSY_GL_ERROR_INVALID_FRAMEBUFFER_OPERATION: glError returned GL_INVALID_FRAMEBUFFER_OPERATION
 * @PSY_GL_ERROR_OUT_OF_MEMORY: glError returned GL_OUT_OF_MEMORY
 * @PSY_GL_ERROR_STACK_UNDERFLOW: glError returned GL_STACK_UNDERFLOW
 * @PSY_GL_ERROR_STACK_OVERFLOW: glError returned GL_STACK_OVERFLOW
 *
 * An operation related to OpenGL failed. 
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
psy_gl_error_quark(void);

G_MODULE_EXPORT gboolean
psy_gl_check_error(GError** error);

G_END_DECLS

#endif 
