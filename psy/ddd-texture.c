
#include "ddd-texture.h"

#define STBI_NO_FAILURE_STRINGS
#define STBI_FAILURE_USERMSG

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

G_DEFINE_QUARK("ddd-texture-error", ddd_texture_error)

typedef enum PictureState {
    STATE_NULL,
    STATE_DECODED,
    STATE_UPLOADED
} PictureState;

typedef struct ImageDescription {
    guint width, height, num_channels;
    guint8* image_data;
} ImageDescription;

typedef struct TextureData {
    ImageDescription img_description;
    GFile*           file;
} TextureData;

static TextureData*
texture_data_new(GFile* file) {
    TextureData * data = g_slice_alloc0(sizeof(TextureData));
    data->file = g_file_dup(file);
    return data;
}

static void
texture_data_destroy(TextureData* data) {
    g_object_unref(data->file);
    g_slice_free(TextureData, data);
}

typedef struct DddTexturePrivate {
    ImageDescription    image;
    GMutex              lock;
    PictureState        state;
} DddTexturePrivate;


G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE(DddTexture, ddd_texture, G_TYPE_OBJECT)

typedef enum {
    PROP_NULL,
    PROP_PATH,
    PROP_FILE,
    PROP_NUM_CHANNELS,
    PROP_IS_DECODED,
    PROP_IS_UPLOADED,
    PROP_WIDTH,
    PROP_HEIGHT,
    NUM_PROPERTIES
} DddTextureProperty;

static GParamSpec* texture_properties[NUM_PROPERTIES];

static void
ddd_texture_set_property(GObject        *object,
                         guint           prop_id,
                         const GValue   *value,
                         GParamSpec     *pspec)
{
    DddTexture* self = DDD_TEXTURE(object);

    switch((DddTextureProperty) prop_id) {
        case PROP_FILE:
            ddd_texture_set_file(self, g_value_get_object(value));
            break;
        case PROP_PATH:
            ddd_texture_set_path(self, g_value_get_string(value));
            break;
        case PROP_NUM_CHANNELS:
            ddd_texture_set_num_channels(self, g_value_get_uint(value));
            break;
        case PROP_IS_UPLOADED:
        case PROP_IS_DECODED:
        case PROP_HEIGHT:
        case PROP_WIDTH:
        case PROP_NULL:
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

static void
ddd_texture_get_property(GObject    *object,
                         guint       prop_id,
                         GValue     *value,
                         GParamSpec *pspec)
{
    DddTexture * self = DDD_TEXTURE(object);
    DddTexturePrivate *priv = ddd_texture_get_instance_private(self);

    switch((DddTextureProperty) prop_id) {
        case PROP_NUM_CHANNELS:
            g_value_set_uint(value, priv->image.num_channels);
            break;
        case PROP_IS_DECODED:
            g_value_set_boolean(value,
                                priv->state  == STATE_DECODED);
            break;
        case PROP_IS_UPLOADED:
            g_value_set_boolean(value,
                                priv->state == STATE_UPLOADED);
            break;
        case PROP_WIDTH:
            g_value_set_uint(value,
                             priv->image.width);
            break;
        case PROP_HEIGHT:
            g_value_set_uint(value,
                             priv->image.height);
            break;
        case PROP_FILE:
        case PROP_PATH:
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

static void
ddd_texture_init(DddTexture* self)
{
    DddTexturePrivate *priv = ddd_texture_get_instance_private(self);
    g_mutex_init(&priv->lock);
}

static void
ddd_texture_dispose(GObject* object)
{
    (void) object;
}

static void
ddd_texture_finalize(GObject* object)
{
    DddTexture *self = DDD_TEXTURE(object);
    DddTexturePrivate *priv = ddd_texture_get_instance_private(self);

    if (priv->image.image_data)
        stbi_image_free(priv->image.image_data);

    g_mutex_clear(&priv->lock);
}

static void
ddd_texture_class_init(DddTextureClass* klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    object_class->set_property  = ddd_texture_set_property;
    object_class->get_property  = ddd_texture_get_property;
    object_class->finalize      = ddd_texture_finalize;
    object_class->dispose       = ddd_texture_dispose;

    /**
     * DddTexture:file
     *
     * Setting a value to the path, means a file will be opened at that path,
     * from disc and decoded.
     */
    texture_properties[PROP_PATH] =
            g_param_spec_string(
                    "path",
                    "Path",
                    "The utf8 encoded path name of an image file",
                    "",
                    G_PARAM_WRITABLE
                    );

    /**
     * DddTexture:file
     *
     * Setting a value to this file, means that this file will be loaded
     * from disc and decoded.
     */
    texture_properties[PROP_FILE] =
            g_param_spec_object(
                    "file",
                    "GIO.file",
                    "A GIO file whose path/uri etc is already specified.",
                    G_TYPE_FILE,
                    G_PARAM_WRITABLE
            );
    /**
     * DddTexture:num-channels:
     *
     * Before decoding the image:
     * 0 means let the image determine the number of output channels.
     * 1 for a grayscale image,
     * 2 for a (grayscale, alpha)
     * 3 for (r, g, b) image and
     * 4 for a (r,g,b,a) image.
     *
     * After decoding:
     * This should reflect the number of channels in the texture.
     * The meaning is identical to the above, but it should not be 0 anymore.
     *
     * Don't write to this property when the image is uploaded or decoded.
     */
    texture_properties[PROP_NUM_CHANNELS] =
            g_param_spec_uint("num-channels",
                              "Num-Channels",
                              "The number of channels of the image",
                              0,
                              4,
                              0,
                              G_PARAM_READWRITE);

    /**
     *  DddTexture:is-decoded
     *
     *  This is set to true, once the image is decoded. When you'll upload the
     *  image to a graphics card this should be cleared again.
     */
    texture_properties[PROP_IS_DECODED] =
            g_param_spec_boolean(
                    "is-decoded",
                    "Is-Decoded",
                    "If the texture is decoded from its source",
                    FALSE,
                    G_PARAM_READABLE
            );

    /**
     * DddTexture:is-uploaded
     *
     * Once the decoded image is uploaded to a graphics card this property
     * is set to true.
     */
    texture_properties[PROP_IS_UPLOADED] =
            g_param_spec_boolean(
                    "is-uploaded",
                    "Is-Uploaded",
                    "If the texture is uploaded to the graphics hardware",
                    FALSE,
                    G_PARAM_READABLE
            );

    /**
     * DddTexture:width
     *
     * The width of the picture/texture
     */
    texture_properties[PROP_WIDTH] =
            g_param_spec_uint("width",
                              "Width",
                              "The width in pixels of the texture",
                              0,
                              G_MAXINT,
                              0,
                              G_PARAM_READABLE);


    /**
     * DddTexture:height
     *
     * The height of the picture/texture
     */
    texture_properties[PROP_HEIGHT] =
            g_param_spec_uint("height",
                              "Height",
                              "The hieght in pixels of the texture",
                              0,
                              G_MAXINT,
                              0,
                              G_PARAM_READABLE);

    g_object_class_install_properties(
            object_class,
            NUM_PROPERTIES,
            texture_properties
            );
}

/* *********** helper functions ************* */

static GByteArray*
read_file(DddTexture *texture, GFile *file, GError **error)
{
    GByteArray       *bytes   = g_byte_array_sized_new(1024 * 1024 * 10);
    GFileInputStream *istream = g_file_read(file, NULL, error);
    (void) texture;
    if (*error)
        goto on_error;

    gsize picsize = 0;
    while (TRUE) {
        guint8 buffer[1024];
        gsize  nread;

        if (! g_input_stream_read_all(G_INPUT_STREAM(istream),
                                      buffer,
                                      sizeof(buffer),
                                      &nread,
                                      NULL,
                                      error)
                )
            goto on_error;

        g_byte_array_append(bytes, buffer, nread);
        picsize += nread;
        if (nread < sizeof(buffer))
            break;
    }

on_error:
    g_object_unref(istream);
    if (*error) {
        g_object_unref(bytes);
        bytes = NULL;
    }
    return bytes;
}

static void
decode_picture (DddTexture *self,
                GByteArray *bytes,
                GFile      *file,
                GError    **error)
{
    int width, height, num_channels;
    DddTexturePrivate *priv = ddd_texture_get_instance_private(self);

    guint8* data = stbi_load_from_memory(
            bytes->data,
            (int) bytes->len,
            &width,
            &height,
            &num_channels,
            (int) priv->image.num_channels
    );

    if (!data) {
        const char* msg = stbi_failure_reason();
        g_set_error(error, DDD_TEXTURE_ERROR, DDD_TEXTURE_ERROR_DECODE,
                    "Unable to load '%s': %s",
                    g_file_peek_path(file),
                    msg
        );
    }
    else {
        if(priv->image.num_channels == 0)
            priv->image.num_channels = num_channels;
        priv->image.width = width;
        priv->image.height = height;
        priv->image.image_data = data;
    }
}

static void
load_and_decode(DddTexture* self, GFile* file, GError** error)
{
    GByteArray *bytes = read_file(self, file, error);
    if (*error) {
        return;
    }
    decode_picture(self, bytes, file, error);
}

static void
load_picture(
        GTask          *task,
        gpointer        source_object,
        gpointer        task_data,
        GCancellable   *cancellable)
{
    (void) cancellable;
    GError             *error   = NULL;
    TextureData        *tdata   = task_data;
    DddTexture         *self    = source_object;
    (void) self; // we currently do not use self.
    // 10 megabytes should be enough to store most image files
    GByteArray         *bytes   = g_byte_array_sized_new(1024 * 1024 * 10);

    GFileInputStream *istream = g_file_read(tdata->file, NULL, &error);
    if (error)
        goto on_error;

    gsize picsize = 0;
    while (error == NULL) {
        guint8 buffer[1024];
        gsize nread;

        if (! g_input_stream_read_all(G_INPUT_STREAM(istream),
                                      buffer,
                                      sizeof(buffer),
                                      &nread,
                                      NULL,
                                      &error)
                )
            goto on_error;

        g_byte_array_append(bytes, buffer, nread);
        picsize += nread;
        if (nread < sizeof(buffer))
            break;

    }

    int width, height, num_channels;

    tdata->img_description.image_data = stbi_load_from_memory(
            bytes->data,
            (int)bytes->len,
            &width,
            &height,
            &num_channels,
            (int) tdata->img_description.num_channels
            );

    if (!tdata->img_description.image_data) {
        const char* msg = stbi_failure_reason();
        g_set_error(&error, DDD_TEXTURE_ERROR, DDD_TEXTURE_ERROR_DECODE,
                    "Unable to load '%s': %s",
                    g_file_peek_path(tdata->file),
                    msg
                    );
    }

on_error:

    g_object_unref(istream);
    g_byte_array_unref(bytes);

    if (error)
        g_task_return_error(task, error);
    else
        g_task_return_pointer(task,
                              tdata,
                              (GDestroyNotify) texture_data_destroy);
}


/* *********** public texture functions ************* */

void
ddd_texture_upload(DddTexture *self, GError **error)
{
    g_return_if_fail(DDD_IS_TEXTURE(self));
    DddTextureClass *klass = DDD_TEXTURE_GET_CLASS(self);

    g_return_if_fail(klass->upload);
    klass->upload(self, error);
}

gboolean
ddd_texture_is_uploaded(DddTexture *self)
{
    g_return_val_if_fail(DDD_IS_TEXTURE(self), FALSE);
    DddTextureClass *klass = DDD_TEXTURE_GET_CLASS(self);

    g_return_val_if_fail(klass->is_uploaded, FALSE);
    return klass->is_uploaded(self);
}

void
ddd_texture_bind(DddTexture *self, GError **error)
{
    g_return_if_fail(DDD_IS_TEXTURE(self));
    g_return_if_fail(error || *error);
    DddTextureClass *klass = DDD_TEXTURE_GET_CLASS(self);

    g_return_if_fail(klass->bind);
    klass->bind(self, error);
}

void
ddd_texture_set_file(DddTexture *self, GFile* file)
{
    g_return_if_fail(DDD_IS_TEXTURE(self));
    g_return_if_fail(G_IS_FILE(file));

    GError *error = NULL;

    load_and_decode(self, file, &error);

    if (error) {
        g_warning("%s", error->message);
        g_error_free(error);
        return;
    }
}

void
ddd_texture_set_file_async(DddTexture          *self,
                           GFile               *file,
                           GCancellable        *cancellable,
                           GAsyncReadyCallback  callback,
                           gpointer             data)
{
    g_return_if_fail(DDD_IS_TEXTURE(self));
    g_return_if_fail(G_IS_FILE(self));

    DddTexturePrivate *priv = ddd_texture_get_instance_private(self);

    TextureData *tdata = texture_data_new(file);
    tdata->img_description.num_channels = priv->image.num_channels;

    GTask *task = g_task_new(
            self,
            cancellable,
            callback,
            data);

    g_task_set_task_data(task,
                         tdata,
                         (GDestroyNotify)texture_data_destroy);
    g_task_run_in_thread(task, load_picture);
    g_object_unref(task);
}

void
ddd_texture_set_path(DddTexture* self, const gchar* path)
{
    g_return_if_fail(DDD_IS_TEXTURE(self));
    g_return_if_fail(path);

    GFile* file = g_file_new_for_path(path);
    ddd_texture_set_file(self, file);
    g_object_unref(file);
}

void
ddd_texture_set_path_async(DddTexture          *self,
                           const gchar         *path,
                           GCancellable        *cancellable,
                           GAsyncReadyCallback  callback,
                           gpointer             data
                           )
{
    g_return_if_fail(DDD_IS_TEXTURE(self));
    g_return_if_fail(path);

    GFile *file = g_file_new_for_path(path);
    ddd_texture_set_file_async(self, file, cancellable, callback, data);
    g_object_unref(file);
}

guint8*
ddd_texture_get_data(DddTexture* self)
{
    g_return_val_if_fail(DDD_IS_TEXTURE(self), NULL);
    DddTexturePrivate *priv = ddd_texture_get_instance_private(self);

    return priv->image.image_data;
}

void
ddd_texture_set_num_channels(DddTexture* self, guint nchannels)
{
    g_return_if_fail(DDD_IS_TEXTURE(self));
    g_return_if_fail(nchannels <= 4);

    DddTexturePrivate *priv = ddd_texture_get_instance_private(self);
    priv->image.num_channels = nchannels;
}

guint
ddd_texture_get_num_channels(DddTexture *self)
{
    g_return_val_if_fail(DDD_IS_TEXTURE(self), 0);

    DddTexturePrivate *priv = ddd_texture_get_instance_private(self);
    return priv->image.num_channels;
}

guint
ddd_texture_get_width(DddTexture *self)
{
    g_return_val_if_fail(DDD_IS_TEXTURE(self), 0);

    DddTexturePrivate *priv = ddd_texture_get_instance_private(self);
    return priv->image.width;
}

guint
ddd_texture_get_height(DddTexture* self) {
    g_return_val_if_fail(DDD_IS_TEXTURE(self), 0);

    DddTexturePrivate *priv = ddd_texture_get_instance_private(self);
    return priv->image.height;
}
