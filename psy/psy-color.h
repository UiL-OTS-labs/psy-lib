

#include <glib-object.h>
#include <glib.h>
#include <gio/gio.h>

G_BEGIN_DECLS


/**
 * PSY_TYPE_RGB:
 *
 * The #GType for a boxed holding a floating point RGB range
 */
#define PSY_TYPE_RGB (psy_rgba_get_type())
G_MODULE_EXPORT GType psy_rgba_get_type();

typedef struct _PsyRgba PsyRgba;

/**
 * PsyRgba:
 * @r: The red component of the color [0.0 - 1.0] 
 * @g: The green component of the color [0.0 - 1.0] 
 * @b: The blue component of the color [0.0 - 1.0] 
 * @a: The alpha component of the color [0.0 - 1.0] 
 *
 * You can use PsyRgb to specify the color in Red, Blue, Green and Alpha components
 * The useful range is between 0 and 1.0, if you go beyond this border the
 * color will saturate.
 */
 struct _PsyRgba {
    gfloat r, g, b, a;
};

G_MODULE_EXPORT PsyRgba*
psy_rgba_new(gfloat red, gfloat green, gfloat blue);

G_MODULE_EXPORT PsyRgba*
psy_rgba_new_full(gfloat red, gfloat green, gfloat blue, gfloat alpha);

G_MODULE_EXPORT PsyRgba*
psy_rgba_copy(PsyRgba* self);

G_MODULE_EXPORT void
psy_rgba_free(PsyRgba* self);

/* ******* psy-color ********** */

#define PSY_TYPE_COLOR psy_color_get_type()
G_DECLARE_FINAL_TYPE(PsyColor, psy_color, PSY, COLOR, GObject)

G_MODULE_EXPORT PsyColor*
psy_color_new(void);

G_MODULE_EXPORT PsyColor*
psy_color_new_rgb(gfloat r, gfloat g, gfloat b);

G_MODULE_EXPORT PsyColor*
psy_color_new_rgba(gfloat r, gfloat g, gfloat b, gfloat a);

G_MODULE_EXPORT PsyColor*
psy_color_new_rgbi(gint r, gint g, gint b);

G_MODULE_EXPORT PsyColor*
psy_color_new_rgbai(gint r, gint g, gint b, gint a);

//G_MODULE_EXPORT PsyColor*
//psy_color_new_hsv(guint8 h, guint8 s, guint8 v);

G_END_DECLS
