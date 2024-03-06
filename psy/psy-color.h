
#pragma once

#include <gio/gio.h>
#include <glib-object.h>
#include <glib.h>

G_BEGIN_DECLS

/**
 * PSY_TYPE_RGBA:
 *
 * The #GType for a boxed holding a floating point RGB range
 */
#define PSY_TYPE_RGBA (psy_rgba_get_type())
G_MODULE_EXPORT GType
psy_rgba_get_type(void);

typedef struct _PsyRgba PsyRgba;

/**
 * PsyRgba:
 * @r: The red component of the color [0.0 - 1.0]
 * @g: The green component of the color [0.0 - 1.0]
 * @b: The blue component of the color [0.0 - 1.0]
 * @a: The alpha component of the color [0.0 - 1.0]
 *
 * You can use PsyRgb to specify the color in Red, Blue, Green and Alpha
 * components The useful range is between 0 and 1.0, if you go beyond this
 * border the color will saturate.
 */
struct _PsyRgba {
    gfloat r, g, b, a;
};

G_MODULE_EXPORT PsyRgba *
psy_rgba_new(gfloat red, gfloat green, gfloat blue);

G_MODULE_EXPORT PsyRgba *
psy_rgba_new_full(gfloat red, gfloat green, gfloat blue, gfloat alpha);

G_MODULE_EXPORT PsyRgba *
psy_rgba_copy(PsyRgba *self);

G_MODULE_EXPORT void
psy_rgba_free(PsyRgba *self);

/* ******* psy-color ********** */

#define PSY_TYPE_COLOR psy_color_get_type()
G_MODULE_EXPORT
G_DECLARE_FINAL_TYPE(PsyColor, psy_color, PSY, COLOR, GObject)

G_MODULE_EXPORT PsyColor *
psy_color_new(void);

G_MODULE_EXPORT PsyColor *
psy_color_new_rgb(gfloat r, gfloat g, gfloat b);

G_MODULE_EXPORT PsyColor *
psy_color_new_rgba(gfloat r, gfloat g, gfloat b, gfloat a);

G_MODULE_EXPORT PsyColor *
psy_color_new_rgbi(gint r, gint g, gint b);

G_MODULE_EXPORT PsyColor *
psy_color_new_rgbai(gint r, gint g, gint b, gint a);

G_MODULE_EXPORT void
psy_color_free(PsyColor *self);

G_MODULE_EXPORT PsyColor *
psy_color_copy(PsyColor *self);

G_MODULE_EXPORT gfloat
psy_color_get_red(PsyColor *self);

G_MODULE_EXPORT gint
psy_color_get_redi(PsyColor *self);

G_MODULE_EXPORT void
psy_color_set_red(PsyColor *self, gfloat red);

G_MODULE_EXPORT void
psy_color_set_redi(PsyColor *self, gint red);

G_MODULE_EXPORT gfloat
psy_color_get_green(PsyColor *self);

G_MODULE_EXPORT gint
psy_color_get_greeni(PsyColor *self);

G_MODULE_EXPORT void
psy_color_set_green(PsyColor *self, gfloat green);

G_MODULE_EXPORT void
psy_color_set_greeni(PsyColor *self, gint green);

G_MODULE_EXPORT gfloat
psy_color_get_blue(PsyColor *self);

G_MODULE_EXPORT gint
psy_color_get_bluei(PsyColor *self);

G_MODULE_EXPORT void
psy_color_set_blue(PsyColor *self, gfloat blue);

G_MODULE_EXPORT void
psy_color_set_bluei(PsyColor *self, gint blue);

G_MODULE_EXPORT gfloat
psy_color_get_alpha(PsyColor *self);

G_MODULE_EXPORT gint
psy_color_get_alphai(PsyColor *self);

G_MODULE_EXPORT void
psy_color_set_alpha(PsyColor *self, gfloat alpha);

G_MODULE_EXPORT void
psy_color_set_alphai(PsyColor *self, gint alpha);

G_MODULE_EXPORT gboolean
psy_color_equal(PsyColor *self, PsyColor *other);

G_MODULE_EXPORT gboolean
psy_color_not_equal(PsyColor *self, PsyColor *other);

G_MODULE_EXPORT gboolean
psy_color_equal_eps(PsyColor *self, PsyColor *other, gfloat eps);

G_MODULE_EXPORT gboolean
psy_color_not_equal_eps(PsyColor *self, PsyColor *other, gfloat eps);

// G_MODULE_EXPORT PsyColor*
// psy_color_new_hsv(guint8 h, guint8 s, guint8 v);

G_END_DECLS
