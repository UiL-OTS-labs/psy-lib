
#include <epoxy/egl.h>

#include "psy-gl-program.h"
#include "psy-gl-utilities.h"

/**
 * psy_gl_canvas_init_default_shaders:
 *
 * Takes care that the default shaders are uploaded.
 *
 * Stability: private
 */
void
psy_gl_canvas_init_default_shaders(PsyCanvas *self, GError **error)
{
    // Uniform color program
    PsyDrawingContext *context = psy_canvas_get_context(PSY_CANVAS(self));

    PsyShaderProgram *program = psy_drawing_context_create_program(context);

    psy_shader_program_set_vertex_shader_from_path(
        program, "./psy/uniform-color.vert", error);
    if (*error)
        goto fail;

    psy_shader_program_set_fragment_shader_from_path(
        program, "./psy/uniform-color.frag", error);
    if (*error)
        goto fail;

    psy_shader_program_link(program, error);
    if (*error)
        goto fail;

    psy_drawing_context_register_program(
        context, PSY_UNIFORM_COLOR_PROGRAM_NAME, program, error);
    if (*error)
        return;

    g_clear_object(&program);

    // Picture program
    program = psy_drawing_context_create_program(context);

    psy_shader_program_set_vertex_shader_from_path(
        program, "./psy/picture.vert", error);
    if (*error)
        goto fail;

    psy_shader_program_set_fragment_shader_from_path(
        program, "./psy/picture.frag", error);
    if (*error)
        goto fail;

    psy_shader_program_link(program, error);
    if (*error)
        goto fail;

    psy_drawing_context_register_program(
        context, PSY_PICTURE_PROGRAM_NAME, program, error);

    g_clear_object(&program);
    return;

fail:
    g_object_unref(program);
}

/**
 * psy_gl_canvas_upload_projection_matrices:
 *
 *
 * Stability: private
 */
void
psy_gl_canvas_upload_projection_matrices(PsyCanvas *self)
{
    PsyMatrix4 *projection = psy_canvas_get_projection(self);
    GError     *error      = NULL;

    PsyDrawingContext *context = psy_canvas_get_context(self);
    PsyShaderProgram  *program = psy_drawing_context_get_program(
        context, PSY_UNIFORM_COLOR_PROGRAM_NAME);

    if (program) {
        psy_shader_program_use(program, &error);
        if (error) {
            g_critical("Unable to set picture projection matrix: %s",
                       error->message);
            g_error_free(error);
            error = NULL;
        }
        psy_shader_program_set_uniform_matrix4(
            program, "projection", projection, &error);
        if (error) {
            g_critical("Unable to set picture projection matrix: %s",
                       error->message);
            g_error_free(error);
            error = NULL;
        }
    }
    program
        = psy_drawing_context_get_program(context, PSY_PICTURE_PROGRAM_NAME);
    if (program) {
        psy_shader_program_use(program, &error);
        if (error) {
            g_critical("Unable to set picture projection matrix: %s",
                       error->message);
            g_error_free(error);
            error = NULL;
        }
        psy_shader_program_set_uniform_matrix4(
            program, "projection", projection, &error);
        if (error) {
            g_critical("Unable to set picture projection matrix: %s",
                       error->message);
            g_error_free(error);
            error = NULL;
        }
    }
}

/**
 * psy_egl_strerr:
 * @error: an error code returned by `eglGetError()`
 *
 * Turns an error code of Egl into a string
 *
 * stability: private
 * Returns: a string describing the error occured.
 */
const gchar *
psy_egl_strerr(gint error)
{
    switch (error) {
    case EGL_SUCCESS:
        return "The last function succeeded without error";
    case EGL_NOT_INITIALIZED:
        return "EGL is not initialized, or could not be initialized, for the "
               "specified EGL display connection";
    case EGL_BAD_ACCESS:
        return "EGL cannot access a requested resource (for example a context "
               "is bound in another thread)";
    case EGL_BAD_ALLOC:
        return "EGL failed to allocate resources for the requested "
               "operation";
    case EGL_BAD_ATTRIBUTE:
        return "An unrecognized attribute or attribute value was passed in "
               "the attribute list";
    case EGL_BAD_CONTEXT:
        return "An EGLContext argument does not name a valid EGL rendering "
               "context";
    case EGL_BAD_CONFIG:
        return "An EGLConfig argument does not name a valid EGL frame "
               "buffer configuration";
    case EGL_BAD_CURRENT_SURFACE:
        return "The current surface of the calling thread is a window, "
               "pixel buffer or pixmap that is no longer "
               "valid";
    case EGL_BAD_DISPLAY:
        return "An EGLDisplay argument does not name a valid EGL display "
               "connection";
    case EGL_BAD_SURFACE:
        return "An EGLSurface argument does not name a valid surface "
               "(window, pixel buffer or pixmap) configured for GL "
               "rendering";
    case EGL_BAD_MATCH:
        return "Arguments are inconsistent (for example, a valid context "
               "requires buffers not supplied by a valid "
               "surface)";
    case EGL_BAD_PARAMETER:
        return "One or more argument values are "
               "invalid";
    case EGL_BAD_NATIVE_PIXMAP:
        return "A NativePixmapType argument does not refer to a valid "
               "native pixmap";
    case EGL_BAD_NATIVE_WINDOW:
        return "A NativeWindowType argument does not refer to a valid "
               "native window";
    case EGL_CONTEXT_LOST:
        return "A power management event has occurred. The application "
               "must destroy all contexts and reinitialise OpenGL ES state "
               "and objects to continue rendering";
    default:
        g_error("unexpected value");
    }
}
