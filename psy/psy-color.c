
#include "psy-color.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
G_DEFINE_BOXED_TYPE(PsyRgb, psy_rgb, psy_rgb_copy, psy_rgb_free)
#pragma GCC diagnostic pop

/**
 * psy_rgb_new:
 * @red: The red component of the color
 * @green: The green component of the color
 * @blue: The blue component of the color
 *
 * The valid colors are between [0.0 and 1.0], if the values
 * go outside of that color, the colors will saturate.
 *
 * Returns:(transfer full): a newly created `PsyRgb` instance
 */
PsyRgb*
psy_rgb_new(gfloat red, gfloat green, gfloat blue)
{
    PsyRgb* rgb = g_slice_new(PsyRgb);
    
    rgb->r = red;
    rgb->g = green;
    rgb->b = blue;

    return rgb;
}

/*
 * psy_rgb_copy:
 * @out: The `PsyRgb` instance to copy
 *
 * Copy is made by value semantics.
 *
 * Returns:(transfer full): a copy of the input
 */
PsyRgb* 
psy_rgb_copy(PsyRgb* in)
{
    g_return_val_if_fail(in, NULL);

    PsyRgb* out = g_memdup2(in, sizeof(PsyRgb));
    return out;
}


/**
 * psy_rgb_free:
 * @in: The `PsyRgb` instance to free
 *
 * free an instance of `PsyRgb
 */
void
psy_rgb_free(PsyRgb* in)
{
    g_free(in);
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
    gfloat rgba[4];
} PsyColor;

G_DEFINE_TYPE(PsyColor, psy_color, G_TYPE_OBJECT)


typedef enum {
    PROP_NULL,          // not used required by GObject

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


static GParamSpec*  color_properties[NUM_PROPERTIES] = {0};

static void
color_set_property(GObject        *object,
                   guint           property_id,
                   const GValue   *value,
                   GParamSpec     *pspec
                   )              
{
    PsyColor* self = PSY_COLOR(object);
    const gfloat factor = (1.0/255.0);

    switch((ColorProperty) property_id) {
        case PROP_R:
            self->rgba[0] = g_value_get_float(value);
            break;
        case PROP_G:
            self->rgba[1] = g_value_get_float(value);
            break;
        case PROP_B:
            self->rgba[2] = g_value_get_float(value);
            break;
        case PROP_A:
            self->rgba[3] = g_value_get_float(value);
            break;

        case PROP_RI:
            self->rgba[0] = factor * g_value_get_int(value);
            break;
        case PROP_GI:
            self->rgba[1] = factor * g_value_get_int(value);
            break;
        case PROP_BI:
            self->rgba[2] = factor * g_value_get_int(value);
            break;
        case PROP_AI:
            self->rgba[3] = factor * g_value_get_int(value);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

static void
color_get_property(GObject       *object,
                   guint          property_id,
                   GValue        *value,
                   GParamSpec    *pspec
                   )
{
    PsyColor* self = PSY_COLOR(object);
    const gfloat factor = 255.0;

    switch((ColorProperty) property_id) {

        case PROP_R:
            g_value_set_float(value, self->rgba[0]);
            break;
        case PROP_G:
            g_value_set_float(value, self->rgba[1]);
            break;
        case PROP_B:
            g_value_set_float(value, self->rgba[2]);
            break;
        case PROP_A:
            g_value_set_float(value, self->rgba[3]);
            break;

        case PROP_RI:
            g_value_set_int(value, (int) (self->rgba[0] * factor));
            break;
        case PROP_GI:
            g_value_set_int(value, (int) (self->rgba[1] * factor));
            break;
        case PROP_BI:
            g_value_set_int(value, (int) (self->rgba[2] * factor));
            break;
        case PROP_AI:
            g_value_set_int(value, (int) (self->rgba[3] * factor));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

static void
psy_color_init(PsyColor* self)
{
    (void) self;
}

static void
psy_color_class_init(PsyColorClass* klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    object_class->set_property = color_set_property;
    object_class->get_property = color_get_property;

    color_properties[PROP_R] = g_param_spec_float(
            "r",
            "red",
            "The red color component",
            0.0,
            1.0,
            0.0,
            G_PARAM_READWRITE
            );
    
    color_properties[PROP_G] = g_param_spec_float(
            "g",
            "green",
            "The green color component",
            0.0,
            1.0,
            0.0,
            G_PARAM_READWRITE
            );
    
    color_properties[PROP_B] = g_param_spec_float(
            "b",
            "blue",
            "The blue color component",
            0.0,
            1.0,
            0.0,
            G_PARAM_READWRITE
            );
    
    color_properties[PROP_A] = g_param_spec_float(
            "a",
            "alpha",
            "The alpha color component",
            0.0,
            1.0,
            1.0,
            G_PARAM_READWRITE | G_PARAM_CONSTRUCT
            );
    
    color_properties[PROP_RI] = g_param_spec_int(
            "ri",
            "red integer",
            "The red color component in integer format",
            0,
            255,
            0,
            G_PARAM_READWRITE
            );
    
    color_properties[PROP_GI] = g_param_spec_int(
            "gi",
            "green integer",
            "The green color component in integer format",
            0,
            255,
            0,
            G_PARAM_READWRITE
            );
    
    color_properties[PROP_BI] = g_param_spec_int(
            "bi",
            "blue integer",
            "The blue color component in integer format",
            0,
            255,
            0,
            G_PARAM_READWRITE
            );
    
    color_properties[PROP_AI] = g_param_spec_int(
            "ai",
            "alpha integer",
            "The alpha color component in integer format",
            0,
            255,
            255,
            G_PARAM_READWRITE
            );

    g_object_class_install_properties(
            object_class, NUM_PROPERTIES, color_properties
    );
}

/* ******************* Public functions ********************** */

/**
 * psy_color_new:
 *
 * Returns: a new color with r, g, b = 0 and a = 1.0
 */
PsyColor*
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
PsyColor*
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
PsyColor*
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

    return g_object_new(
            PSY_TYPE_COLOR,
            "r", r,
            "g", g,
            "b", b,
            "a", a,
            NULL
            );
}

/**
 * psy_color_new_rgbi:
 * @r: the red value for the color  
 * @g: the green value for the format
 * @b: the red value for the format
 * @a: the alpha value for the format
 *
 * Specify colors in the range [0, 255]
 *
 * Returns: a PsyColor with the specified values for r, g, b and an alpha of 255 
 */
PsyColor*
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
PsyColor*
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
            PSY_TYPE_COLOR,
            "r", r,
            "g", g,
            "b", b,
            "a", a,
            NULL
            );
}

