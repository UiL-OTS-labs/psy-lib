
#include <math.h>

#include "psy-color.h"

PsyRgba *
psy_rgba_new(gfloat red, gfloat green, gfloat blue);

PsyRgba *
psy_rgba_copy(PsyRgba *self);

void
psy_rgb_free(PsyRgba *self);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"

G_DEFINE_BOXED_TYPE(PsyRgba, psy_rgba, psy_rgba_copy, psy_rgba_free)

#pragma GCC diagnostic pop

/**
 * psy_rgba_new:
 * @red: The red component of the color
 * @green: The green component of the color
 * @blue: The blue component of the color
 *
 * The valid colors are between [0.0 and 1.0], if the values
 * go outside of that color, the colors will saturate.
 *
 * This will set the alpha channel to 1.0
 *
 * Returns:(transfer full): a newly created `PsyRgb` instance
 */
PsyRgba *
psy_rgba_new(gfloat red, gfloat green, gfloat blue)
{
    return psy_rgba_new_full(red, green, blue, 1.0);
}

/**
 * psy_rgba_new_full:
 * @red: The red component of the color
 * @green: The green component of the color
 * @blue: The blue component of the color
 * @alpha: The blue component of the color
 *
 * The valid colors are between [0.0 and 1.0], if the values
 * go outside of that color, the colors will saturate.
 *
 * Returns:(transfer full): a newly created `PsyRgb` instance
 */
PsyRgba *
psy_rgba_new_full(gfloat red, gfloat green, gfloat blue, gfloat alpha)
{
    PsyRgba *ret = g_slice_alloc(sizeof(PsyRgba));
    ret->r       = red;
    ret->g       = green;
    ret->b       = blue;
    ret->a       = alpha;

    return ret;
}

/**
 * psy_rgba_copy:
 * @self: The `PsyRgb` instance to copy
 *
 * Copy is made by value semantics.
 *
 * Returns:(transfer full): a copy of the input
 */
PsyRgba *
psy_rgba_copy(PsyRgba *self)
{
    g_return_val_if_fail(self, NULL);

    PsyRgba *out = g_memdup2(self, sizeof(PsyRgba));
    return out;
}

/**
 * psy_rgba_free:
 * @self: The `PsyRgba` instance to free
 *
 * free an instance of `PsyRgba`
 */
void
psy_rgba_free(PsyRgba *self)
{
    g_free(self);
}

/**
 * PsyColor:
 * A class representing a color.
 * Internally PsyColor uses gfloats because they are send to the GPU in this
 * format. Eventually the colors should end up in a range 0.0 <= color < 1.0,
 * however, this depends on what the entire rendering pipeline is going
 * to do with the color values.
 */

typedef struct _PsyColor {
    GObject parent;

    union {
        gfloat rgba[4];

        struct {
            gfloat red;
            gfloat green;
            gfloat blue;
            gfloat alpha;
        } colors;
    } values;
} PsyColor;

G_DEFINE_TYPE(PsyColor, psy_color, G_TYPE_OBJECT)

typedef enum {
    PROP_NULL, // not used required by GObject

    PROP_R,
    PROP_G,
    PROP_B,
    PROP_A,

    PROP_RI,
    PROP_GI,
    PROP_BI,
    PROP_AI,

    NUM_PROPERTIES
} ColorProperty;

static GParamSpec *color_properties[NUM_PROPERTIES] = {0};

static void
color_set_property(GObject      *object,
                   guint         property_id,
                   const GValue *value,
                   GParamSpec   *pspec)
{
    PsyColor    *self   = PSY_COLOR(object);
    const gfloat factor = (1.0 / 255.0);

    switch ((ColorProperty) property_id) {
    case PROP_R:
        self->values.rgba[0] = g_value_get_float(value);
        break;
    case PROP_G:
        self->values.rgba[1] = g_value_get_float(value);
        break;
    case PROP_B:
        self->values.rgba[2] = g_value_get_float(value);
        break;
    case PROP_A:
        self->values.rgba[3] = g_value_get_float(value);
        break;

    case PROP_RI:
        self->values.rgba[0] = factor * g_value_get_int(value);
        break;
    case PROP_GI:
        self->values.rgba[1] = factor * g_value_get_int(value);
        break;
    case PROP_BI:
        self->values.rgba[2] = factor * g_value_get_int(value);
        break;
    case PROP_AI:
        self->values.rgba[3] = factor * g_value_get_int(value);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

static void
color_get_property(GObject    *object,
                   guint       property_id,
                   GValue     *value,
                   GParamSpec *pspec)
{
    PsyColor    *self   = PSY_COLOR(object);
    const gfloat factor = 255.0;

    switch ((ColorProperty) property_id) {

    case PROP_R:
        g_value_set_float(value, self->values.rgba[0]);
        break;
    case PROP_G:
        g_value_set_float(value, self->values.rgba[1]);
        break;
    case PROP_B:
        g_value_set_float(value, self->values.rgba[2]);
        break;
    case PROP_A:
        g_value_set_float(value, self->values.rgba[3]);
        break;

    case PROP_RI:
        g_value_set_int(value, (int) (self->values.rgba[0] * factor));
        break;
    case PROP_GI:
        g_value_set_int(value, (int) (self->values.rgba[1] * factor));
        break;
    case PROP_BI:
        g_value_set_int(value, (int) (self->values.rgba[2] * factor));
        break;
    case PROP_AI:
        g_value_set_int(value, (int) (self->values.rgba[3] * factor));
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

static void
psy_color_init(PsyColor *self)
{
    (void) self;
}

static void
psy_color_class_init(PsyColorClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    object_class->set_property = color_set_property;
    object_class->get_property = color_get_property;

    color_properties[PROP_R] = g_param_spec_float("r",
                                                  "red",
                                                  "The red color component",
                                                  0.0,
                                                  1.0,
                                                  0.0,
                                                  G_PARAM_READWRITE);

    color_properties[PROP_G] = g_param_spec_float("g",
                                                  "green",
                                                  "The green color component",
                                                  0.0,
                                                  1.0,
                                                  0.0,
                                                  G_PARAM_READWRITE);

    color_properties[PROP_B] = g_param_spec_float("b",
                                                  "blue",
                                                  "The blue color component",
                                                  0.0,
                                                  1.0,
                                                  0.0,
                                                  G_PARAM_READWRITE);

    color_properties[PROP_A]
        = g_param_spec_float("a",
                             "alpha",
                             "The alpha color component",
                             0.0,
                             1.0,
                             1.0,
                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

    color_properties[PROP_RI]
        = g_param_spec_int("ri",
                           "red integer",
                           "The red color component in integer format",
                           0,
                           255,
                           0,
                           G_PARAM_READWRITE);

    color_properties[PROP_GI]
        = g_param_spec_int("gi",
                           "green integer",
                           "The green color component in integer format",
                           0,
                           255,
                           0,
                           G_PARAM_READWRITE);

    color_properties[PROP_BI]
        = g_param_spec_int("bi",
                           "blue integer",
                           "The blue color component in integer format",
                           0,
                           255,
                           0,
                           G_PARAM_READWRITE);

    color_properties[PROP_AI]
        = g_param_spec_int("ai",
                           "alpha integer",
                           "The alpha color component in integer format",
                           0,
                           255,
                           255,
                           G_PARAM_READWRITE);

    g_object_class_install_properties(
        object_class, NUM_PROPERTIES, color_properties);
}

/* ******************* Public functions ********************** */

/**
 * psy_color_new:
 *
 * Returns: a new color with r, g, b = 0 and a = 1.0
 */
PsyColor *
psy_color_new(void)
{
    return g_object_new(PSY_TYPE_COLOR, NULL);
}

/**
 * psy_color_new_rgb:
 * @r: the red value for the color
 * @g: the green value for the format
 * @b: the red value for the format
 *
 * Returns: a PsyColor with the specified values for r, g, b and an alpha of 1
 */
PsyColor *
psy_color_new_rgb(gfloat r, gfloat g, gfloat b)
{

    return psy_color_new_rgba(r, b, g, 1.0);
}

/**
 * psy_color_new_rgba:
 * @r: the red value for the color
 * @g: the green value for the format
 * @b: the red value for the format
 *
 * Returns: a PsyColor with the specified values for r, g, b and an alpha of 1
 */
PsyColor *
psy_color_new_rgba(gfloat r, gfloat g, gfloat b, gfloat a)
{
    g_warn_if_fail(r >= 0.0 && r <= 1.0);
    g_warn_if_fail(g >= 0.0 && g <= 1.0);
    g_warn_if_fail(b >= 0.0 && b <= 1.0);
    g_warn_if_fail(a >= 0.0 && a <= 1.0);

    r = CLAMP(r, 0.0, 1.0);
    g = CLAMP(g, 0.0, 1.0);
    b = CLAMP(b, 0.0, 1.0);
    a = CLAMP(a, 0.0, 1.0);

    return g_object_new(PSY_TYPE_COLOR, "r", r, "g", g, "b", b, "a", a, NULL);
}

/**
 * psy_color_new_rgbi:
 * @r: the red value for the color
 * @g: the green value for the format
 * @b: the red value for the format
 *
 * Specify colors in the range [0, 255]
 *
 * Returns: a PsyColor with the specified values for r, g, b and an alpha of 255
 */
PsyColor *
psy_color_new_rgbi(gint r, gint g, gint b)
{

    return psy_color_new_rgbai(r, b, g, 255);
}

/**
 * psy_color_new_rgbai:
 * @r: the red value for the color
 * @g: the green value for the format
 * @b: the red value for the format
 *
 * Returns: a PsyColor with the specified values for r, g, b and an alpha of 255
 */
PsyColor *
psy_color_new_rgbai(gint r, gint g, gint b, gint a)
{
    g_warn_if_fail(r >= 0 && r <= 255);
    g_warn_if_fail(g >= 0 && g <= 255);
    g_warn_if_fail(b >= 0 && b <= 255);
    g_warn_if_fail(a >= 0 && a <= 255);

    r = CLAMP(r, 0, 255);
    g = CLAMP(g, 0, 255);
    b = CLAMP(b, 0, 255);
    a = CLAMP(a, 0, 255);

    return g_object_new(
        PSY_TYPE_COLOR, "ri", r, "gi", g, "bi", b, "ai", a, NULL);
}

/**
 * psy_color_dup:
 * @self: a color to duplicate
 *
 * This duplicates an existing color.
 *
 * Returns:(transfer full): A copy of the input.
 */
PsyColor *
psy_color_dup(PsyColor *self)
{
    g_return_val_if_fail(PSY_IS_COLOR(self), NULL);
    // clang-format off
    PsyColor *dup = g_object_new(PSY_TYPE_COLOR,
                                 "r", self->values.colors.red,
                                 "g", self->values.colors.green,
                                 "b", self->values.colors.blue,
                                 "a", self->values.colors.alpha,
                                 NULL);
    // clang-format on
    return dup;
}

/**
 * psy_color_get_red:
 * @self: the color whose value to obtain.
 *
 * Returns the value in floating point format
 *
 * Returns: the value in floating point format
 */
gfloat
psy_color_get_red(PsyColor *self)
{
    g_return_val_if_fail(PSY_IS_COLOR(self), 0);
    return self->values.colors.red;
}

/**
 * psy_color_get_redi:
 * @self: the color whose value to obtain.
 *
 * Returns the value in floating point format
 *
 * Returns: the value in floating point format
 */
gint
psy_color_get_redi(PsyColor *self)
{
    g_return_val_if_fail(PSY_IS_COLOR(self), 0);
    return (gint) roundf(self->values.colors.red * 255);
}

/**
 * psy_color_set_red:
 * @self: the color whose value to obtain.
 * @red: the color in floating point format
 *
 * Set the red channel of the color.
 */
void
psy_color_set_red(PsyColor *self, gfloat red)
{
    g_return_if_fail(PSY_IS_COLOR(self));
    self->values.colors.red = red;
}

/**
 * psy_color_set_redi:
 * @self: the color whose value to obtain.
 * @red: the color in integer format
 *
 * Set the color in integer format.
 */
void
psy_color_set_redi(PsyColor *self, gint red)
{
    g_return_if_fail(PSY_IS_COLOR(self));
    self->values.colors.red = red / 255.0;
}

/**
 * psy_color_get_green:
 * @self: the color whose value to obtain.
 *
 * Returns the value in floating point format
 *
 * Returns: the value in floating point format
 */
gfloat
psy_color_get_green(PsyColor *self)
{
    g_return_val_if_fail(PSY_IS_COLOR(self), 0);
    return self->values.colors.green;
}

/**
 * psy_color_get_greeni:
 * @self: the color whose value to obtain.
 *
 * Returns the value in floating point format
 *
 * Returns: the value in floating point format
 */
gint
psy_color_get_greeni(PsyColor *self)
{
    g_return_val_if_fail(PSY_IS_COLOR(self), 0);
    return (gint) roundf(self->values.colors.green * 255);
}

/**
 * psy_color_set_green:
 * @self: the color whose value to obtain.
 * @green: the color in floating point format
 *
 * Set the green channel of the color.
 */
void
psy_color_set_green(PsyColor *self, gfloat green)
{
    g_return_if_fail(PSY_IS_COLOR(self));
    self->values.colors.green = green;
}

/**
 * psy_color_set_greeni:
 * @self: the color whose value to obtain.
 * @green: the color in integer format
 *
 * Set the color in integer format.
 */
void
psy_color_set_greeni(PsyColor *self, gint green)
{
    g_return_if_fail(PSY_IS_COLOR(self));
    self->values.colors.green = green / 255.0;
}

/**
 * psy_color_get_blue:
 * @self: the color whose value to obtain.
 *
 * Returns the value in floating point format
 *
 * Returns: the value in floating point format
 */
gfloat
psy_color_get_blue(PsyColor *self)
{
    g_return_val_if_fail(PSY_IS_COLOR(self), 0);
    return self->values.colors.blue;
}

/**
 * psy_color_get_bluei:
 * @self: the color whose value to obtain.
 *
 * Returns the value in floating point format
 *
 * Returns: the value in floating point format
 */
gint
psy_color_get_bluei(PsyColor *self)
{
    g_return_val_if_fail(PSY_IS_COLOR(self), 0);
    return (gint) roundf(self->values.colors.blue * 255);
}

/**
 * psy_color_set_blue:
 * @self: the color whose value to obtain.
 * @blue: the color in floating point format
 *
 * Set the blue channel of the color.
 */
void
psy_color_set_blue(PsyColor *self, gfloat blue)
{
    g_return_if_fail(PSY_IS_COLOR(self));
    self->values.colors.blue = blue;
}

/**
 * psy_color_set_bluei:
 * @self: the color whose value to obtain.
 * @blue: the color in integer format
 *
 * Set the color in integer format.
 */
void
psy_color_set_bluei(PsyColor *self, gint blue)
{
    g_return_if_fail(PSY_IS_COLOR(self));
    self->values.colors.blue = blue / 255.0;
}

/**
 * psy_color_get_alpha:
 * @self: the color whose value to obtain.
 *
 * Returns the value in floating point format
 *
 * Returns: the value in floating point format
 */
gfloat
psy_color_get_alpha(PsyColor *self)
{
    g_return_val_if_fail(PSY_IS_COLOR(self), 0);
    return self->values.colors.alpha;
}

/**
 * psy_color_get_alphai:
 * @self: the color whose value to obtain.
 *
 * Returns the value in floating point format
 *
 * Returns: the value in floating point format
 */
gint
psy_color_get_alphai(PsyColor *self)
{
    g_return_val_if_fail(PSY_IS_COLOR(self), 0);
    return (gint) roundf(self->values.colors.alpha * 255);
}

/**
 * psy_color_set_alpha:
 * @self: the color whose value to obtain.
 * @alpha: the color in floating point format
 *
 * Set the alpha channel of the color.
 */
void
psy_color_set_alpha(PsyColor *self, gfloat alpha)
{
    g_return_if_fail(PSY_IS_COLOR(self));
    self->values.colors.alpha = alpha;
}

/**
 * psy_color_set_alphai:
 * @self: the color whose value to obtain.
 * @alpha: the color in integer format
 *
 * Set the color in integer format.
 */
void
psy_color_set_alphai(PsyColor *self, gint alpha)
{
    g_return_if_fail(PSY_IS_COLOR(self));
    self->values.colors.alpha = alpha / 255.0;
}

/**
 * psy_color_equal:
 * @self: an instance of [class@PsyColor]
 * @other: another instance of [class@PsyColor]
 *
 * This function checks whether self == other.
 *
 * Returns: `TRUE` is @self == @other, `FALSE` otherwise
 */
gboolean
psy_color_equal(PsyColor *self, PsyColor *other)
{
    g_return_val_if_fail(PSY_IS_COLOR(self), FALSE);
    g_return_val_if_fail(PSY_IS_COLOR(other), FALSE);

    return self->values.colors.red == other->values.colors.red
           && self->values.colors.green == other->values.colors.green
           && self->values.colors.blue == other->values.colors.blue
           && self->values.colors.alpha == other->values.colors.alpha;
}

/**
 * psy_color_not_equal:
 * @self: an instance of [class@PsyColor]
 * @other: another instance of [class@PsyColor]
 *
 * This function checks whether self != other.
 *
 * Returns: `TRUE` is @self != @other, `FALSE` otherwise
 */
gboolean
psy_color_not_equal(PsyColor *self, PsyColor *other)
{
    g_return_val_if_fail(PSY_IS_COLOR(self), FALSE);
    g_return_val_if_fail(PSY_IS_COLOR(other), FALSE);

    return !psy_color_equal(self, other);
}

/**
 * psy_color_equal_eps:
 * @self: an instance of [class@PsyColor]
 * @other: another instance of [class@PsyColor]
 * @eps: epsilon value that the colors are considered to be the same
 *
 * This function checks whether self == other.
 *
 * The color are considered the same if |self.red - other.red| <= eps,
 * holds for all four color values
 *
 * As pixels within an image are converted between a unsigned 8 bit integer
 * and a floating point number, you might lose some precision.
 *
 * Then you might want to do something like `psy_color_equal_eps(a, b, 1.0/255)`
 * to allow for a small difference in 8 bit to float conversion. If there should
 * be only rounding errors, you might opt for a value of (1.0/255) / 2.0
 * instead.
 *
 * Returns: `TRUE` is @self == @other, `FALSE` otherwise
 */
gboolean
psy_color_equal_eps(PsyColor *self, PsyColor *other, gfloat eps)
{
    g_return_val_if_fail(PSY_IS_COLOR(self), FALSE);
    g_return_val_if_fail(PSY_IS_COLOR(other), FALSE);

    gfloat diff[4]
        = {fabs(self->values.colors.red - other->values.colors.red),
           fabs(self->values.colors.green - other->values.colors.green),
           fabs(self->values.colors.blue - other->values.colors.blue),
           fabs(self->values.colors.alpha - other->values.colors.alpha)};

    return diff[0] <= eps && diff[1] <= eps && diff[2] <= eps && diff[3] <= eps;
}

/**
 * psy_color_not_equal_eps:
 * @self: an instance of [class@PsyColor]
 * @other: another instance of [class@PsyColor]
 *
 * This function checks whether self != other.
 * See the discussion of [method@Psy.Color.is_equal] why an _eps version of
 * this function exists.
 *
 * Returns: `TRUE` is @self != @other, `FALSE` otherwise
 */
gboolean
psy_color_not_equal_eps(PsyColor *self, PsyColor *other, gfloat eps)
{
    g_return_val_if_fail(PSY_IS_COLOR(self), FALSE);
    g_return_val_if_fail(PSY_IS_COLOR(other), FALSE);

    return !psy_color_equal_eps(self, other, eps);
}
