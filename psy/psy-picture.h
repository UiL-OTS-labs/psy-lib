
#pragma once

#include "psy-enums.h"
#include "psy-rectangle.h"

G_BEGIN_DECLS

#define PSY_TYPE_PICTURE psy_picture_get_type()
G_DECLARE_DERIVABLE_TYPE(PsyPicture, psy_picture, PSY, PICTURE, PsyRectangle)

typedef struct _PsyPictureClass {
    PsyRectangleClass parent;
} PsyPictureClass;

G_MODULE_EXPORT PsyPicture *
psy_picture_new(PsyWindow *window);

G_MODULE_EXPORT PsyPicture *
psy_picture_new_filename(PsyWindow *window, const gchar *filename);

G_MODULE_EXPORT PsyPicture *
psy_picture_new_xy_filename(PsyWindow   *window,
                            gfloat       x,
                            gfloat       y,
                            const gchar *filename);

G_MODULE_EXPORT PsyPicture *
psy_picture_new_full(PsyWindow   *window,
                     gfloat       x,
                     gfloat       y,
                     gfloat       width,
                     gfloat       height,
                     const gchar *filename);

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
