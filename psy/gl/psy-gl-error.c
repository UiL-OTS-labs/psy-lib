
#include "psy-gl-error.h"
#include <epoxy/gl.h>

G_DEFINE_QUARK(psy-gl-error-quark, psy_gl_error)

/**
 * psy_gl_check_error:
 * @param error : If an error occurred it will be returned here.
 *
 * You can use this function to see whether opengl encountered an
 * error previously. Of course you need to have a current opengl context.
 *
 * Returns: TRUE if an error occurred FALSE otherwise.
 */
gboolean
psy_gl_check_error(GError **error)
{
    GLenum erroreno = glGetError();
    switch (erroreno) {
        case GL_INVALID_ENUM:
            g_set_error(error,
                        PSY_GL_ERROR,
                        PSY_GL_ERROR_INVALID_ENUM,
                        "An invalid enum specified to an OpenGL operation"
                        );
            break;
        case GL_INVALID_VALUE:
            g_set_error(error,
                        PSY_GL_ERROR,
                        PSY_GL_ERROR_INVALID_VALUE,
                        "An invalid value specified to an OpenGL operation"
                        );
            break;
        case GL_INVALID_OPERATION:
            g_set_error(error,
                        PSY_GL_ERROR,
                        PSY_GL_ERROR_INVALID_OPERATION,
                        "The specified operation is not allowed in the current state. The offending command is ignored."
                        );
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            g_set_error(error,
                        PSY_GL_ERROR,
                        PSY_GL_ERROR_INVALID_FRAMEBUFFER_OPERATION,
                        "The framebuffer object is not complete"
                        );
            break;
        case GL_OUT_OF_MEMORY:
            g_set_error(error,
                        PSY_GL_ERROR,
                        PSY_GL_ERROR_OUT_OF_MEMORY,
                        "There is not enough memory left to execute the command"
                        );
            break;
        case GL_STACK_UNDERFLOW:
            g_set_error(error,
                        PSY_GL_ERROR,
                        PSY_GL_ERROR_STACK_UNDERFLOW,
                        "An attempt has been made to perform an operation that would cause an internal stack to underflow"
                        );
            break;
        case GL_STACK_OVERFLOW:
            g_set_error(error,
                        PSY_GL_ERROR,
                        PSY_GL_ERROR_STACK_OVERFLOW,
                        "An attempt has been made to perform an operation that would cause an internal stack to overflow"
            );
            break;
        default:

            break;
    }
    return erroreno != GL_NO_ERROR;
}
