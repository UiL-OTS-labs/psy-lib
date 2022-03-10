
#include "psy-vbuffer.h"

typedef struct PsyShaderPrivate {
    GArray *vertices;
} PsyVBufferPrivate;

G_DEFINE_TYPE_WITH_PRIVATE(PsyVBuffer, psy_vbuffer, G_TYPE_OBJECT)

typedef enum {
    PROP_NULL,
    PROP_NVERTS,
    PROP_MEMSIZE,
    PROP_UPLOADED,
    NUM_PROPERTIES
} PsyVBufferProperty;

static GParamSpec* vbuffer_properties[NUM_PROPERTIES] = {0};

static void
psy_vbuffer_set_property(GObject       *object,
                         guint          prop_id,
                         const GValue  *value,
                         GParamSpec    *pspec)
{
    PsyVBuffer *self = PSY_VBUFFER(object);

    switch((PsyVBufferProperty) prop_id) {
        case PROP_NVERTS:
            psy_vbuffer_set_nvertices(self, g_value_get_uint(value));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

static void
psy_vbuffer_get_property(GObject       *object,
                         guint          prop_id,
                         GValue        *value,
                         GParamSpec    *pspec)
{
    PsyVBuffer *self = PSY_VBUFFER(object);

    switch((PsyVBufferProperty) prop_id) {
        case PROP_NVERTS:
            g_value_set_uint(value, psy_vbuffer_get_nvertices(self));
            break;
        case PROP_MEMSIZE:
            g_value_set_uint64(value, psy_vbuffer_get_size(self));
            break;
        case PROP_UPLOADED:
            g_value_set_boolean(value, psy_vbuffer_is_uploaded(self));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

static void
psy_vbuffer_init(PsyVBuffer* self)
{
    PsyVBufferPrivate * priv = psy_vbuffer_get_instance_private(self);
    priv->vertices = g_array_new(FALSE, TRUE, sizeof(PsyVertex));
}

static void
psy_vbuffer_dispose(GObject* object)
{
    PsyVBuffer *self = PSY_VBUFFER(object);
    PsyVBufferPrivate *priv = psy_vbuffer_get_instance_private(self);

    if (priv->vertices) {
        g_array_unref(priv->vertices);
        priv->vertices = NULL;
    }
    G_OBJECT_CLASS(psy_vbuffer_parent_class)->dispose(object);
}

static void
psy_vbuffer_finalize(GObject* object)
{
    G_OBJECT_CLASS(psy_vbuffer_parent_class)->finalize(object);
}

static void
psy_vbuffer_class_init(PsyVBufferClass* klass)
{
    GObjectClass* gobject_class = G_OBJECT_CLASS(klass);

    gobject_class->set_property = psy_vbuffer_set_property;
    gobject_class->get_property = psy_vbuffer_get_property;
    gobject_class->dispose      = psy_vbuffer_dispose;
    gobject_class->finalize     = psy_vbuffer_finalize;

    vbuffer_properties[PROP_NVERTS] = g_param_spec_uint(
            "num-vertices",
            "NVerts",
            "The number of vertices in this buffer",
            0,
            G_MAXUINT,
            0,
            G_PARAM_READWRITE
            );

    vbuffer_properties[PROP_MEMSIZE] = g_param_spec_uint64(
            "mem-size",
            "memory-size",
            "The size in bytes that the buffer will occupy",
            0,
            G_MAXUINT64,
            0,
            G_PARAM_READABLE
            );

    vbuffer_properties[PROP_UPLOADED] = g_param_spec_boolean(
            "uploaded",
            "Uploaded",
            "Whether or not the buffer is uploaded to the GPU.",
            FALSE,
            G_PARAM_READABLE
            );

    g_object_class_install_properties(
            gobject_class, NUM_PROPERTIES, vbuffer_properties
            );
}

/* ************** public function ************************ */

void
psy_vbuffer_upload(PsyVBuffer* self, GError **error)
{
    g_return_if_fail(PSY_IS_VBUFFER(self));
    g_return_if_fail(error != NULL && *error == NULL);

    PsyVBufferClass *klass = PSY_VBUFFER_GET_CLASS(self);

    g_return_if_fail(klass->upload);
    klass->upload(self, error);
}

gboolean
psy_vbuffer_is_uploaded(PsyVBuffer* self)
{
    g_return_val_if_fail(PSY_IS_VBUFFER(self), FALSE);

    PsyVBufferClass *klass = PSY_VBUFFER_GET_CLASS(self);

    g_return_val_if_fail(klass->is_uploaded, FALSE);
    return klass->is_uploaded(self);
}

void
psy_vbuffer_draw_triangles(PsyVBuffer *self, GError **error)
{
    g_return_if_fail(PSY_IS_VBUFFER(self));
    g_return_if_fail(error == NULL || *error == NULL);

    PsyVBufferClass* klass = PSY_VBUFFER_GET_CLASS(self);

    g_return_if_fail(klass->draw_triangles);

    klass->draw_triangles(self, error);
}

void
psy_vbuffer_draw_triangle_strip(PsyVBuffer *self, GError **error)
{
    g_return_if_fail(PSY_IS_VBUFFER(self));
    g_return_if_fail(error == NULL || *error == NULL);

    PsyVBufferClass* klass = PSY_VBUFFER_GET_CLASS(self);

    g_return_if_fail(klass->draw_triangle_strip);

    klass->draw_triangle_strip(self, error);
}

void
psy_vbuffer_draw_triangle_fan(PsyVBuffer *self, GError **error)
{
    g_return_if_fail(PSY_IS_VBUFFER(self));
    g_return_if_fail(error == NULL || *error == NULL);

    PsyVBufferClass* klass = PSY_VBUFFER_GET_CLASS(self);

    g_return_if_fail(klass->draw_triangle_fan);

    klass->draw_triangle_fan(self, error);
}

void
psy_vbuffer_set_nvertices(PsyVBuffer* self, guint nverts)
{
    g_return_if_fail(PSY_IS_VBUFFER(self));
    PsyVBufferPrivate *priv = psy_vbuffer_get_instance_private(self);

    g_array_set_size(priv->vertices, nverts);
}

guint
psy_vbuffer_get_nvertices(PsyVBuffer* self)
{
    g_return_val_if_fail(PSY_IS_VBUFFER(self), (guint) 0);
    PsyVBufferPrivate *priv = psy_vbuffer_get_instance_private(self);

    return priv->vertices->len;
}

gsize
psy_vbuffer_get_size (PsyVBuffer* self)
{
    g_return_val_if_fail(PSY_IS_VBUFFER(self), 0);
    PsyVBufferPrivate *priv = psy_vbuffer_get_instance_private(self);

    return priv->vertices->len * sizeof(PsyVertex);
}

const guint8*
psy_vbuffer_get_buffer(PsyVBuffer* self)
{
    g_return_val_if_fail(PSY_IS_VBUFFER(self), NULL);
    PsyVBufferPrivate *priv = psy_vbuffer_get_instance_private(self);

    return (guint8*) priv->vertices->data;
}

gboolean
psy_vbuffer_set_from_data(PsyVBuffer* self, const void* data, guint nverts)
{
    g_return_val_if_fail(PSY_IS_VBUFFER(self), FALSE);
    PsyVBufferPrivate *priv = psy_vbuffer_get_instance_private(self);

    g_array_set_size(priv->vertices, nverts);
    if (psy_vbuffer_get_nvertices(self) != nverts)
        return FALSE;
    memcpy(priv->vertices->data, data, psy_vbuffer_get_size(self));
    return TRUE;
}

gboolean
psy_vbuffer_set_x(PsyVBuffer* self, guint i, gfloat x)
{
    g_return_val_if_fail(PSY_IS_VBUFFER(self), FALSE);
    PsyVBufferPrivate *priv = psy_vbuffer_get_instance_private(self);

    if (i >= psy_vbuffer_get_nvertices(self))
        return FALSE;

    PsyVertex *vert = &g_array_index(priv->vertices, PsyVertex, i);
    vert->pos.x = x;

    return TRUE;
}

gboolean
psy_vbuffer_set_y(PsyVBuffer* self, guint i, gfloat y)
{
    g_return_val_if_fail(PSY_IS_VBUFFER(self), FALSE);
    PsyVBufferPrivate *priv = psy_vbuffer_get_instance_private(self);

    if (i >= psy_vbuffer_get_nvertices(self))
        return FALSE;

    PsyVertex *vert = &g_array_index(priv->vertices, PsyVertex, i);
    vert->pos.y = y;

    return TRUE;
}

gboolean
psy_vbuffer_set_z(PsyVBuffer* self, guint i, gfloat z)
{
    g_return_val_if_fail(PSY_IS_VBUFFER(self), FALSE);
    PsyVBufferPrivate *priv = psy_vbuffer_get_instance_private(self);

    if (i >= psy_vbuffer_get_nvertices(self))
        return FALSE;

    PsyVertex *vert = &g_array_index(priv->vertices, PsyVertex, i);
    vert->pos.z = z;

    return TRUE;
}

gboolean
psy_vbuffer_set_xyz(PsyVBuffer* self, guint i, gfloat x, gfloat y, gfloat z)
{
    g_return_val_if_fail(PSY_IS_VBUFFER(self), FALSE);
    PsyVBufferPrivate *priv = psy_vbuffer_get_instance_private(self);

    if (i >= psy_vbuffer_get_nvertices(self))
        return FALSE;

    PsyVertex *vert = &g_array_index(priv->vertices, PsyVertex, i);
    vert->pos.x = x;
    vert->pos.y = y;
    vert->pos.z = z;

    return TRUE;
}

gboolean
psy_vbuffer_set_pos(PsyVBuffer* self, guint i, PsyVertexPos* pos)
{
    g_return_val_if_fail(PSY_IS_VBUFFER(self), FALSE);
    g_return_val_if_fail(pos != NULL, FALSE);
    PsyVBufferPrivate *priv = psy_vbuffer_get_instance_private(self);

    if (i >= psy_vbuffer_get_nvertices(self))
        return FALSE;

    PsyVertex *vert = &g_array_index(priv->vertices, PsyVertex, i);
    memcpy(&vert->pos, pos, sizeof(PsyVertexPos));

    return TRUE;
}

/**
 * psy_vbuffer_get_pos:
 * @self: a `PsyVBuffer` instance
 * @i: an index that should be smaller than PsyVBuffer:num_vertices
 * @pos:(out):The PsyVertexPosition is returned here.
 *
 * Obtain the vertex position at vertex i.
 *
 * Returns: True if the position is extracted, FALSE otherwise.
 */
gboolean
psy_vbuffer_get_pos(PsyVBuffer* self, guint i, PsyVertexPos* pos)
{
    g_return_val_if_fail(PSY_IS_VBUFFER(self), FALSE);
    g_return_val_if_fail(pos != NULL, FALSE);
    PsyVBufferPrivate *priv = psy_vbuffer_get_instance_private(self);

    if (i >= psy_vbuffer_get_nvertices(self))
        return FALSE;

    PsyVertex *vert = &g_array_index(priv->vertices, PsyVertex, i);
    memcpy(pos, &vert->pos, sizeof(PsyVertexPos));

    return TRUE;
}

gboolean
psy_vbuffer_set_r(PsyVBuffer* self, guint i, gfloat r)
{
    g_return_val_if_fail(PSY_IS_VBUFFER(self), FALSE);
    PsyVBufferPrivate *priv = psy_vbuffer_get_instance_private(self);

    if (i >= psy_vbuffer_get_nvertices(self))
        return FALSE;

    PsyVertex *vert = &g_array_index(priv->vertices, PsyVertex, i);
    vert->color.r = r;

    return TRUE;
}

gboolean
psy_vbuffer_set_g(PsyVBuffer* self, guint i, gfloat g)
{
    g_return_val_if_fail(PSY_IS_VBUFFER(self), FALSE);
    PsyVBufferPrivate *priv = psy_vbuffer_get_instance_private(self);

    if (i >= psy_vbuffer_get_nvertices(self))
        return FALSE;

    PsyVertex *vert = &g_array_index(priv->vertices, PsyVertex, i);
    vert->color.g = g;

    return TRUE;
}

gboolean
psy_vbuffer_set_b(PsyVBuffer* self, guint i, gfloat b)
{
    g_return_val_if_fail(PSY_IS_VBUFFER(self), FALSE);
    PsyVBufferPrivate *priv = psy_vbuffer_get_instance_private(self);

    if (i >= psy_vbuffer_get_nvertices(self))
        return FALSE;

    PsyVertex *vert = &g_array_index(priv->vertices, PsyVertex, i);
    vert->color.b = b;

    return TRUE;
}

gboolean
psy_vbuffer_set_a(PsyVBuffer* self, guint i, gfloat a)
{
    g_return_val_if_fail(PSY_IS_VBUFFER(self), FALSE);
    PsyVBufferPrivate *priv = psy_vbuffer_get_instance_private(self);

    if (i >= psy_vbuffer_get_nvertices(self))
        return FALSE;

    PsyVertex *vert = &g_array_index(priv->vertices, PsyVertex, i);
    vert->color.a = a;

    return TRUE;
}

gboolean
psy_vbuffer_set_color(PsyVBuffer* self, guint i, PsyVertexColor* color)
{
    g_return_val_if_fail(PSY_IS_VBUFFER(self), FALSE);
    PsyVBufferPrivate *priv = psy_vbuffer_get_instance_private(self);

    if (i >= psy_vbuffer_get_nvertices(self))
        return FALSE;

    PsyVertex *vert = &g_array_index(priv->vertices, PsyVertex, i);
    memcpy(&vert->color, color, sizeof(PsyVertexColor));

    return TRUE;
}

/**
 * psy_vbuffer_get_color:
 * @self: a `PsyVBuffer` instance
 * @i: an index that should be smaller than PsyVBuffer:num_vertices
 * @color:(out):The PsyVertexColor is returned here.
 *
 * Obtain the vertex color at vertex i.
 *
 * Returns: True if the color is extracted, FALSE otherwise.
 */
gboolean
psy_vbuffer_get_color(PsyVBuffer* self, guint i, PsyVertexColor* color)
{
    g_return_val_if_fail(PSY_IS_VBUFFER(self), FALSE);
    g_return_val_if_fail(color != NULL, FALSE);
    PsyVBufferPrivate *priv = psy_vbuffer_get_instance_private(self);

    if (i >= psy_vbuffer_get_nvertices(self))
        return FALSE;

    PsyVertex *vert = &g_array_index(priv->vertices, PsyVertex, i);
    memcpy(color, &vert->color, sizeof(PsyVertexColor));

    return TRUE;
}

gboolean
psy_vbuffer_set_rgba(
        PsyVBuffer* self, guint i, gfloat r, gfloat g, gfloat b, gfloat a
        )
{
    g_return_val_if_fail(PSY_IS_VBUFFER(self), FALSE);
    PsyVBufferPrivate *priv = psy_vbuffer_get_instance_private(self);

    if (i >= psy_vbuffer_get_nvertices(self))
        return FALSE;

    PsyVertex *vert = &g_array_index(priv->vertices, PsyVertex, i);
    vert->color.r = r;
    vert->color.g = g;
    vert->color.b = b;
    vert->color.a = a;

    return TRUE;
}

gboolean
psy_vbuffer_set_s(PsyVBuffer* self, guint i, gfloat s)
{
    g_return_val_if_fail(PSY_IS_VBUFFER(self), FALSE);
    PsyVBufferPrivate *priv = psy_vbuffer_get_instance_private(self);

    if (i >= psy_vbuffer_get_nvertices(self))
        return FALSE;

    PsyVertex *vert = &g_array_index(priv->vertices, PsyVertex, i);
    vert->texture_pos.s = s;

    return TRUE;
}

gboolean
psy_vbuffer_set_t(PsyVBuffer* self, guint i, gfloat t)
{
    g_return_val_if_fail(PSY_IS_VBUFFER(self), FALSE);
    PsyVBufferPrivate *priv = psy_vbuffer_get_instance_private(self);

    if (i >= psy_vbuffer_get_nvertices(self))
        return FALSE;

    PsyVertex *vert = &g_array_index(priv->vertices, PsyVertex, i);
    vert->texture_pos.t = t;

    return TRUE;
}

gboolean
psy_vbuffer_set_texture_pos(PsyVBuffer* self, guint i, PsyVertexTexPos *tpos)
{
    g_return_val_if_fail(PSY_IS_VBUFFER(self), FALSE);
    PsyVBufferPrivate *priv = psy_vbuffer_get_instance_private(self);

    if (i >= psy_vbuffer_get_nvertices(self))
        return FALSE;

    PsyVertex *vert = &g_array_index(priv->vertices, PsyVertex, i);
    memcpy(&vert->texture_pos, tpos, sizeof(PsyVertexTexPos));

    return TRUE;
}

/**
 * psy_vbuffer_get_texture_pos:
 * @self: a `PsyVBuffer` instance
 * @i: an index that should be smaller than PsyVBuffer:num_vertices
 * @tpos:(out):The PsyVertexPosition is returned here.
 *
 * Obtain the vertex texture position at vertex i.
 *
 * Returns: True if the texture position is extracted, FALSE otherwise.
 */
gboolean
psy_vbuffer_get_texture_pos(PsyVBuffer* self, guint i, PsyVertexTexPos* tpos)
{
    g_return_val_if_fail(PSY_IS_VBUFFER(self), FALSE);
    g_return_val_if_fail(tpos != NULL, FALSE);
    PsyVBufferPrivate *priv = psy_vbuffer_get_instance_private(self);

    if (i >= psy_vbuffer_get_nvertices(self))
        return FALSE;

    PsyVertex *vert = &g_array_index(priv->vertices, PsyVertex, i);
    memcpy(tpos, &vert->texture_pos, sizeof(PsyVertexColor));

    return TRUE;
}
