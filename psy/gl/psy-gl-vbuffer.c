

#include "psy-gl-vbuffer.h"
#include "psy-gl-error.h"

#include <epoxy/gl.h>

typedef struct _PsyGlVBuffer {
    PsyVBuffer parent;
    GLuint     vertex_buffer_id;
    GLuint     vertex_array_id;
    guint      is_uploaded : 1;
} PsyGlVBuffer;

G_DEFINE_TYPE(PsyGlVBuffer, psy_gl_vbuffer, PSY_TYPE_VBUFFER)

typedef enum { PROP_NULL, PROP_OBJECT_ID, NUM_PROPERTIES } PsyGlVBufferProperty;

static GParamSpec *gl_vbuffer_properties[NUM_PROPERTIES];

static void
psy_gl_vbuffer_set_property(GObject      *object,
                            guint         prop_id,
                            const GValue *value,
                            GParamSpec   *pspec)
{
    PsyGlVBuffer *self = PSY_GL_VBUFFER(object);
    (void) self, (void) value;

    switch ((PsyGlVBufferProperty) prop_id) {
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

static void
psy_gl_vbuffer_get_property(GObject    *object,
                            guint       prop_id,
                            GValue     *value,
                            GParamSpec *pspec)
{
    PsyGlVBuffer *self = PSY_GL_VBUFFER(object);

    switch ((PsyGlVBufferProperty) prop_id) {
    case PROP_OBJECT_ID:
        g_value_set_uint(value, self->vertex_buffer_id);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

static void
psy_gl_vbuffer_init(PsyGlVBuffer *self)
{
    self->vertex_buffer_id = 0;
    self->is_uploaded      = 0;
}

static void
psy_gl_vbuffer_dispose(GObject *object)
{
    PsyGlVBuffer *self = PSY_GL_VBUFFER(object);
    (void) self;

    G_OBJECT_CLASS(psy_gl_vbuffer_parent_class)->dispose(object);
}

static void
psy_gl_vbuffer_finalize(GObject *object)
{
    PsyGlVBuffer *self = PSY_GL_VBUFFER(object);

    if (self->vertex_buffer_id) {
        glDeleteBuffers(1, &self->vertex_buffer_id);
        self->vertex_buffer_id = 0;
    }

    if (self->vertex_array_id) {
        glDeleteVertexArrays(1, &self->vertex_array_id);
        self->vertex_array_id = 0;
    }

    G_OBJECT_CLASS(psy_gl_vbuffer_parent_class)->finalize(object);
}

static void
psy_gl_vbuffer_upload(PsyVBuffer *vbuffer, GError **error)
{
    PsyGlVBuffer *self = PSY_GL_VBUFFER(vbuffer);
    if (self->vertex_buffer_id) {
        glDeleteBuffers(1, &self->vertex_buffer_id);
        self->vertex_buffer_id = 0;
    }

    if (self->vertex_array_id) {
        glDeleteVertexArrays(1, &self->vertex_array_id);
        self->vertex_array_id = 0;
    }

    glGenBuffers(1, &self->vertex_buffer_id);
    psy_gl_check_error(error);
    if (*error)
        return;
    glBindBuffer(GL_ARRAY_BUFFER, self->vertex_buffer_id);
    psy_gl_check_error(error);
    if (*error)
        return;
    glBufferData(GL_ARRAY_BUFFER,
                 (GLsizeiptr) psy_vbuffer_get_size(vbuffer),
                 psy_vbuffer_get_buffer(vbuffer),
                 GL_STATIC_DRAW);
    psy_gl_check_error(error);
    if (*error)
        return;

    glGenVertexArrays(1, &self->vertex_array_id);
    if (psy_gl_check_error(error))
        return;

    glBindVertexArray(self->vertex_array_id);
    if (psy_gl_check_error(error))
        return;

    // Enable vertex position
    glVertexAttribPointer(0,
                          sizeof(PsyVertexPos) / sizeof(gfloat),
                          GL_FLOAT,
                          GL_FALSE,
                          sizeof(PsyVertex),
                          (void *) G_STRUCT_OFFSET(PsyVertex, pos));
    if (psy_gl_check_error(error))
        return;

    glEnableVertexAttribArray(0);
    if (psy_gl_check_error(error))
        return;

    // enable vertex color
    glVertexAttribPointer(1,
                          sizeof(PsyVertexColor) / sizeof(gfloat),
                          GL_FLOAT,
                          GL_FALSE,
                          sizeof(PsyVertex),
                          (void *) G_STRUCT_OFFSET(PsyVertex, color));
    if (psy_gl_check_error(error))
        return;

    glEnableVertexAttribArray(1);
    if (psy_gl_check_error(error))
        return;

    // enable vertex texture position
    glVertexAttribPointer(2,
                          sizeof(PsyVertexTexPos) / sizeof(gfloat),
                          GL_FLOAT,
                          GL_FALSE,
                          sizeof(PsyVertex),
                          (void *) G_STRUCT_OFFSET(PsyVertex, texture_pos));
    if (psy_gl_check_error(error))
        return;

    glEnableVertexAttribArray(2);
    if (psy_gl_check_error(error))
        return;

    self->is_uploaded = true;
}

static gboolean
psy_gl_vbuffer_is_uploaded(PsyVBuffer *self)
{
    PsyGlVBuffer *gl_vbuffer = PSY_GL_VBUFFER(self);
    return gl_vbuffer->is_uploaded;
}

static void
psy_gl_vbuffer_draw_triangles(PsyVBuffer *self, GError **error)
{
    PsyGlVBuffer *gl_vbuffer = PSY_GL_VBUFFER(self);
    if (!psy_gl_vbuffer_is_uploaded(self))
        psy_gl_vbuffer_upload(self, error);
    if (*error)
        return;

    glBindVertexArray(gl_vbuffer->vertex_array_id);
#ifndef NDEBUG
    psy_gl_check_error(error);
    if (*error) {
        g_assert_not_reached();
        return;
    }
#endif
    glDrawArrays(GL_TRIANGLES, 0, (GLint) psy_vbuffer_get_nvertices(self));
    psy_gl_check_error(error);
}

static void
psy_gl_vbuffer_draw_triangle_strip(PsyVBuffer *self, GError **error)
{
    PsyGlVBuffer *gl_vbuffer = PSY_GL_VBUFFER(self);
    if (!psy_gl_vbuffer_is_uploaded(self))
        psy_gl_vbuffer_upload(self, error);
    if (*error)
        return;

    glBindVertexArray(gl_vbuffer->vertex_array_id);
#ifndef NDEBUG
    if (psy_gl_check_error(error))
        g_assert_not_reached();
#endif
    glDrawArrays(GL_TRIANGLE_STRIP, 0, (GLint) psy_vbuffer_get_nvertices(self));
    psy_gl_check_error(error);
}

static void
psy_gl_vbuffer_draw_triangle_fan(PsyVBuffer *self, GError **error)
{
    PsyGlVBuffer *gl_vbuffer = PSY_GL_VBUFFER(self);
    if (!psy_gl_vbuffer_is_uploaded(self))
        psy_gl_vbuffer_upload(self, error);
    if (*error)
        return;

    glBindVertexArray(gl_vbuffer->vertex_array_id);
#ifndef NDEBUG
    if (psy_gl_check_error(error))
        g_assert_not_reached();
#endif
    glDrawArrays(GL_TRIANGLE_FAN, 0, (GLint) psy_vbuffer_get_nvertices(self));
    psy_gl_check_error(error);
}

static void
psy_gl_vbuffer_class_init(PsyGlVBufferClass *class)
{
    GObjectClass    *gobject_class = G_OBJECT_CLASS(class);
    PsyVBufferClass *vbuffer_class = PSY_VBUFFER_CLASS(class);

    gobject_class->set_property = psy_gl_vbuffer_set_property;
    gobject_class->get_property = psy_gl_vbuffer_get_property;
    gobject_class->finalize     = psy_gl_vbuffer_finalize;
    gobject_class->dispose      = psy_gl_vbuffer_dispose;

    vbuffer_class->upload              = psy_gl_vbuffer_upload;
    vbuffer_class->is_uploaded         = psy_gl_vbuffer_is_uploaded;
    vbuffer_class->draw_triangles      = psy_gl_vbuffer_draw_triangles;
    vbuffer_class->draw_triangle_strip = psy_gl_vbuffer_draw_triangle_strip;
    vbuffer_class->draw_triangle_fan   = psy_gl_vbuffer_draw_triangle_fan;

    gl_vbuffer_properties[PROP_OBJECT_ID]
        = g_param_spec_string("object-id",
                              "Object ID",
                              "The OpenGL id of the object",
                              0,
                              G_PARAM_READWRITE);

    g_object_class_install_properties(
        gobject_class, NUM_PROPERTIES, gl_vbuffer_properties);
}

/* ************ public functions ******************** */

/**
 * psy_gl_vbuffer_new:(constructor)
 *
 * Create a new PsyGlVBuffer object.
 *
 * Returns: A new instance of [class@PsyGlVBuffer] that should be freed
 * with g_object_unref or psy_gl_vbuffer_free.
 */
PsyGlVBuffer *
psy_gl_vbuffer_new(void)
{
    PsyGlVBuffer *gl_vbuffer = g_object_new(PSY_TYPE_GL_VBUFFER, NULL);
    return gl_vbuffer;
}

/**
 * psy_gl_vbuffer_free:(skip)
 *
 * Free a PsyGlVBuffer object. Previously created with psy_gl_vbuffer_new.
 */
void
psy_gl_vbuffer_free(PsyGlVBuffer *self)
{
    g_return_if_fail(PSY_IS_GL_VBUFFER(self));
    g_object_unref(self);
}

guint
psy_gl_vbuffer_get_object_id(PsyGlVBuffer *self)
{
    g_return_val_if_fail(PSY_IS_VBUFFER(self), 0);

    return self->vertex_buffer_id;
}
