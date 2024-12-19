
#include "psy-display-info.h"

PsySize *
psy_size_copy(const PsySize *self);
void
psy_size_free(PsySize *self);

PsyPos *
psy_pos_copy(const PsyPos *self);
void
psy_pos_free(PsyPos *self);

PsyRect *
psy_rect_copy(const PsyRect *self);
void
psy_rect_free(PsyRect *self);

PsyDisplayInfo *
psy_display_info_copy(const PsyDisplayInfo *self);
void
psy_display_info_free(PsyDisplayInfo *self);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"

G_DEFINE_BOXED_TYPE(PsyDisplayInfo,
                    psy_display_info,
                    psy_display_info_copy,
                    psy_display_info_free)

G_DEFINE_BOXED_TYPE(PsySize, psy_size, psy_size_copy, psy_size_free)

G_DEFINE_BOXED_TYPE(PsyPos, psy_pos, psy_pos_copy, psy_pos_free)

G_DEFINE_BOXED_TYPE(PsyRect, psy_rect, psy_rect_copy, psy_rect_free)

#pragma GCC diagnostic pop

/* ************************* PsySize ************************************* */

/**
 * psy_size_new:(constructor)
 *
 * Create a new size with width and height set to 0.
 *
 * Return: A new size object
 */
PsySize *
psy_size_new(void)
{
    return g_new0(PsySize, 1);
}

/**
 * psy_size_new_full:(constructor)
 * @width: the width of the size
 * @height: the height of the size
 *
 * Create a new size with width and height set to specific values.
 *
 * Return: A new size object
 */
PsySize *
psy_size_new_full(gint width, gint height)
{
    PsySize *size = g_new(PsySize, 1);
    size->width   = width;
    size->height  = height;
    return size;
}

/**
 * psy_size_copy:
 * @self: The size to make a copy of.
 *
 * Returns: a new size equal to self.
 */
PsySize *
psy_size_copy(const PsySize *self)
{
    PsySize *copy = g_new(PsySize, 1);
    memcpy(copy, self, sizeof(PsySize));
    return copy;
}

/**
 * psy_size_free:(skip)
 *
 */
void
psy_size_free(PsySize *self)
{
    g_free(self);
}

/* ************************* PsyPos ************************************* */

/**
 * psy_pos_new:(constructor)
 *
 * Returns: A new instance of [struct@Pos] at the origin(0,0)
 */
PsyPos *
psy_pos_new(void)
{
    return g_new0(PsyPos, 1);
}

/**
 * psy_pos_new_full:(constructor)
 *
 * Returns: A new instance of [struct@Pos] at (x,y)
 */
PsyPos *
psy_pos_new_full(gint x, gint y)
{
    PsyPos *pos = g_new(PsyPos, 1);
    pos->x      = x;
    pos->y      = y;
    return pos;
}

/**
 * psy_pos_copy:
 * @self: the position to copy
 *
 * Returns: a copy of self
 */
PsyPos *
psy_pos_copy(const PsyPos *self)
{
    PsyPos *copy = g_new(PsyPos, 1);
    memcpy(copy, self, sizeof(PsyPos));
    return copy;
}

/**
 * psy_pos_free:(skip)
 *
 * Frees previously created instances of [struct@Pos]
 */
void
psy_pos_free(PsyPos *self)
{
    g_free(self);
}

/* *********************** PsyRect ********************** */

/**
 * psy_rect_new:(constructor)
 *
 * Create a new Rectangle with the default size (0, 0) at
 * the origin (0,0)
 */
PsyRect *
psy_rect_new(void)
{
    PsyRect *ret = g_new(PsyRect, 1);
    ret->pos     = psy_pos_new();
    ret->size    = psy_size_new();
    return ret;
}

/**
 * psy_rect_new_full:
 * @x: the x position of the rect
 * @y: the y position of the rect
 * @width: the width of the rect
 * @height: the height of the rect
 *
 * Create a new Rect with a given postion and size
 */
PsyRect *
psy_rect_new_full(gint x, gint y, gint width, gint height)
{
    PsyRect *rect = g_new(PsyRect, 1);
    rect->pos     = psy_pos_new_full(x, y);
    rect->size    = psy_size_new_full(width, height);
    return rect;
}

/**
 * psy_rect_copy:
 * @self: the rect to copy
 *
 * Returns: a new copy of self
 */
PsyRect *
psy_rect_copy(const PsyRect *self)
{
    PsyRect *copy = g_new(PsyRect, 1);
    copy->pos     = psy_pos_copy(self->pos);
    copy->size    = psy_size_copy(self->size);
    return copy;
}

/**
 * psy_rect_free:(skip)
 * @self: the rectangle to free
 */
void
psy_rect_free(PsyRect *self)
{
    psy_pos_free(self->pos);
    psy_size_free(self->size);
    g_free(self);
}

/* **************** PsyDisplayInfo ************* */

/**
 * psy_display_info_new:(constructor)
 * @name: the name of the display
 * @frame_dur: the duration of one frame
 *
 * Create a new display info. It at the origin(0,0)
 * and also with the size (0,0).
 *
 * So after creating the object like this you
 * propably want to set its parameters, regarding
 * the position and size
 *
 * Returns: A new DisplayInfo
 */
PsyDisplayInfo *
psy_display_info_new(const gchar *name, const PsyDuration *frame_dur)
{
    PsyDisplayInfo *info = g_new(PsyDisplayInfo, 1);
    info->name           = g_strdup(name);
    info->frame_dur      = psy_duration_copy((PsyDuration *) frame_dur);
    info->rect           = psy_rect_new();
    info->size_mm        = psy_size_new_full(-1, -1);
    return info;
}

/**
 * psy_display_info_new_full:(constructor)
 * @name: the name of this dispaly
 * @frame_dur:(transfer none): the duration between two successive frames.
 * @x: the x position of the display
 * @y: the y position of the display
 * @width: the width of the display in number of pixels
 * @height: the height of the display in number of pixels
 * @width_mm: the width of the display in mm or -1 if unknown
 * @height_mm: the height of the display in mm or -1 if unknown
 *
 */
PsyDisplayInfo *
psy_display_info_new_full(const gchar       *name,
                          const PsyDuration *frame_dur,
                          gint               x,
                          gint               y,
                          gint               width,
                          gint               height,
                          gint               width_mm,
                          gint               height_mm)
{
    PsyDisplayInfo *info = g_new(PsyDisplayInfo, 1);
    info->name           = g_strdup(name);
    info->frame_dur      = psy_duration_copy((PsyDuration *) frame_dur);
    info->rect           = psy_rect_new_full(x, y, width, height);
    info->size_mm        = psy_size_new_full(width_mm, height_mm);
    return info;
}

PsyDisplayInfo *
psy_display_info_copy(const PsyDisplayInfo *self)
{
    PsyDisplayInfo *new = g_new(PsyDisplayInfo, 1);
    new->name           = g_strdup(self->name);
    new->frame_dur      = psy_duration_copy(self->frame_dur);
    new->rect           = psy_rect_copy(self->rect);
    new->size_mm        = psy_size_copy(self->size_mm);
    return new;
}

void
psy_display_info_free(PsyDisplayInfo *self)
{
    g_free(self->name);
    psy_duration_free(self->frame_dur);
    psy_rect_free(self->rect);
    psy_size_free(self->size_mm);
    g_free(self);
}

/**
 * psy_display_info_get_frame_dur:
 * @self: the display from which to query the frame dur.
 *
 * Get the frame dur of the display info
 *
 * Returns:(transfer none): The duration of one frame on this display.
 */
const PsyDuration *
psy_display_info_get_frame_dur(const PsyDisplayInfo *self)
{
    g_return_val_if_fail(self != NULL, NULL);
    return self->frame_dur;
}

/**
 * psy_display_info_get_x:
 *
 * Get the x position of the screen
 */
gint
psy_display_info_get_x(const PsyDisplayInfo *self)
{
    g_return_val_if_fail(self != NULL, G_MININT);
    return self->rect->pos->x;
}

/**
 * psy_display_info_get_y:
 *
 * Get the y position of the screen
 */
gint
psy_display_info_get_y(const PsyDisplayInfo *self)
{
    g_return_val_if_fail(self != NULL, G_MININT);
    return self->rect->pos->y;
}

/**
 * psy_display_info_get_width:
 *
 * Get the width of the screen
 */
gint
psy_display_info_get_width(const PsyDisplayInfo *self)
{
    g_return_val_if_fail(self != NULL, G_MININT);
    return self->rect->size->width;
}

/**
 * psy_display_info_get_height:
 *
 * Get the height of the screen
 */
gint
psy_display_info_get_height(const PsyDisplayInfo *self)
{
    g_return_val_if_fail(self != NULL, G_MININT);
    return self->rect->size->height;
}

/**
 * psy_display_info_set_frame_dur:
 * @frame_dur:(transfer none): the framedur for this display
 *
 * Set the frame duration that belongs to this display.
 */
G_MODULE_EXPORT void
psy_display_info_set_frame_dur(PsyDisplayInfo    *self,
                               const PsyDuration *frame_dur)
{
    g_return_if_fail(self != NULL);
    psy_duration_free(self->frame_dur);
    self->frame_dur = psy_duration_copy((PsyDuration *) frame_dur);
}

/**
 * psy_display_info_set_x:
 * @x: The x postion of this display
 *
 * Set the x postion of this display
 */
void
psy_display_info_set_x(PsyDisplayInfo *self, gint x)
{
    g_return_if_fail(self != NULL);
    self->rect->pos->x = x;
}

/**
 * psy_display_info_set_y:
 * @y: the y postion of the display
 */
void
psy_display_info_set_y(PsyDisplayInfo *self, gint y)
{
    g_return_if_fail(self != NULL);
    self->rect->pos->y = y;
}

/**
 * psy_display_info_set_width:
 * @width: the width of this display in number of pixels
 */
void
psy_display_info_set_width(PsyDisplayInfo *self, gint width)
{
    g_return_if_fail(self != NULL);
    self->rect->size->width = width;
}

/**
 * psy_display_set_height:
 * @self: A PsyDisplayInfo
 * @height: the height of the display in pixels
 *
 * Set the height of this display.
 */
void
psy_display_info_set_height(PsyDisplayInfo *self, gint height)
{
    g_return_if_fail(self != NULL);
    self->rect->size->height = height;
}