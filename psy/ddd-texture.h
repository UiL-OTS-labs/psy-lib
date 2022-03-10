
#ifndef DDD_TEXTURE_H
#define DDD_TEXTURE_H

#include <glib-object.h>
#include <gio/gio.h>

G_BEGIN_DECLS

#define DDD_TEXTURE_ERROR ddd_texture_error_quark()
GQuark ddd_texture_error_quark();

typedef enum _DddTextureError {
    DDD_TEXTURE_ERROR_DECODE, // failed to decode the texture.
    DDD_TEXTURE_ERROR_FAILED
} DddTextureError;

#define DDD_TYPE_TEXTURE ddd_texture_get_type()
G_DECLARE_DERIVABLE_TYPE(DddTexture, ddd_texture, DDD, TEXTURE, GObject)

typedef struct _DddTextureClass {
    GObjectClass parent_class;

    void        (*upload)       (DddTexture* texture, GError **error);
    gboolean    (*is_uploaded)  (DddTexture* texture);
    void        (*bind)         (DddTexture* texture, GError **error);
} DddTextureClass;

G_MODULE_EXPORT void
ddd_texture_set_path(DddTexture* texture, const char* path);

G_MODULE_EXPORT void
ddd_texture_set_file(DddTexture* texture, GFile* file);

G_MODULE_EXPORT void
ddd_texture_set_file_async(DddTexture          *texture,
                           GFile               *file,
                           GCancellable        *cancellable,
                           GAsyncReadyCallback  callback,
                           gpointer             data);

G_MODULE_EXPORT void
ddd_texture_set_path_async(DddTexture          *texture,
                           const gchar         *path,
                           GCancellable        *cancellable,
                           GAsyncReadyCallback  callback,
                           gpointer             data);

G_MODULE_EXPORT guint
ddd_texture_get_num_channels(DddTexture* texture);

G_MODULE_EXPORT void
ddd_texture_set_num_channels(DddTexture* texture, guint num_channels);

G_MODULE_EXPORT guint
ddd_texture_get_width(DddTexture* texture);

G_MODULE_EXPORT guint
ddd_texture_get_height(DddTexture* texture);

G_MODULE_EXPORT void
ddd_texture_upload(DddTexture* texture, GError **error);

G_MODULE_EXPORT gboolean
ddd_texture_is_uploaded(DddTexture* texture);

G_MODULE_EXPORT void
ddd_texture_bind(DddTexture* texture, GError **error);

G_MODULE_EXPORT guint8*
ddd_texture_get_data();

G_END_DECLS

#endif