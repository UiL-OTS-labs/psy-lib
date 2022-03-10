

#include "ddd-gl-vbuffer.h"
#include "ddd-gl-error.h"

#include <epoxy/gl.h>

typedef struct _DddGlVBuffer {
    DddVBuffer           parent;
    GLuint               vertex_buffer_id;
    GLuint               vertex_array_id;
    guint                is_uploaded : 1;
} DddGlVBuffer;

G_DEFINE_TYPE(DddGlVBuffer, ddd_gl_vbuffer, DDD_TYPE_VBUFFER)

typedef enum {
    PROP_NULL,
    PROP_OBJECT_ID,
    NUM_PROPERTIES
} DddGlVBufferProperty;

static GParamSpec* gl_vbuffer_properties[NUM_PROPERTIES];

static void
ddd_gl_vbuffer_set_property(GObject        *object,
                           guint           prop_id,
                           const GValue   *value,
                           GParamSpec     *pspec)
{
    DddGlVBuffer* self = DDD_GL_VBUFFER(object);
    (void) self, (void) value;

    switch((DddGlVBufferProperty) prop_id) {
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

static void
ddd_gl_vbuffer_get_property(GObject    *object,
                           guint       prop_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
    DddGlVBuffer* self = DDD_GL_VBUFFER(object);

    switch((DddGlVBufferProperty) prop_id) {
        case PROP_OBJECT_ID:
            g_value_set_uint(value, self->vertex_buffer_id);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

static void
ddd_gl_vbuffer_init(DddGlVBuffer *self)
{
    self->vertex_buffer_id = 0;
    self->is_uploaded = 0;
}

static void
ddd_gl_vbuffer_dispose(GObject* object)
{
    DddGlVBuffer* self = DDD_GL_VBUFFER(object);
    (void) self;

    G_OBJECT_CLASS(ddd_gl_vbuffer_parent_class)->dispose(object);
}

static void
ddd_gl_vbuffer_finalize(GObject* object)
{
    DddGlVBuffer* self = DDD_GL_VBUFFER(object);

    if (self->vertex_buffer_id) {
        glDeleteBuffers(1, &self->vertex_buffer_id);
        self->vertex_buffer_id = 0;
    }

    if (self->vertex_array_id) {
        glDeleteVertexArrays(1, &self->vertex_array_id);
        self->vertex_array_id = 0;
    }

    G_OBJECT_CLASS(ddd_gl_vbuffer_parent_class)->dispose(object);
}

static void
ddd_gl_vbuffer_upload(DddVBuffer* vbuffer, GError **error)
{
    DddGlVBuffer *self = DDD_GL_VBUFFER(vbuffer);
    if (self->vertex_buffer_id) {
        glDeleteBuffers(1, &self->vertex_buffer_id);
        self->vertex_buffer_id = 0;
    }

    if (self->vertex_array_id) {
        glDeleteVertexArrays(1, &self->vertex_array_id);
        self->vertex_array_id = 0;
    }

    glGenBuffers(1, &self->vertex_buffer_id);
    ddd_gl_check_error(error);
    if (*error)
        return;
    glBindBuffer(GL_ARRAY_BUFFER, self->vertex_buffer_id);
    ddd_gl_check_error(error);
    if (*error)
        return;
    glBufferData(
            GL_ARRAY_BUFFER,
            (GLsizeiptr) ddd_vbuffer_get_size(vbuffer),
            ddd_vbuffer_get_buffer(vbuffer),
            GL_STATIC_DRAW
            );
    ddd_gl_check_error(error);
    if (*error)
        return;

    glGenVertexArrays(1, &self->vertex_array_id);
    if (ddd_gl_check_error(error))
        return;

    glBindVertexArray(self->vertex_array_id);
    if (ddd_gl_check_error(error))
        return;

    // Enable vertex position
    glVertexAttribPointer(0,
                          sizeof(DddVertexPos) / sizeof (gfloat),
                          GL_FLOAT,
                          GL_FALSE,
                          sizeof(DddVertex),
                          (void*) G_STRUCT_OFFSET(DddVertex, pos));
    if(ddd_gl_check_error(error))
        return;

    glEnableVertexAttribArray(0);
    if (ddd_gl_check_error(error))
        return;

    // enable vertex color
    glVertexAttribPointer(1,
                          sizeof(DddVertexColor) / sizeof (gfloat),
                          GL_FLOAT,
                          GL_FALSE,
                          sizeof(DddVertex),
                          (void*) G_STRUCT_OFFSET(DddVertex, color));
    if(ddd_gl_check_error(error))
        return;

    glEnableVertexAttribArray(1);
    if (ddd_gl_check_error(error))
        return;

    // enable vertex texture position
    glVertexAttribPointer(2,
                          sizeof(DddVertexTexPos) / sizeof (gfloat),
                          GL_FLOAT,
                          GL_FALSE,
                          sizeof(DddVertex),
                          (void*) G_STRUCT_OFFSET(DddVertex, texture_pos));
    if(ddd_gl_check_error(error))
        return;

    glEnableVertexAttribArray(2);
    if (ddd_gl_check_error(error))
        return;

    self->is_uploaded = true;
}

static gboolean
ddd_gl_vbuffer_is_uploaded(DddVBuffer* self)
{
    DddGlVBuffer* gl_vbuffer = DDD_GL_VBUFFER(self);
    return gl_vbuffer->is_uploaded;
}

static void
ddd_gl_vbuffer_draw_triangles(DddVBuffer *self, GError **error)
{
    DddGlVBuffer *gl_vbuffer = DDD_GL_VBUFFER(self);
    if (!ddd_gl_vbuffer_is_uploaded(self))
        ddd_gl_vbuffer_upload(self, error);
    if (*error)
        return;

    glBindVertexArray(gl_vbuffer->vertex_array_id);
#ifndef NDEBUG
    ddd_gl_check_error(error);
    if (*error) {
        g_assert_not_reached();
        return;
    }
#endif
    glDrawArrays(GL_TRIANGLES, 0, (GLint)ddd_vbuffer_get_nvertices(self));
    ddd_gl_check_error(error);
}

static void
ddd_gl_vbuffer_draw_triangle_strip(DddVBuffer *self, GError **error)
{
    DddGlVBuffer *gl_vbuffer = DDD_GL_VBUFFER(self);
    if (!ddd_gl_vbuffer_is_uploaded(self))
        ddd_gl_vbuffer_upload(self, error);
    if (*error)
        return;

    glBindVertexArray(gl_vbuffer->vertex_array_id);
#ifndef NDEBUG
    if (ddd_gl_check_error(error))
        g_assert_not_reached();
#endif
    glDrawArrays(GL_TRIANGLE_STRIP, 0, (GLint)ddd_vbuffer_get_nvertices(self));
    ddd_gl_check_error(error);
}

static void
ddd_gl_vbuffer_draw_triangle_fan(DddVBuffer *self, GError **error)
{
    DddGlVBuffer *gl_vbuffer = DDD_GL_VBUFFER(self);
    if (!ddd_gl_vbuffer_is_uploaded(self))
        ddd_gl_vbuffer_upload(self, error);
    if (*error)
        return;

    glBindVertexArray(gl_vbuffer->vertex_array_id);
#ifndef NDEBUG
    if (ddd_gl_check_error(error))
        g_assert_not_reached();
#endif
    glDrawArrays(GL_TRIANGLE_FAN, 0, (GLint)ddd_vbuffer_get_nvertices(self));
    ddd_gl_check_error(error);
}

static void
ddd_gl_vbuffer_class_init(DddGlVBufferClass* class)
{
    GObjectClass       *gobject_class = G_OBJECT_CLASS(class);
    DddVBufferClass    *vbuffer_class = DDD_VBUFFER_CLASS(class);

    gobject_class->set_property = ddd_gl_vbuffer_set_property;
    gobject_class->get_property = ddd_gl_vbuffer_get_property;
    gobject_class->finalize     = ddd_gl_vbuffer_finalize;
    gobject_class->dispose      = ddd_gl_vbuffer_dispose;

    vbuffer_class->upload               = ddd_gl_vbuffer_upload;
    vbuffer_class->is_uploaded          = ddd_gl_vbuffer_is_uploaded;
    vbuffer_class->draw_triangles       = ddd_gl_vbuffer_draw_triangles;
    vbuffer_class->draw_triangle_strip  = ddd_gl_vbuffer_draw_triangle_strip;
    vbuffer_class->draw_triangle_fan    = ddd_gl_vbuffer_draw_triangle_fan;

    gl_vbuffer_properties[PROP_OBJECT_ID] = g_param_spec_string(
            "object-id",
            "Object ID",
            "The OpenGL id of the object",
            0,
            G_PARAM_READWRITE
            );

    g_object_class_install_properties(gobject_class,
                                      NUM_PROPERTIES,
                                      gl_vbuffer_properties);
}

/* ************ public functions ******************** */

DddGlVBuffer*
ddd_gl_vbuffer_new()
{
    DddGlVBuffer *gl_vbuffer = g_object_new(DDD_TYPE_GL_VBUFFER, NULL);
    return gl_vbuffer;
}

guint
ddd_gl_vbuffer_get_object_id(DddGlVBuffer* self) {
    g_return_val_if_fail(DDD_IS_VBUFFER(self), 0);
    
    return self->vertex_buffer_id;
}

