
#include <epoxy/egl.h>
#include <epoxy/gl.h>

#include "psy-gl-canvas.h"
#include "psy-gl-error.h"
#include "psy-gl-utilities.h"

static void GLAPIENTRY
on_gl_canvas_error(GLenum        source,
                   GLenum        type,
                   guint         id,
                   GLenum        severity,
                   GLsizei       length,
                   const GLchar *message,
                   const void   *object);

struct _PsyGlCanvas {
    PsyImageCanvas parent;

    /* stuff related to egl */
    EGLDisplay display;
    EGLConfig  config;
    EGLContext egl_context;
    EGLSurface surface;

    /* object properties */
    gint     gl_major, gl_minor; // OpenGL (ES) version.
    gboolean debug;              /* whether or not to create a debug context. */
    gboolean use_es; /* whether or not to create OpenGL es context. If false
                        regular opengl will be used */
};

G_DEFINE_TYPE(PsyGlCanvas, psy_gl_canvas, PSY_TYPE_IMAGE_CANVAS)

typedef enum GlCanvasProperty {
    PROP_0,
    PROP_ENABLE_DEBUG,
    PROP_USE_ES,
    PROP_MAJOR,
    PROP_MINOR,
    NUM_PROPS, // number of properties, keep this one last.
} GlCanvasProperty;

typedef enum GlCanvasSignals { SIG_DEBUG_MESSAGE, NUM_SIGNALS } GlCanvasSignals;

static GParamSpec *properties[NUM_PROPS] = {0};
static guint       signals[NUM_SIGNALS]  = {0};

static void
psy_gl_canvas_set_property(GObject      *object,
                           guint         property_id,
                           const GValue *value,
                           GParamSpec   *pspec)
{
    PsyGlCanvas *self = PSY_GL_CANVAS(object);

    switch ((GlCanvasProperty) property_id) {
    case PROP_ENABLE_DEBUG:
        self->debug = g_value_get_boolean(value);
        break;
    case PROP_USE_ES:
        self->use_es = g_value_get_boolean(value);
        break;
    case PROP_MAJOR:
        self->gl_major = g_value_get_int(value);
        break;
    case PROP_MINOR:
        self->gl_minor = g_value_get_int(value);
        break;
    default:
        /* We don't have any other property... */
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static void
psy_gl_canvas_get_property(GObject    *object,
                           guint       property_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
    PsyGlCanvas *self = PSY_GL_CANVAS(object);

    switch ((GlCanvasProperty) property_id) {
    case PROP_ENABLE_DEBUG:
        g_value_set_boolean(value, self->debug);
        break;
    case PROP_USE_ES:
        g_value_set_boolean(value, self->use_es);
        break;
    case PROP_MAJOR:
        g_value_set_int(value, self->gl_major);
        break;
    case PROP_MINOR:
        g_value_set_int(value, self->gl_minor);
        break;
    default:
        /* We don't have any other property... */
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static void
psy_gl_canvas_init(PsyGlCanvas *self)
{
    (void) self;
}

static void
psy_gl_canvas_constructed(GObject *obj)
{
    PsyGlCanvas *canvas = PSY_GL_CANVAS(obj);
    EGLint       egl_major, egl_minor, num_configs;
    GError      *error = NULL;

    G_OBJECT_CLASS(psy_gl_canvas_parent_class)->constructed(obj);

    // clang-format off
    EGLint const config_attrs[] = {
        EGL_RED_SIZE,   8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE,  8,
        EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 24,
        EGL_NONE
    };

    EGLint const context_attributes[] = {
        EGL_CONTEXT_MAJOR_VERSION, canvas->gl_major,
        EGL_CONTEXT_MINOR_VERSION, canvas->gl_minor,
        EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
        EGL_CONTEXT_OPENGL_DEBUG, canvas->debug ? EGL_TRUE : EGL_FALSE,
        EGL_NONE
    };

    EGLint const surf_attributes[] = {
        EGL_WIDTH, psy_canvas_get_width(PSY_CANVAS(canvas)),
        EGL_HEIGHT, psy_canvas_get_height(PSY_CANVAS(canvas)),
        EGL_GL_COLORSPACE, EGL_GL_COLORSPACE_LINEAR,
        EGL_NONE
    };
    // clang-format on

    canvas->display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglBindAPI(canvas->use_es ? EGL_OPENGL_ES_API : EGL_OPENGL_API);
    // eglBindAPI(EGL_OPENGL_API);
    g_assert(eglGetError() == EGL_SUCCESS);

    if (eglInitialize(canvas->display, &egl_major, &egl_minor) != EGL_TRUE)
        g_critical("Unable to initialize egl display");
    else
        g_debug("Initialized display for egl %d.%d", egl_major, egl_minor);
    g_assert(eglGetError() == EGL_SUCCESS);

    if (eglChooseConfig(
            canvas->display, config_attrs, &(canvas->config), 1, &num_configs)
        != EGL_TRUE)
        g_critical("Unable to choose an egl config");
    g_assert(eglGetError() == EGL_SUCCESS);

    canvas->egl_context = eglCreateContext(
        canvas->display, canvas->config, EGL_NO_CONTEXT, context_attributes);
    if (!canvas->egl_context) {
        EGLint error = eglGetError();
        g_critical("Unable to create egl context: %s", psy_egl_strerr(error));
    }
    g_assert(eglGetError() == EGL_SUCCESS);

    canvas->surface = eglCreatePbufferSurface(
        canvas->display, canvas->config, surf_attributes);
    if (!canvas->surface) {
        EGLint error = eglGetError();
        g_critical("Unable to create Pbuffer surface: %s",
                   psy_egl_strerr(error));
    }
    g_assert(eglGetError() == EGL_SUCCESS);

    if (eglMakeCurrent(canvas->display,
                       canvas->surface,
                       canvas->surface,
                       canvas->egl_context)
        != EGL_TRUE) {
        EGLint error = eglGetError();
        g_critical("Unable to make the context current: %s",
                   psy_egl_strerr(error));
    }

    if (canvas->debug) {
        glEnable(GL_DEBUG_OUTPUT);
        psy_gl_check_error(&error);
        if (error) {
            g_critical("Unable to enable debug output: %s", error->message);
            g_clear_error(&error);
        }

        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        psy_gl_check_error(&error);
        if (error) {
            g_critical("Unable to enable synchronous output: %s",
                       error->message);
            g_clear_error(&error);
        }

        glDebugMessageCallback(on_gl_canvas_error, canvas);
        psy_gl_check_error(&error);
        if (error) {
            g_critical("Unable to set debug callback: %s", error->message);
            g_clear_error(&error);
        }

        glDebugMessageControl(
            GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
        psy_gl_check_error(&error);
        if (error) {
            g_critical("Unable to control debug callback: %s", error->message);
            g_clear_error(&error);
        }
    }

    glEnable(GL_DEPTH_TEST);
    psy_gl_check_error(&error);
    if (error) {
        g_critical("Unable to enable depth testing: %s", error->message);
        g_clear_error(&error);
    }

    PsyCanvasClass *cls;
    cls = PSY_CANVAS_GET_CLASS(canvas);
    cls->init_default_shaders(PSY_CANVAS(canvas), &error);

    if (error) {
        g_critical("Unable to initialize shaders: %s", error->message);
        g_clear_error(&error);
    }
}

static void
psy_gl_canvas_finalize(GObject *object)
{
    PsyGlCanvas *self = PSY_GL_CANVAS(object);

    eglDestroySurface(self->display, self->surface);
    eglDestroyContext(self->display, self->egl_context);
    eglTerminate(self->display);

    G_OBJECT_CLASS(psy_gl_canvas_parent_class)->finalize(object);
}

static void
gl_canvas_draw(PsyCanvas *canvas, guint64 frame_num, PsyTimePoint *tp)
{
    PsyGlCanvas *self = PSY_GL_CANVAS(canvas);
    if (eglMakeCurrent(
            self->display, self->surface, self->surface, self->egl_context)
        != EGL_TRUE) {
        EGLint error = eglGetError();
        g_critical("Unable to make GlCanvas current: %s",
                   psy_egl_strerr(error));
    }

    PSY_CANVAS_CLASS(psy_gl_canvas_parent_class)->draw(canvas, frame_num, tp);
}

static void
gl_canvas_clear(PsyCanvas *self)
{
    // don't chain up, its not implemented in parent

    gfloat    r, b, g, a;
    GError   *error = NULL;
    PsyColor *color = psy_canvas_get_background_color(self);

    // clang-format off
    g_object_get(color,
                 "r", &r,
                 "g", &b,
                 "b", &g,
                 "a", &a,
                 NULL);
    // clang-format on

    glClearColor(r, b, g, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if (psy_gl_check_error(&error)) {
        g_critical("Unable to set background color: %s", error->message);
        g_clear_error(&error);
    }
}

static void
gl_canvas_upload_projection_matrices(PsyCanvas *self)
{
    psy_gl_canvas_upload_projection_matrices(self);
}

static void
gl_canvas_init_default_shaders(PsyCanvas *self, GError **error)
{
    psy_gl_canvas_init_default_shaders(self, error);
}

static PsyImage *
gl_canvas_get_image(PsyCanvas *canvas)
{
    GError      *error = NULL;
    PsyGlCanvas *self  = PSY_GL_CANVAS(canvas);
    PsyImage    *ret
        = PSY_CANVAS_CLASS(psy_gl_canvas_parent_class)->get_image(canvas);

    g_assert(psy_image_get_width(ret) == (guint) psy_canvas_get_width(canvas));
    g_assert(psy_image_get_height(ret)
             == (guint) psy_canvas_get_height(canvas));

    if (eglMakeCurrent(
            self->display, self->surface, self->surface, self->egl_context)
        != EGL_TRUE) {
        EGLint error = eglGetError();
        g_critical("Unable to make context current: %s", psy_egl_strerr(error));
    }

    glReadPixels(0,
                 0,
                 psy_canvas_get_width(canvas),
                 psy_canvas_get_height(canvas),
                 GL_RGBA,
                 GL_UNSIGNED_BYTE,
                 psy_image_get_ptr(ret));

    psy_gl_check_error(&error);
    if (error) {
        g_critical("unable to read pixels: %s", error->message);
        g_clear_error(&error);
    }

    glFinish();

    // In OpenGL the origin is at the left bottom not left top.
    psy_image_flip_upside_down(ret);

    return ret;
}

static void GLAPIENTRY
on_gl_canvas_error(GLenum        source,
                   GLenum        type,
                   guint         id,
                   GLenum        severity,
                   GLsizei       length,
                   const GLchar *message,
                   const void   *object)
{
    const char *source_str   = NULL;
    const char *type_str     = NULL;
    const char *severity_str = NULL;
    (void) length;

    switch (source) {
    case GL_DEBUG_SOURCE_API:
        source_str = "API";
        break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
        source_str = "WINDOW_SYSTEM";
        break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
        source_str = "SHADER_COMPILER";
        break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:
        source_str = "THIRD_PARTY";
        break;
    case GL_DEBUG_SOURCE_APPLICATION:
        source_str = "APPLICATION";
        break;
    case GL_DEBUG_SOURCE_OTHER:
        source_str = "OTHER";
        break;
    default:
        source_str = "unexpected <file bug> with the value of source";
    }

    switch (type) {
    case GL_DEBUG_TYPE_ERROR:
        type_str = "ERROR";
        break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        type_str = "DEPRECATED_BEHAVIOR";
        break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        type_str = "UNDEFINED_BEHAVIOR";
        break;
    case GL_DEBUG_TYPE_PORTABILITY:
        type_str = "PORTABILITY";
        break;
    case GL_DEBUG_TYPE_PERFORMANCE:
        type_str = "PERFORMANCE";
        break;
    case GL_DEBUG_TYPE_MARKER:
        type_str = "MARKER";
        break;
    case GL_DEBUG_TYPE_PUSH_GROUP:
        type_str = "PUSH_GROUP";
        break;
    case GL_DEBUG_TYPE_POP_GROUP:
        type_str = "POP_GROUP";
        break;
    case GL_DEBUG_TYPE_OTHER:
        type_str = "OTHER";
        break;
    default:
        type_str = "unexpected <file bug> with the value of type";
    }

    switch (severity) {
    case GL_DEBUG_SEVERITY_HIGH:
        severity_str = "HIGH";
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        severity_str = "MEDIUM";
        break;
    case GL_DEBUG_SEVERITY_LOW:
        severity_str = "LOW";
        break;
    case GL_DEBUG_SEVERITY_NOTIFICATION:
        severity_str = "NOTIFICATION";
        break;
    default:
        type_str = "unexpected <file bug> with the value of severity";
    }

    // Cast const away
    PsyGlCanvas *self = (gpointer) object;

    g_signal_emit(self,
                  signals[SIG_DEBUG_MESSAGE],
                  0,
                  (guint) source,
                  (guint) type,
                  id,
                  (guint) severity,
                  message,
                  source_str,
                  type_str,
                  severity_str,
                  NULL);
}

static void
psy_gl_canvas_class_init(PsyGlCanvasClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    object_class->constructed  = psy_gl_canvas_constructed;
    //    object_class->dispose      = psy_gl_canvas_dispose;
    object_class->finalize     = psy_gl_canvas_finalize;
    object_class->get_property = psy_gl_canvas_get_property;
    object_class->set_property = psy_gl_canvas_set_property;

    PsyCanvasClass *psy_canvas_class = PSY_CANVAS_CLASS(klass);
    psy_canvas_class->clear          = gl_canvas_clear;
    psy_canvas_class->draw           = gl_canvas_draw;
    psy_canvas_class->upload_projection_matrices
        = gl_canvas_upload_projection_matrices;
    psy_canvas_class->init_default_shaders = gl_canvas_init_default_shaders;
    psy_canvas_class->get_image            = gl_canvas_get_image;

    /**
     * PsyGlCanvas:enable-debug:
     *
     * This boolean may be set when constructing the window. This enables
     * some extra debugging features at the expense of runtime performance.
     *
     * It may be handy, to put a breakpoint in a debugger at gl_debug_cb in
     * this file. Additionally, the SIG_DEBUG signal will be emitted when
     * something is happening, so than one can get some extra information
     * about the error that occurs.
     */
    properties[PROP_ENABLE_DEBUG] = g_param_spec_boolean(
        "enable-debug",
        "EnableDebug",
        "optionally obtain extra debugging info from OpenGL calls.",
        FALSE,
        G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

    /**
     * PsyGlCanvas:use-es:
     *
     * This property allows to open context suitable for drawing with OpenGL
     * ES The value of this property must be know before a GlCanvas can be
     * created, hence it's value should be set on construtction.
     */
    properties[PROP_USE_ES]
        = g_param_spec_boolean("use-es",
                               "UseEs",
                               "Use OpenGL for Embedded Systems",
                               FALSE,
                               G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

    properties[PROP_MAJOR]
        = g_param_spec_int("gl-major",
                           "GLMajor",
                           "the major version of the OpenGL (es) context",
                           2,
                           4,
                           3,
                           G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

    properties[PROP_MINOR]
        = g_param_spec_int("gl-minor",
                           "GLMinor",
                           "the minor version of the OpenGL (es) context",
                           2,
                           4,
                           3,
                           G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

    g_object_class_install_properties(object_class, NUM_PROPS, properties);

    /**
     * PsyGlCanvas::debug-message:
     * @self: An instance of `PsyGlCanvas`
     * @source: A GLenum that specifies the source of the error
     * @type: A GLenum that specifies the type of the error
     * @id: The OpenGL id of the object where an error occurred.
     * @severity: The severity of the error
     * @message:the error message in string format
     * @source_str: The string version of @source
     * @type_str: The string version of @type
     * @severity_str: The string version of @severity
     * @data: a user specified pointer to data when the signal was connected
     *
     * This signal is emitted when the window is created with the
     * "enable-debug" property set to true. This signal is raised when the
     * OpenGL debugging context encounters something weird. It is mostly
     * useful for debugging errors related to opengl.
     */
    signals[SIG_DEBUG_MESSAGE]
        = g_signal_new("debug-message",
                       G_TYPE_FROM_CLASS(klass),
                       G_SIGNAL_RUN_FIRST | G_SIGNAL_NO_RECURSE,
                       0,
                       NULL,
                       NULL,
                       NULL,
                       G_TYPE_NONE,
                       8,
                       G_TYPE_UINT,
                       G_TYPE_UINT,
                       G_TYPE_UINT,
                       G_TYPE_UINT,
                       G_TYPE_STRING,
                       G_TYPE_STRING,
                       G_TYPE_STRING,
                       G_TYPE_STRING);
}

/**
 * psy_gl_canvas_new:(constructor)
 * @width: The desired width of the context
 * @height: The desired height of the context
 *
 * Returns: a new PsyGlCanvas. free with psy_gl_canvas_free or g_object_unref
 */
PsyGlCanvas *
psy_gl_canvas_new(gint width, gint height)
{
    g_return_val_if_fail(width >= 0, NULL);
    g_return_val_if_fail(height >= 0, NULL);

    // clang-format off
    return g_object_new(PSY_TYPE_GL_CANVAS,
                         "width", width,
                         "height", height,
                         NULL);
    // clang-format on
}

/**
 * psy_gl_canvas_new_full:(constructor)
 * @width: The desired width of the context
 * @height: The desired height of the context
 * @use_es: If true we create a context for OpenGL for Embedded Systems
 * @debug: If true, the context is created with a debug context. This
 *         may be handy for debugging, but comes with a runtime performance
 *         penalty
 * @gl_major: the major version of OpenGL.
 * @gl_minor: the minor version of OpenGL.
 *
 * This returns a PsyGlCanvas with optional support for a debug context,
 * opengl ES support. For regular opengl (default) you should aim for at
 * least 3.3 version with @gl_major and @gl_minor, for OpenGL ES, you should
 * aim for at least 2.0
 *
 * Returns: a new PsyGlCanvas. free with psy_gl_canvas_free or g_object_unref
 */
PsyGlCanvas *
psy_gl_canvas_new_full(gint     width,
                       gint     height,
                       gboolean use_es,
                       gboolean debug,
                       gint     gl_major,
                       gint     gl_minor)
{
    g_return_val_if_fail(width >= 0, NULL);
    g_return_val_if_fail(height >= 0, NULL);

    // clang-format off
    return g_object_new(PSY_TYPE_GL_CANVAS,
                        "width", width,
                        "height", height,
                        "use-es", use_es,
                        "enable-debug", debug,
                        "gl-major", gl_major,
                        "gl-minor", gl_minor,
                        NULL
                        );
    // clang-format on
}

/**
 * psy_gl_canvas_free:(skip)
 */
void
psy_gl_canvas_free(PsyGlCanvas *self)
{
    g_return_if_fail(PSY_IS_GL_CANVAS(self));
    g_object_unref(self);
}
