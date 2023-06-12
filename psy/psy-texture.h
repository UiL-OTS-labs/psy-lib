
#ifndef PSY_TEXTURE_H
#define PSY_TEXTURE_H

#include <gio/gio.h>
#include <glib-object.h>

#include "psy-image.h"

G_BEGIN_DECLS

#define PSY_TEXTURE_ERROR psy_texture_error_quark()
GQuark
psy_texture_error_quark(void);

#define PSY_TYPE_TEXTURE psy_texture_get_type()
G_DECLARE_DERIVABLE_TYPE(PsyTexture, psy_texture, PSY, TEXTURE, GObject)

typedef struct _PsyTextureClass {
    GObjectClass parent_class;

    void (*upload)(PsyTexture *self, GError **error);
    gboolean (*is_uploaded)(PsyTexture *self);
    void (*upload_image)(PsyTexture *self, PsyImage *image, GError **error);
    void (*bind)(PsyTexture *self, GError **error);
} PsyTextureClass;

G_MODULE_EXPORT void
psy_texture_set_path(PsyTexture *self, const char *path);

G_MODULE_EXPORT void
psy_texture_set_file(PsyTexture *self, GFile *file);

G_MODULE_EXPORT void
psy_texture_set_file_async(PsyTexture         *self,
                           GFile              *file,
                           GCancellable       *cancellable,
                           GAsyncReadyCallback callback,
                           gpointer            data);

G_MODULE_EXPORT void
psy_texture_set_path_async(PsyTexture         *self,
                           const gchar        *path,
                           GCancellable       *cancellable,
                           GAsyncReadyCallback callback,
                           gpointer            data);

G_MODULE_EXPORT guint
psy_texture_get_num_channels(PsyTexture *self);

G_MODULE_EXPORT void
psy_texture_set_num_channels(PsyTexture *self, guint num_channels);

G_MODULE_EXPORT guint
psy_texture_get_width(PsyTexture *self);

G_MODULE_EXPORT guint
psy_texture_get_height(PsyTexture *self);

G_MODULE_EXPORT void
psy_texture_upload(PsyTexture *self, GError **error);

G_MODULE_EXPORT gboolean
psy_texture_is_uploaded(PsyTexture *self);

G_MODULE_EXPORT void
psy_texture_bind(PsyTexture *self, GError **error);

G_MODULE_EXPORT guint8 *
psy_texture_get_data(PsyTexture *self);

G_MODULE_EXPORT const gchar *
psy_texture_get_filename(PsyTexture *self);

G_MODULE_EXPORT void
psy_texture_upload_image(PsyTexture *self, PsyImage *image, GError **error);

G_END_DECLS

#endif
