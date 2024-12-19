#pragma once

#include <glib-object.h>

#include <gio/gio.h>

#include "psy-duration.h"

G_BEGIN_DECLS

/**
 * PsySize:
 * @width: Should be a positive integer
 * @height: Should be a positive integer
 *
 * PsySize represents the size of an object in
 * terms of width and height. Typically
 * width and height should be positive.
 *
 * PsySize's should be freed with [func@Size.free]
 */
typedef struct _PsySize {
    gint width;
    gint height;
} PsySize;

/**
 * PsyPos:
 * @x: the x position
 * @y: the y position
 *
 * A position in 2D space
 *
 * PsyPos's should be freed with [func@Pos.free]
 */
typedef struct _PsyPos {
    gint x;
    gint y;
} PsyPos;

/**
 * PsyRect:
 * @pos: The position of the rectangle,
 *       this will typically be the coordinate of
 *       the upper left corner.
 * @Size: The width and height of the rectangle.
 *
 * This class represents a rectangle in 2D space.
 * The bottom and right most coordinate can
 * be obtained by pos->y + size->height and
 * pos->y + size->width;
 *
 * PsyRects's should be freed with [func@Rect.free]
 */
typedef struct _PsyRect {
    PsyPos  *pos;
    PsySize *size;
} PsyRect;

/**
 * PsyDisplayInfo:
 * @name: the name of the display
 * @rect: the rectangle that tells the position of the window
 *        relative to the other windows. Hence, expect
 *        rect->pos->x/y not to be at the origin.
 * @size_mm: The size of the display, this will be set to a
 *           width=-1, and height=-1 if this isn't available.
 *
 * This class represents some general information of a monitor.
 * It also tries to obtain the physical size of the monitor.
 * You, should think about what that means, the size of a beamer
 * is very dependent to the distance to the projection screen.
 * Psylib might also not be always able to determine the
 * size as it isn't always easy to obtain the information
 * of the EDID of monitors.
 * The physical size of the monitor will be (-1, -1) when it
 * is unknown.
 *
 * PsyDisplayInfos's should be freed with [func@DisplayInfo.free]
 */
typedef struct _PsyDisplayInfo {
    /*< private >*/
    char        *name;
    PsyRect     *rect;
    PsySize     *size_mm;
    PsyDuration *frame_dur;
} PsyDisplayInfo;

#define PSY_TYPE_DISPLAY_INFO psy_display_info_get_type()
G_MODULE_EXPORT GType
psy_display_info_get_type(void);

#define PSY_TYPE_SIZE psy_size_get_type()
G_MODULE_EXPORT GType
psy_size_get_type(void);

#define PSY_TYPE_POS psy_pos_get_type()
G_MODULE_EXPORT GType
psy_pos_get_type(void);

#define PSY_TYPE_RECT psy_rect_get_type()
G_MODULE_EXPORT GType
psy_rect_get_type(void);

/* *** PsySize *** */

G_MODULE_EXPORT PsySize *
psy_size_new(void);

G_MODULE_EXPORT PsySize *
psy_size_new_full(gint width, gint height);

G_MODULE_EXPORT void
psy_size_free(PsySize *self);

G_MODULE_EXPORT PsySize *
psy_size_copy(const PsySize *self);

/* *** PsyPos *** */

G_MODULE_EXPORT PsyPos *
psy_pos_new(void);

G_MODULE_EXPORT PsyPos *
psy_pos_new_full(gint x, gint y);

G_MODULE_EXPORT void
psy_pos_free(PsyPos *self);

G_MODULE_EXPORT PsyPos *
psy_pos_copy(const PsyPos *self);

/* *** PsyRect *** */

G_MODULE_EXPORT PsyRect *
psy_rect_new(void);

G_MODULE_EXPORT PsyRect *
psy_rect_new_full(gint x, gint y, gint width, gint height);

G_MODULE_EXPORT void
psy_rect_free(PsyRect *self);

G_MODULE_EXPORT PsyRect *
psy_rect_copy(const PsyRect *self);

/* *** PsyDisplayInfo *** */

G_MODULE_EXPORT PsyDisplayInfo *
psy_display_info_new(const gchar *name, const PsyDuration *frame_dur);

G_MODULE_EXPORT PsyDisplayInfo *
psy_display_info_new_full(const gchar       *name,
                          const PsyDuration *frame_dur,
                          gint               x,
                          gint               y,
                          gint               width,
                          gint               height,
                          gint               width_mm,
                          gint               height_mm);

G_MODULE_EXPORT PsyDisplayInfo *
psy_display_info_copy(const PsyDisplayInfo *self);

G_MODULE_EXPORT void
psy_display_info_free(PsyDisplayInfo *self);

G_MODULE_EXPORT const gchar *
psy_display_info_get_name(const PsyDisplayInfo *self);

G_MODULE_EXPORT const PsyDuration *
psy_display_info_get_frame_dur(const PsyDisplayInfo *self);
G_MODULE_EXPORT gint
psy_display_info_get_x(const PsyDisplayInfo *self);
G_MODULE_EXPORT gint
psy_display_info_get_y(const PsyDisplayInfo *self);
G_MODULE_EXPORT gint
psy_display_info_get_width(const PsyDisplayInfo *self);
G_MODULE_EXPORT gint
psy_display_info_get_height(const PsyDisplayInfo *self);

G_MODULE_EXPORT void
psy_display_info_set_frame_dur(PsyDisplayInfo    *self,
                               const PsyDuration *frame_dur);
G_MODULE_EXPORT void
psy_display_info_set_x(PsyDisplayInfo *self, gint x);
G_MODULE_EXPORT void
psy_display_info_set_y(PsyDisplayInfo *self, gint y);
G_MODULE_EXPORT void
psy_display_info_set_width(PsyDisplayInfo *self, gint width);
G_MODULE_EXPORT void
psy_display_info_set_height(PsyDisplayInfo *self, gint height);

G_END_DECLS