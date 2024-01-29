
#pragma once

#include "psy-enums.h"
#include "psy-rectangle.h"

G_BEGIN_DECLS

#define PSY_TYPE_PICTURE psy_picture_get_type()
G_DECLARE_DERIVABLE_TYPE(PsyPicture, psy_picture, PSY, PICTURE, PsyRectangle)

typedef struct _PsyPictureClass {
    PsyRectangleClass parent;
    void (*auto_resize)(PsyPicture *self, gfloat width, gfloat height);
} PsyPictureClass;

G_MODULE_EXPORT PsyPicture *
psy_picture_new(PsyCanvas *canvas);

G_MODULE_EXPORT PsyPicture *
psy_picture_new_filename(PsyCanvas *canvas, const gchar *filename);

G_MODULE_EXPORT PsyPicture *
psy_picture_new_xy_filename(PsyCanvas   *canvas,
                            gfloat       x,
                            gfloat       y,
                            const gchar *filename);

G_MODULE_EXPORT PsyPicture *
psy_picture_new_full(PsyCanvas   *canvas,
                     gfloat       x,
                     gfloat       y,
                     gfloat       width,
                     gfloat       height,
                     const gchar *filename);

G_MODULE_EXPORT void
psy_picture_free(PsyPicture *self);

G_MODULE_EXPORT void
psy_picture_set_filename(PsyPicture *self, const gchar *filename);

G_MODULE_EXPORT const gchar *
psy_picture_get_filename(PsyPicture *self);

G_MODULE_EXPORT PsyPictureSizeStrategy
psy_picture_get_size_strategy(PsyPicture *self);

G_MODULE_EXPORT PsyPictureSizeStrategy
psy_picture_set_size_strategy(PsyPicture *self);

G_MODULE_EXPORT void
psy_picture_set_strategy(PsyPicture *self, PsyPictureSizeStrategy strategy);

G_END_DECLS
