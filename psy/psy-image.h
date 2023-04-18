#pragma once

#include <glib-object.h>

#include "psy-color.h"

G_BEGIN_DECLS

#define PSY_TYPE_IMAGE psy_image_get_type()
G_DECLARE_FINAL_TYPE(PsyImage, psy_image, PSY, IMAGE, GObject)

G_MODULE_EXPORT PsyImage *
psy_image_new(guint width, guint height, guint num_channels);

G_MODULE_EXPORT void
psy_image_set_width(PsyImage *self, guint width);

G_MODULE_EXPORT guint
psy_image_get_width(PsyImage *self);

G_MODULE_EXPORT void
psy_image_set_height(PsyImage *self, guint height);

G_MODULE_EXPORT guint
psy_image_get_height(PsyImage *self);

G_MODULE_EXPORT void
psy_image_set_num_channels(PsyImage *self, guint num_channels);

G_MODULE_EXPORT guint
psy_image_get_num_channels(PsyImage *self);

G_MODULE_EXPORT guint64
psy_image_get_num_bytes(PsyImage *self);

G_MODULE_EXPORT guint
psy_image_get_stride(PsyImage *self);

G_MODULE_EXPORT GBytes *
psy_image_get_bytes(PsyImage *self);

G_MODULE_EXPORT void
psy_image_clear(PsyImage *self, PsyColor *color);

G_MODULE_EXPORT void
psy_image_set_pixel(PsyImage *self, guint row, guint column, PsyColor *color);

G_MODULE_EXPORT PsyColor *
psy_image_get_pixel(PsyImage *self, guint row, guint column);

G_MODULE_EXPORT gboolean
psy_image_save(PsyImage *self, GFile *file, const gchar *type, GError **error);

G_MODULE_EXPORT gboolean
psy_image_save_path(PsyImage    *self,
                    const gchar *path,
                    const gchar *type,
                    GError     **error);

guint8 *
psy_image_get_ptr(PsyImage *self);

G_END_DECLS
