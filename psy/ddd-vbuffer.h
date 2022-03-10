
#ifndef  DDD_VBUFFER_H
#define  DDD_VBUFFER_H

#include <glib-object.h>
#include <gio/gio.h>

G_BEGIN_DECLS

#define DDD_TYPE_VBUFFER ddd_vbuffer_get_type()
G_DECLARE_DERIVABLE_TYPE(DddVBuffer, ddd_vbuffer, DDD, VBUFFER, GObject)

typedef struct DddVertexPos {
    gfloat x, y, z;
} DddVertexPos;

typedef struct DddVertexColor {
    gfloat r, g, b, a;
} DddVertexColor;

typedef struct DddVertexTexPos {
    gfloat s, t;
} DddVertexTexPos;

typedef struct DddVertex {
    DddVertexPos    pos;
    DddVertexColor  color;
    DddVertexTexPos texture_pos;
} DddVertex;

typedef struct _DddVBufferClass {
    GObjectClass parent_class;
    void     (*upload)      (DddVBuffer* self, GError **error);
    gboolean (*is_uploaded) (DddVBuffer* self);

    void     (*draw_triangles) (DddVBuffer* self, GError **error);
    void     (*draw_triangle_strip)(DddVBuffer* self, GError **error);
    void     (*draw_triangle_fan)(DddVBuffer* self, GError **error);
} DddVBufferClass;

G_MODULE_EXPORT void
ddd_vbuffer_upload(DddVBuffer* self, GError **error);

G_MODULE_EXPORT gboolean
ddd_vbuffer_is_uploaded(DddVBuffer* self);

G_MODULE_EXPORT void
ddd_vbuffer_set_nvertices(DddVBuffer* self, guint nverts);

G_MODULE_EXPORT guint
ddd_vbuffer_get_nvertices(DddVBuffer* self);

G_MODULE_EXPORT gsize
ddd_vbuffer_get_size (DddVBuffer* self);

G_MODULE_EXPORT const guint8*
ddd_vbuffer_get_buffer(DddVBuffer* self);

G_MODULE_EXPORT gboolean
ddd_vbuffer_set_from_data(DddVBuffer * self, const void *data, guint nverts);

G_MODULE_EXPORT gboolean
ddd_vbuffer_set_x(DddVBuffer *self, guint i, gfloat x);

G_MODULE_EXPORT gboolean
ddd_vbuffer_set_y(DddVBuffer *self, guint i, gfloat y);

G_MODULE_EXPORT gboolean
ddd_vbuffer_set_z(DddVBuffer *self, guint i, gfloat z);

G_MODULE_EXPORT gboolean
ddd_vbuffer_set_xyz(DddVBuffer *self, guint i, gfloat x, gfloat y, gfloat z);

G_MODULE_EXPORT gboolean
ddd_vbuffer_set_pos(DddVBuffer *self, guint i, DddVertexPos* pos);

G_MODULE_EXPORT gboolean
ddd_vbuffer_get_pos(DddVBuffer* self, guint i, DddVertexPos* pos);

G_MODULE_EXPORT gboolean
ddd_vbuffer_set_r(DddVBuffer* self, guint i, gfloat r);

G_MODULE_EXPORT gboolean
ddd_vbuffer_set_g(DddVBuffer* self, guint i, gfloat g);

G_MODULE_EXPORT gboolean
ddd_vbuffer_set_b(DddVBuffer* self, guint i, gfloat b);

G_MODULE_EXPORT gboolean
ddd_vbuffer_set_a(DddVBuffer* self, guint i, gfloat a);

G_MODULE_EXPORT gboolean
ddd_vbuffer_set_rgb(DddVBuffer* self, guint i, gfloat r, gfloat g, gfloat b);

G_MODULE_EXPORT gboolean
ddd_vbuffer_set_rgba(
        DddVBuffer* self, guint i, gfloat r, gfloat g, gfloat b, gfloat a
        );

G_MODULE_EXPORT gboolean
ddd_vbuffer_set_color (DddVBuffer* self, guint i, DddVertexColor* color);

G_MODULE_EXPORT gboolean
ddd_vbuffer_get_color (DddVBuffer* self, guint i, DddVertexColor* color);

G_MODULE_EXPORT gboolean
ddd_vbuffer_set_s(DddVBuffer* self, guint i, gfloat s);

G_MODULE_EXPORT gboolean
ddd_vbuffer_set_t(DddVBuffer* self, guint i, gfloat t);

G_MODULE_EXPORT gboolean
ddd_vbuffer_set_texture_pos(DddVBuffer* self, guint i, DddVertexTexPos* tpos);

G_MODULE_EXPORT gboolean
ddd_vbuffer_get_texture_pos(DddVBuffer* self, guint i, DddVertexTexPos* tpos);

G_MODULE_EXPORT void
ddd_vbuffer_draw_triangles(DddVBuffer* self, GError **error);

G_MODULE_EXPORT void
ddd_vbuffer_draw_triangle_strip(DddVBuffer* self, GError **error);

G_MODULE_EXPORT void
ddd_vbuffer_draw_triangle_fan(DddVBuffer* self, GError **error);

G_END_DECLS


#endif