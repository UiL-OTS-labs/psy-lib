
#include "ddd-vbuffer.h"

typedef struct DddShaderPrivate {
    GArray *vertices;
} DddVBufferPrivate;

G_DEFINE_TYPE_WITH_PRIVATE(DddVBuffer, ddd_vbuffer, G_TYPE_OBJECT)

typedef enum {
    PROP_NULL,
    PROP_NVERTS,
    PROP_MEMSIZE,
    PROP_UPLOADED,
    NUM_PROPERTIES
} DddVBufferProperty;

static GParamSpec* vbuffer_properties[NUM_PROPERTIES] = {0};

static void
ddd_vbuffer_set_property(GObject       *object,
                         guint          prop_id,
                         const GValue  *value,
                         GParamSpec    *pspec)
{
    DddVBuffer *self = DDD_VBUFFER(object);

    switch((DddVBufferProperty) prop_id) {
        case PROP_NVERTS:
            ddd_vbuffer_set_nvertices(self, g_value_get_uint(value));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

static void
ddd_vbuffer_get_property(GObject       *object,
                         guint          prop_id,
                         GValue        *value,
                         GParamSpec    *pspec)
{
    DddVBuffer *self = DDD_VBUFFER(object);

    switch((DddVBufferProperty) prop_id) {
        case PROP_NVERTS:
            g_value_set_uint(value, ddd_vbuffer_get_nvertices(self));
            break;
        case PROP_MEMSIZE:
            g_value_set_uint64(value, ddd_vbuffer_get_size(self));
            break;
        case PROP_UPLOADED:
            g_value_set_boolean(value, ddd_vbuffer_is_uploaded(self));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

static void
ddd_vbuffer_init(DddVBuffer* self)
{
    DddVBufferPrivate * priv = ddd_vbuffer_get_instance_private(self);
    priv->vertices = g_array_new(FALSE, TRUE, sizeof(DddVertex));
}

static void
ddd_vbuffer_dispose(GObject* object)
{
    DddVBuffer *self = DDD_VBUFFER(object);
    DddVBufferPrivate *priv = ddd_vbuffer_get_instance_private(self);

    if (priv->vertices) {
        g_array_unref(priv->vertices);
        priv->vertices = NULL;
    }
    G_OBJECT_CLASS(ddd_vbuffer_parent_class)->dispose(object);
}

static void
ddd_vbuffer_finalize(GObject* object)
{
    G_OBJECT_CLASS(ddd_vbuffer_parent_class)->finalize(object);
}

static void
ddd_vbuffer_class_init(DddVBufferClass* klass)
{
    GObjectClass* gobject_class = G_OBJECT_CLASS(klass);

    gobject_class->set_property = ddd_vbuffer_set_property;
    gobject_class->get_property = ddd_vbuffer_get_property;
    gobject_class->dispose      = ddd_vbuffer_dispose;
    gobject_class->finalize     = ddd_vbuffer_finalize;

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
ddd_vbuffer_upload(DddVBuffer* self, GError **error)
{
    g_return_if_fail(DDD_IS_VBUFFER(self));
    g_return_if_fail(error != NULL && *error == NULL);

    DddVBufferClass *klass = DDD_VBUFFER_GET_CLASS(self);

    g_return_if_fail(klass->upload);
    klass->upload(self, error);
}

gboolean
ddd_vbuffer_is_uploaded(DddVBuffer* self)
{
    g_return_val_if_fail(DDD_IS_VBUFFER(self), FALSE);

    DddVBufferClass *klass = DDD_VBUFFER_GET_CLASS(self);

    g_return_val_if_fail(klass->is_uploaded, FALSE);
    return klass->is_uploaded(self);
}

void
ddd_vbuffer_draw_triangles(DddVBuffer *self, GError **error)
{
    g_return_if_fail(DDD_IS_VBUFFER(self));
    g_return_if_fail(error == NULL || *error == NULL);

    DddVBufferClass* klass = DDD_VBUFFER_GET_CLASS(self);

    g_return_if_fail(klass->draw_triangles);

    klass->draw_triangles(self, error);
}

void
ddd_vbuffer_draw_triangle_strip(DddVBuffer *self, GError **error)
{
    g_return_if_fail(DDD_IS_VBUFFER(self));
    g_return_if_fail(error == NULL || *error == NULL);

    DddVBufferClass* klass = DDD_VBUFFER_GET_CLASS(self);

    g_return_if_fail(klass->draw_triangle_strip);

    klass->draw_triangle_strip(self, error);
}

void
ddd_vbuffer_draw_triangle_fan(DddVBuffer *self, GError **error)
{
    g_return_if_fail(DDD_IS_VBUFFER(self));
    g_return_if_fail(error == NULL || *error == NULL);

    DddVBufferClass* klass = DDD_VBUFFER_GET_CLASS(self);

    g_return_if_fail(klass->draw_triangle_fan);

    klass->draw_triangle_fan(self, error);
}

void
ddd_vbuffer_set_nvertices(DddVBuffer* self, guint nverts)
{
    g_return_if_fail(DDD_IS_VBUFFER(self));
    DddVBufferPrivate *priv = ddd_vbuffer_get_instance_private(self);

    g_array_set_size(priv->vertices, nverts);
}

guint
ddd_vbuffer_get_nvertices(DddVBuffer* self)
{
    g_return_val_if_fail(DDD_IS_VBUFFER(self), (guint) 0);
    DddVBufferPrivate *priv = ddd_vbuffer_get_instance_private(self);

    return priv->vertices->len;
}

gsize
ddd_vbuffer_get_size (DddVBuffer* self)
{
    g_return_val_if_fail(DDD_IS_VBUFFER(self), 0);
    DddVBufferPrivate *priv = ddd_vbuffer_get_instance_private(self);

    return priv->vertices->len * sizeof(DddVertex);
}

const guint8*
ddd_vbuffer_get_buffer(DddVBuffer* self)
{
    g_return_val_if_fail(DDD_IS_VBUFFER(self), NULL);
    DddVBufferPrivate *priv = ddd_vbuffer_get_instance_private(self);

    return (guint8*) priv->vertices->data;
}

gboolean
ddd_vbuffer_set_from_data(DddVBuffer* self, const void* data, guint nverts)
{
    g_return_val_if_fail(DDD_IS_VBUFFER(self), FALSE);
    DddVBufferPrivate *priv = ddd_vbuffer_get_instance_private(self);

    g_array_set_size(priv->vertices, nverts);
    if (ddd_vbuffer_get_nvertices(self) != nverts)
        return FALSE;
    memcpy(priv->vertices->data, data, ddd_vbuffer_get_size(self));
    return TRUE;
}

gboolean
ddd_vbuffer_set_x(DddVBuffer* self, guint i, gfloat x)
{
    g_return_val_if_fail(DDD_IS_VBUFFER(self), FALSE);
    DddVBufferPrivate *priv = ddd_vbuffer_get_instance_private(self);

    if (i >= ddd_vbuffer_get_nvertices(self))
        return FALSE;

    DddVertex *vert = &g_array_index(priv->vertices, DddVertex, i);
    vert->pos.x = x;

    return TRUE;
}

gboolean
ddd_vbuffer_set_y(DddVBuffer* self, guint i, gfloat y)
{
    g_return_val_if_fail(DDD_IS_VBUFFER(self), FALSE);
    DddVBufferPrivate *priv = ddd_vbuffer_get_instance_private(self);

    if (i >= ddd_vbuffer_get_nvertices(self))
        return FALSE;

    DddVertex *vert = &g_array_index(priv->vertices, DddVertex, i);
    vert->pos.y = y;

    return TRUE;
}

gboolean
ddd_vbuffer_set_z(DddVBuffer* self, guint i, gfloat z)
{
    g_return_val_if_fail(DDD_IS_VBUFFER(self), FALSE);
    DddVBufferPrivate *priv = ddd_vbuffer_get_instance_private(self);

    if (i >= ddd_vbuffer_get_nvertices(self))
        return FALSE;

    DddVertex *vert = &g_array_index(priv->vertices, DddVertex, i);
    vert->pos.z = z;

    return TRUE;
}

gboolean
ddd_vbuffer_set_xyz(DddVBuffer* self, guint i, gfloat x, gfloat y, gfloat z)
{
    g_return_val_if_fail(DDD_IS_VBUFFER(self), FALSE);
    DddVBufferPrivate *priv = ddd_vbuffer_get_instance_private(self);

    if (i >= ddd_vbuffer_get_nvertices(self))
        return FALSE;

    DddVertex *vert = &g_array_index(priv->vertices, DddVertex, i);
    vert->pos.x = x;
    vert->pos.y = y;
    vert->pos.z = z;

    return TRUE;
}

gboolean
ddd_vbuffer_set_pos(DddVBuffer* self, guint i, DddVertexPos* pos)
{
    g_return_val_if_fail(DDD_IS_VBUFFER(self), FALSE);
    g_return_val_if_fail(pos != NULL, FALSE);
    DddVBufferPrivate *priv = ddd_vbuffer_get_instance_private(self);

    if (i >= ddd_vbuffer_get_nvertices(self))
        return FALSE;

    DddVertex *vert = &g_array_index(priv->vertices, DddVertex, i);
    memcpy(&vert->pos, pos, sizeof(DddVertexPos));

    return TRUE;
}

/**
 * ddd_vbuffer_get_pos:
 * @self: a `DddVBuffer` instance
 * @i: an index that should be smaller than DddVBuffer:num_vertices
 * @pos:(out):The DddVertexPosition is returned here.
 *
 * Obtain the vertex position at vertex i.
 *
 * Returns: True if the position is extracted, FALSE otherwise.
 */
gboolean
ddd_vbuffer_get_pos(DddVBuffer* self, guint i, DddVertexPos* pos)
{
    g_return_val_if_fail(DDD_IS_VBUFFER(self), FALSE);
    g_return_val_if_fail(pos != NULL, FALSE);
    DddVBufferPrivate *priv = ddd_vbuffer_get_instance_private(self);

    if (i >= ddd_vbuffer_get_nvertices(self))
        return FALSE;

    DddVertex *vert = &g_array_index(priv->vertices, DddVertex, i);
    memcpy(pos, &vert->pos, sizeof(DddVertexPos));

    return TRUE;
}

gboolean
ddd_vbuffer_set_r(DddVBuffer* self, guint i, gfloat r)
{
    g_return_val_if_fail(DDD_IS_VBUFFER(self), FALSE);
    DddVBufferPrivate *priv = ddd_vbuffer_get_instance_private(self);

    if (i >= ddd_vbuffer_get_nvertices(self))
        return FALSE;

    DddVertex *vert = &g_array_index(priv->vertices, DddVertex, i);
    vert->color.r = r;

    return TRUE;
}

gboolean
ddd_vbuffer_set_g(DddVBuffer* self, guint i, gfloat g)
{
    g_return_val_if_fail(DDD_IS_VBUFFER(self), FALSE);
    DddVBufferPrivate *priv = ddd_vbuffer_get_instance_private(self);

    if (i >= ddd_vbuffer_get_nvertices(self))
        return FALSE;

    DddVertex *vert = &g_array_index(priv->vertices, DddVertex, i);
    vert->color.g = g;

    return TRUE;
}

gboolean
ddd_vbuffer_set_b(DddVBuffer* self, guint i, gfloat b)
{
    g_return_val_if_fail(DDD_IS_VBUFFER(self), FALSE);
    DddVBufferPrivate *priv = ddd_vbuffer_get_instance_private(self);

    if (i >= ddd_vbuffer_get_nvertices(self))
        return FALSE;

    DddVertex *vert = &g_array_index(priv->vertices, DddVertex, i);
    vert->color.b = b;

    return TRUE;
}

gboolean
ddd_vbuffer_set_a(DddVBuffer* self, guint i, gfloat a)
{
    g_return_val_if_fail(DDD_IS_VBUFFER(self), FALSE);
    DddVBufferPrivate *priv = ddd_vbuffer_get_instance_private(self);

    if (i >= ddd_vbuffer_get_nvertices(self))
        return FALSE;

    DddVertex *vert = &g_array_index(priv->vertices, DddVertex, i);
    vert->color.a = a;

    return TRUE;
}

gboolean
ddd_vbuffer_set_color(DddVBuffer* self, guint i, DddVertexColor* color)
{
    g_return_val_if_fail(DDD_IS_VBUFFER(self), FALSE);
    DddVBufferPrivate *priv = ddd_vbuffer_get_instance_private(self);

    if (i >= ddd_vbuffer_get_nvertices(self))
        return FALSE;

    DddVertex *vert = &g_array_index(priv->vertices, DddVertex, i);
    memcpy(&vert->color, color, sizeof(DddVertexColor));

    return TRUE;
}

/**
 * ddd_vbuffer_get_color:
 * @self: a `DddVBuffer` instance
 * @i: an index that should be smaller than DddVBuffer:num_vertices
 * @color:(out):The DddVertexColor is returned here.
 *
 * Obtain the vertex color at vertex i.
 *
 * Returns: True if the color is extracted, FALSE otherwise.
 */
gboolean
ddd_vbuffer_get_color(DddVBuffer* self, guint i, DddVertexColor* color)
{
    g_return_val_if_fail(DDD_IS_VBUFFER(self), FALSE);
    g_return_val_if_fail(color != NULL, FALSE);
    DddVBufferPrivate *priv = ddd_vbuffer_get_instance_private(self);

    if (i >= ddd_vbuffer_get_nvertices(self))
        return FALSE;

    DddVertex *vert = &g_array_index(priv->vertices, DddVertex, i);
    memcpy(color, &vert->color, sizeof(DddVertexColor));

    return TRUE;
}

gboolean
ddd_vbuffer_set_rgba(
        DddVBuffer* self, guint i, gfloat r, gfloat g, gfloat b, gfloat a
        )
{
    g_return_val_if_fail(DDD_IS_VBUFFER(self), FALSE);
    DddVBufferPrivate *priv = ddd_vbuffer_get_instance_private(self);

    if (i >= ddd_vbuffer_get_nvertices(self))
        return FALSE;

    DddVertex *vert = &g_array_index(priv->vertices, DddVertex, i);
    vert->color.r = r;
    vert->color.g = g;
    vert->color.b = b;
    vert->color.a = a;

    return TRUE;
}

gboolean
ddd_vbuffer_set_s(DddVBuffer* self, guint i, gfloat s)
{
    g_return_val_if_fail(DDD_IS_VBUFFER(self), FALSE);
    DddVBufferPrivate *priv = ddd_vbuffer_get_instance_private(self);

    if (i >= ddd_vbuffer_get_nvertices(self))
        return FALSE;

    DddVertex *vert = &g_array_index(priv->vertices, DddVertex, i);
    vert->texture_pos.s = s;

    return TRUE;
}

gboolean
ddd_vbuffer_set_t(DddVBuffer* self, guint i, gfloat t)
{
    g_return_val_if_fail(DDD_IS_VBUFFER(self), FALSE);
    DddVBufferPrivate *priv = ddd_vbuffer_get_instance_private(self);

    if (i >= ddd_vbuffer_get_nvertices(self))
        return FALSE;

    DddVertex *vert = &g_array_index(priv->vertices, DddVertex, i);
    vert->texture_pos.t = t;

    return TRUE;
}

gboolean
ddd_vbuffer_set_texture_pos(DddVBuffer* self, guint i, DddVertexTexPos *tpos)
{
    g_return_val_if_fail(DDD_IS_VBUFFER(self), FALSE);
    DddVBufferPrivate *priv = ddd_vbuffer_get_instance_private(self);

    if (i >= ddd_vbuffer_get_nvertices(self))
        return FALSE;

    DddVertex *vert = &g_array_index(priv->vertices, DddVertex, i);
    memcpy(&vert->texture_pos, tpos, sizeof(DddVertexTexPos));

    return TRUE;
}

/**
 * ddd_vbuffer_get_texture_pos:
 * @self: a `DddVBuffer` instance
 * @i: an index that should be smaller than DddVBuffer:num_vertices
 * @tpos:(out):The DddVertexPosition is returned here.
 *
 * Obtain the vertex texture position at vertex i.
 *
 * Returns: True if the texture position is extracted, FALSE otherwise.
 */
gboolean
ddd_vbuffer_get_texture_pos(DddVBuffer* self, guint i, DddVertexTexPos* tpos)
{
    g_return_val_if_fail(DDD_IS_VBUFFER(self), FALSE);
    g_return_val_if_fail(tpos != NULL, FALSE);
    DddVBufferPrivate *priv = ddd_vbuffer_get_instance_private(self);

    if (i >= ddd_vbuffer_get_nvertices(self))
        return FALSE;

    DddVertex *vert = &g_array_index(priv->vertices, DddVertex, i);
    memcpy(tpos, &vert->texture_pos, sizeof(DddVertexColor));

    return TRUE;
}