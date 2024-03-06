
#ifndef PSY_VBUFFER_H
#define PSY_VBUFFER_H

#include <gio/gio.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define PSY_TYPE_VBUFFER psy_vbuffer_get_type()

G_MODULE_EXPORT
G_DECLARE_DERIVABLE_TYPE(PsyVBuffer, psy_vbuffer, PSY, VBUFFER, GObject)

typedef struct PsyVertexPos {
    gfloat x, y, z;
} PsyVertexPos;

typedef struct PsyVertexColor {
    gfloat r, g, b, a;
} PsyVertexColor;

typedef struct PsyVertexTexPos {
    gfloat s, t;
} PsyVertexTexPos;

typedef struct PsyVertex {
    PsyVertexPos    pos;
    PsyVertexColor  color;
    PsyVertexTexPos texture_pos;
} PsyVertex;

typedef struct _PsyVBufferClass {
    GObjectClass parent_class;
    void (*upload)(PsyVBuffer *self, GError **error);
    gboolean (*is_uploaded)(PsyVBuffer *self);

    void (*draw_triangles)(PsyVBuffer *self, GError **error);
    void (*draw_triangle_strip)(PsyVBuffer *self, GError **error);
    void (*draw_triangle_fan)(PsyVBuffer *self, GError **error);
} PsyVBufferClass;

G_MODULE_EXPORT void
psy_vbuffer_upload(PsyVBuffer *self, GError **error);

G_MODULE_EXPORT gboolean
psy_vbuffer_is_uploaded(PsyVBuffer *self);

G_MODULE_EXPORT void
psy_vbuffer_set_nvertices(PsyVBuffer *self, guint nverts);

G_MODULE_EXPORT guint
psy_vbuffer_get_nvertices(PsyVBuffer *self);

G_MODULE_EXPORT gsize
psy_vbuffer_get_size(PsyVBuffer *self);

G_MODULE_EXPORT const guint8 *
psy_vbuffer_get_buffer(PsyVBuffer *self);

G_MODULE_EXPORT gboolean
psy_vbuffer_set_from_data(PsyVBuffer *self, const void *data, guint nverts);

G_MODULE_EXPORT gboolean
psy_vbuffer_set_x(PsyVBuffer *self, guint i, gfloat x);

G_MODULE_EXPORT gboolean
psy_vbuffer_set_y(PsyVBuffer *self, guint i, gfloat y);

G_MODULE_EXPORT gboolean
psy_vbuffer_set_z(PsyVBuffer *self, guint i, gfloat z);

G_MODULE_EXPORT gboolean
psy_vbuffer_set_xyz(PsyVBuffer *self, guint i, gfloat x, gfloat y, gfloat z);

G_MODULE_EXPORT gboolean
psy_vbuffer_set_pos(PsyVBuffer *self, guint i, PsyVertexPos *pos);

G_MODULE_EXPORT gboolean
psy_vbuffer_get_pos(PsyVBuffer *self, guint i, PsyVertexPos *pos);

G_MODULE_EXPORT gboolean
psy_vbuffer_set_r(PsyVBuffer *self, guint i, gfloat r);

G_MODULE_EXPORT gboolean
psy_vbuffer_set_g(PsyVBuffer *self, guint i, gfloat g);

G_MODULE_EXPORT gboolean
psy_vbuffer_set_b(PsyVBuffer *self, guint i, gfloat b);

G_MODULE_EXPORT gboolean
psy_vbuffer_set_a(PsyVBuffer *self, guint i, gfloat a);

G_MODULE_EXPORT gboolean
psy_vbuffer_set_rgb(PsyVBuffer *self, guint i, gfloat r, gfloat g, gfloat b);

G_MODULE_EXPORT gboolean
psy_vbuffer_set_rgba(
    PsyVBuffer *self, guint i, gfloat r, gfloat g, gfloat b, gfloat a);

G_MODULE_EXPORT gboolean
psy_vbuffer_set_color(PsyVBuffer *self, guint i, PsyVertexColor *color);

G_MODULE_EXPORT gboolean
psy_vbuffer_get_color(PsyVBuffer *self, guint i, PsyVertexColor *color);

G_MODULE_EXPORT gboolean
psy_vbuffer_set_s(PsyVBuffer *self, guint i, gfloat s);

G_MODULE_EXPORT gboolean
psy_vbuffer_set_t(PsyVBuffer *self, guint i, gfloat t);

G_MODULE_EXPORT gboolean
psy_vbuffer_set_texture_pos(PsyVBuffer *self, guint i, PsyVertexTexPos *tpos);

G_MODULE_EXPORT gboolean
psy_vbuffer_get_texture_pos(PsyVBuffer *self, guint i, PsyVertexTexPos *tpos);

G_MODULE_EXPORT void
psy_vbuffer_draw_triangles(PsyVBuffer *self, GError **error);

G_MODULE_EXPORT void
psy_vbuffer_draw_triangle_strip(PsyVBuffer *self, GError **error);

G_MODULE_EXPORT void
psy_vbuffer_draw_triangle_fan(PsyVBuffer *self, GError **error);

G_END_DECLS

#endif
