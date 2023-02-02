
#ifndef PSY_TEXTURE_H
#define PSY_TEXTURE_H

#include <gio/gio.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define PSY_TEXTURE_ERROR psy_texture_error_quark()
GQuark
psy_texture_error_quark();

#define PSY_TYPE_TEXTURE psy_texture_get_type()
G_DECLARE_DERIVABLE_TYPE(PsyTexture, psy_texture, PSY, TEXTURE, GObject)

typedef struct _PsyTextureClass {
    GObjectClass parent_class;

    void (*upload)(PsyTexture *texture, GError **error);
    gboolean (*is_uploaded)(PsyTexture *texture);
    void (*bind)(PsyTexture *texture, GError **error);
} PsyTextureClass;

G_MODULE_EXPORT void
psy_texture_set_path(PsyTexture *texture, const char *path);

G_MODULE_EXPORT void
psy_texture_set_file(PsyTexture *texture, GFile *file);

G_MODULE_EXPORT void
psy_texture_set_file_async(PsyTexture         *texture,
                           GFile              *file,
                           GCancellable       *cancellable,
                           GAsyncReadyCallback callback,
                           gpointer            data);

G_MODULE_EXPORT void
psy_texture_set_path_async(PsyTexture         *texture,
                           const gchar        *path,
                           GCancellable       *cancellable,
                           GAsyncReadyCallback callback,
                           gpointer            data);

G_MODULE_EXPORT guint
psy_texture_get_num_channels(PsyTexture *texture);

G_MODULE_EXPORT void
psy_texture_set_num_channels(PsyTexture *texture, guint num_channels);

G_MODULE_EXPORT guint
psy_texture_get_width(PsyTexture *texture);

G_MODULE_EXPORT guint
psy_texture_get_height(PsyTexture *texture);

G_MODULE_EXPORT void
psy_texture_upload(PsyTexture *texture, GError **error);

G_MODULE_EXPORT gboolean
psy_texture_is_uploaded(PsyTexture *texture);

G_MODULE_EXPORT void
psy_texture_bind(PsyTexture *texture, GError **error);

G_MODULE_EXPORT guint8 *
psy_texture_get_data();

G_END_DECLS

#endif
