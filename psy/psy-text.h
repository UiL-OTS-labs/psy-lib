
#pragma once

#include <pango/pango.h>

#include "psy-rectangle.h"

G_BEGIN_DECLS

#define PSY_TYPE_TEXT psy_text_get_type()

G_MODULE_EXPORT
G_DECLARE_DERIVABLE_TYPE(PsyText, psy_text, PSY, TEXT, PsyRectangle)

/**
 * PsyTextClass:
 *
 * The PsyTextClass is a (Base) class for stimuli containing text in a nice
 * layout.
 */
typedef struct _PsyTextClass {
    PsyRectangleClass parent;

    gpointer reserved[16];
} PsyTextClass;

G_MODULE_EXPORT PsyText *
psy_text_new(PsyCanvas *canvas);

G_MODULE_EXPORT PsyText *
psy_text_new_full(PsyCanvas   *canvas,
                  gfloat       x,
                  gfloat       y,
                  gfloat       width,
                  gfloat       height,
                  const gchar *content,
                  gboolean     use_markup);

G_MODULE_EXPORT void
psy_text_free(PsyText *self);

G_MODULE_EXPORT const gchar *
psy_text_get_content(PsyText *self);

G_MODULE_EXPORT void
psy_text_set_content(PsyText *self, const gchar *content);

G_MODULE_EXPORT void
psy_text_set_use_markup(PsyText *self, gboolean use_markup);

G_MODULE_EXPORT gboolean
psy_text_get_use_markup(PsyText *self);

G_MODULE_EXPORT void
psy_text_set_font_color(PsyText *self, PsyColor *font_color);

G_MODULE_EXPORT PsyColor *
psy_text_get_font_color(PsyText *self);

G_MODULE_EXPORT gboolean
psy_text_get_is_dirty(PsyText *self);

G_MODULE_EXPORT PsyImage *
psy_text_create_stimulus(PsyText *self);

G_MODULE_EXPORT const gchar *
psy_text_get_font_family(PsyText *self);

G_MODULE_EXPORT void
psy_text_set_font_family(PsyText *self, const gchar *font_fam);

void
psy_text_set_is_dirty(PsyText *self, gboolean dirty);

PangoFontDescription *
psy_text_get_font_description(PsyText *self);

G_END_DECLS
