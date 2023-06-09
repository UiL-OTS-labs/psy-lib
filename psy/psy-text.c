

#include <math.h>
#include <pango/pango.h>
#include <pango/pangocairo.h>

#include "psy-clock.h"
#include "psy-enums.h"
#include "psy-text-artist.h"
#include "psy-text.h"
#include "psy-time-point.h"
#include "psy-vector4.h"

#define CAIRO_ALPHA_MASK (0xFF << 24)
#define CAIRO_RED_MASK (0xFF << 16)
#define CAIRO_GREEN_MASK (0xFF << 8)
#define CAIRO_BLUE_MASK (0xFF << 0)

/**
 * PsyText:
 *
 * PsyText is a class that is created to display text with optional capabilities
 * to render Bold, Italic etc. using the pango markup language. A text stimulus
 * is basically a Rectangle that contains text. That is the
 * [property@VisualStimulus:color] is used to color the rectangle and the
 * [property@Text:font-color] is used to color the letters of the text. The
 * text stimulus is basically a Rectangle with a Picture containing the text.
 * The font color will be used by default when drawing and laying out the
 * text on the rectangle, the fontcolor may also be adapted by using the
 * pango markup language see: https://docs.gtk.org/Pango/pango_markup.html
 */

// Using PangoCairo, you create a context for a specific `cairo_t* cr`;
// static PangoContext *g_pango_context = NULL;

typedef struct _PsyTextPrivate {
    gchar                *content;          // The text optionally with markup
    gboolean              use_markup;       // Whether or not to use markup.
    PsyColor             *font_color;       // the default color of the font.
    PangoFontDescription *font_description; // A description of the font.
    gboolean is_dirty; // whether or not the image should be uploaded.
} PsyTextPrivate;

G_DEFINE_TYPE_WITH_PRIVATE(PsyText, psy_text, PSY_TYPE_RECTANGLE)

typedef enum {
    PROP_NULL,       // not used required by GObject
    PROP_TEXT,       // set content to text and use-markup to false
    PROP_MARKUP,     // set content to markup and use-markup to true
    PROP_USE_MARKUP, // Whether or not the content contains markup
    PROP_FONT_COLOR, // The default color of the font.
    PROP_IS_DIRTY, // Whether or not the Artist should update the text stimulus
    PROP_FONT_FAM, // The font family of the stimulus
    NUM_PROPERTIES
} TextProperty;

static GParamSpec *text_properties[NUM_PROPERTIES] = {0};

static void
text_set_property(GObject      *object,
                  guint         property_id,
                  const GValue *value,
                  GParamSpec   *pspec)
{
    PsyText *self = PSY_TEXT(object);

    switch ((TextProperty) property_id) {
    case PROP_TEXT:
        psy_text_set_use_markup(self, FALSE);
        psy_text_set_content(self, g_value_get_string(value));
        break;
    case PROP_MARKUP:
        psy_text_set_use_markup(self, TRUE);
        psy_text_set_content(self, g_value_get_string(value));
        break;
    case PROP_USE_MARKUP:
        psy_text_set_use_markup(self, g_value_get_boolean(value));
        break;
    case PROP_FONT_COLOR:
        psy_text_set_font_color(self, g_value_get_object(value));
        break;
    case PROP_FONT_FAM:
        psy_text_set_font_family(self, g_value_get_string(value));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

static void
text_get_property(GObject    *object,
                  guint       property_id,
                  GValue     *value,
                  GParamSpec *pspec)
{
    PsyText        *self = PSY_TEXT(object);
    PsyTextPrivate *priv = psy_text_get_instance_private(self);

    switch ((TextProperty) property_id) {
    case PROP_USE_MARKUP:
        g_value_set_boolean(value, priv->use_markup);
        break;
    case PROP_FONT_COLOR:
        g_value_set_object(value, priv->font_color);
        break;
    case PROP_IS_DIRTY:
        g_value_set_boolean(value, priv->is_dirty);
        break;
    case PROP_FONT_FAM:
        g_value_set_string(value, psy_text_get_font_family(self));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

static void
psy_text_init(PsyText *self)
{
    PsyTextPrivate *priv = psy_text_get_instance_private(self);

    priv->font_color       = psy_color_new_rgb(1.0, 1.0, 1.0);
    priv->font_description = pango_font_description_from_string("Sans 25pt");
    priv->is_dirty         = TRUE;
    priv->use_markup       = FALSE;
}

static PsyArtist *
text_create_artist(PsyVisualStimulus *self)
{
    return PSY_ARTIST(
        psy_text_artist_new(psy_visual_stimulus_get_canvas(self), self));
}

static void
text_set_color(PsyVisualStimulus *stimulus, PsyColor *color)
{
    PsyText        *self = PSY_TEXT(stimulus);
    PsyTextPrivate *priv = psy_text_get_instance_private(self);

    if (!psy_color_equal(psy_visual_stimulus_get_color(stimulus), color))
        priv->is_dirty = TRUE;

    PSY_VISUAL_STIMULUS_CLASS(psy_text_parent_class)
        ->set_color(stimulus, color);
}

static void
text_set_width(PsyRectangle *rect, gfloat width)
{
    PsyText        *self = PSY_TEXT(rect);
    PsyTextPrivate *priv = psy_text_get_instance_private(self);

    if (width != psy_rectangle_get_width(rect))
        priv->is_dirty = TRUE;

    PSY_RECTANGLE_CLASS(psy_text_parent_class)->set_width(rect, width);
}

static void
text_set_height(PsyRectangle *rect, gfloat height)
{
    PsyText        *self = PSY_TEXT(rect);
    PsyTextPrivate *priv = psy_text_get_instance_private(self);

    if (height != psy_rectangle_get_height(rect))
        priv->is_dirty = TRUE;

    PSY_RECTANGLE_CLASS(psy_text_parent_class)->set_height(rect, height);
}

static void
psy_text_class_init(PsyTextClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    object_class->get_property = text_get_property;
    object_class->set_property = text_set_property;

    PsyVisualStimulusClass *vstim_cls = PSY_VISUAL_STIMULUS_CLASS(klass);
    vstim_cls->create_artist          = text_create_artist;
    vstim_cls->set_color              = text_set_color;

    PsyRectangleClass *rect_cls = PSY_RECTANGLE_CLASS(klass);
    rect_cls->set_height        = text_set_height;
    rect_cls->set_width         = text_set_width;

    /**
     * Text:text
     *
     * This property may be used to set the content of the text and
     * [property@Psy.Text:use_markup] to false in one statement.
     */
    text_properties[PROP_TEXT]
        = g_param_spec_string("text",
                              "Text",
                              "The content as plain text",
                              "",
                              G_PARAM_WRITABLE | G_PARAM_CONSTRUCT);
    /**
     * Text:markup
     *
     * This property may be used to set the content of the text and
     * [property@Psy.Text:use_markup] to true in one statement.
     */
    text_properties[PROP_MARKUP]
        = g_param_spec_string("markup",
                              "Markup",
                              "The content text containing Pango markup",
                              "",
                              G_PARAM_WRITABLE);
    /**
     * Text:use-markup
     *
     * This property is used whether the content contains pango markup
     * language. It is generally no problem to specify content without pango
     * markup language and have this property to true, however, the opposite
     * is likely to be a problem, because the XML is also rendered as if it
     * were part of the content while this is most likely not intended.
     */
    text_properties[PROP_USE_MARKUP]
        = g_param_spec_boolean("use-markup",
                               "UseMarkup",
                               "Whether or not to use PangoMarkup language.",
                               FALSE,
                               G_PARAM_READWRITE);

    /**
     * Text:font-color:
     *
     * This property sets the font color.
     */
    text_properties[PROP_FONT_COLOR]
        = g_param_spec_object("font-color",
                              "FontColor",
                              "The color that is used for the letters, may be "
                              "overwritten by pango markup language",
                              PSY_TYPE_COLOR,
                              G_PARAM_READWRITE);

    /**
     * Text:is-dirty:
     *
     * This property indicates whether the [class@TextArtist] should update
     * the texture. If the user updates something that reflects changes in
     * the rectangle containing the text, the artist should update it's texture.
     * Generally this changes to TRUE when new, or something in the Rectangle/
     * Text changed.
     */
    text_properties[PROP_IS_DIRTY]
        = g_param_spec_boolean("is-dirty",
                               "IsDirty",
                               "Whether or not the texture should be updated.",
                               TRUE,
                               G_PARAM_READABLE);

    text_properties[PROP_FONT_FAM]
        = g_param_spec_string("font-family",
                              "FontFamily",
                              "The family name of the font",
                              "",
                              G_PARAM_READWRITE);

    g_object_class_install_properties(
        object_class, NUM_PROPERTIES, text_properties);
}

/**
 * psy_text_new:(constructor)
 * @canvas: an instance of [class@Canvas] on which this stimulus should be
 * drawn
 *
 * Returns: a new instance of [class@PsyText] with default values.
 */
PsyText *
psy_text_new(PsyCanvas *canvas)
{
    return g_object_new(PSY_TYPE_TEXT, "canvas", canvas, NULL);
}

/**
 * psy_text_new_full:(constructor)
 * @canvas:     The canvas on which we would like to draw this text
 * @x:          The x position of the center of the text
 * @y:          The y position of the center of the text
 * @width:      The width of the text (along y-axis)
 * @height:     The height of the text along(x-axis)
 * @content:    The content for the text use [property@Psy.Text:use_markup] to
 *              specify whether or not the content contains Pango markup
 * @use_markup: Whether or not the content contains PangoMarkup language.
 *
 * Construct a new Text stimulus for @canvas at position with coordinates @x and
 * @y the size of the rectangle containing the text is determined by @width and
 * @height Finally the @content parameter specifies the content of the text. By
 * default the [property@Psy.Text:use-markup] is false, so if the content
 * contains markup you should set that property to %TRUE.
 *
 * Returns: a new instance of [class@Text] with the provided values.
 */
PsyText *
psy_text_new_full(PsyCanvas   *canvas,
                  gfloat       x,
                  gfloat       y,
                  gfloat       width,
                  gfloat       height,
                  const gchar *text,
                  gboolean     use_markup)
{
    // clang-format off
    return g_object_new(
            PSY_TYPE_TEXT,
            "canvas", canvas,
            "x", x,
            "y", y,
            "width", width,
            "height", height,
            "text", text,
            "use-markup", use_markup,
            NULL);
    // clang-format on
}

/**
 * psy_text_set_content:
 * @self: an instance of [class@Psy.Text]
 * @content: A string to be rendered by this text object.
 *
 * Set the content of the stimulus. If [property@Psy.Text:use_markup] is
 * true, the string may contain PangoMarkup. Otherwise the XML markup
 * language is also displayed.
 */
void
psy_text_set_content(PsyText *self, const gchar *content)
{
    g_return_if_fail(PSY_IS_TEXT(self));
    g_return_if_fail(content != NULL);

    PsyTextPrivate *priv = psy_text_get_instance_private(self);

    g_clear_pointer(&priv->content, g_free);
    priv->content = g_strdup(content);
}

/**
 * psy_text_get_content:
 * @self: an instance of [class@Text]
 *
 * Returns the content of the text.
 */
const gchar *
psy_text_get_content(PsyText *self)
{
    g_return_val_if_fail(PSY_IS_TEXT(self), NULL);

    PsyTextPrivate *priv = psy_text_get_instance_private(self);
    return priv->content;
}

/**
 * psy_text_set_use_markup:
 * @self: an instance of [class@Psy.Text]
 * @use_markup: A boolean whether or not to parse the content for the pango
 *              markup language.
 *
 * Set the use_markup property, if true the content may contain the pango
 * markup language.
 */
void
psy_text_set_use_markup(PsyText *self, gboolean use_markup)
{
    g_return_if_fail(PSY_IS_TEXT(self));

    PsyTextPrivate *priv = psy_text_get_instance_private(self);

    priv->use_markup = use_markup;
}

/**
 * psy_text_get_use_markup:
 * @self: an instance of [class@Text]
 *
 * Returns: the value of [property:use_markup]
 */
gboolean
psy_text_get_use_markup(PsyText *self)
{
    g_return_val_if_fail(PSY_IS_TEXT(self), FALSE);

    PsyTextPrivate *priv = psy_text_get_instance_private(self);
    return priv->use_markup;
}

/**
 * psy_text_set_font_color:
 * @self: an instance of [class@Text]
 * @font_color: an instance of [class@Color] that is going to be used to
 * fill the background of the image
 *
 * This determines the default font color for the text to be created.
 */
void
psy_text_set_font_color(PsyText *self, PsyColor *font_color)
{
    g_return_if_fail(PSY_IS_TEXT(self));
    g_return_if_fail(PSY_IS_COLOR(font_color));

    PsyTextPrivate *priv = psy_text_get_instance_private(self);

    if (psy_color_equal(priv->font_color, font_color)) { // no reason to redraw.
        priv->is_dirty = TRUE;
    }

    g_clear_object(&priv->font_color);
    priv->font_color = g_object_ref(font_color);
}

/**
 * psy_text_get_font_color:
 * @self: an instance of [class@Text]
 *
 * This returns default font color for the text to be created. Using Pango
 * markup language it is possible to specify other color as well.
 *
 * Returns:(transfer none): The default color used for the text.
 */
PsyColor *
psy_text_get_font_color(PsyText *self)
{
    g_return_val_if_fail(PSY_IS_TEXT(self), NULL);

    PsyTextPrivate *priv = psy_text_get_instance_private(self);
    return priv->font_color;
}

/**
 * psy_text_get_is_dirty:
 * @self: an instance of psy text
 *
 * When some parameters of a PsyText instance change, the texture that was
 * previously stored is out of date. This indicates that [class@TextArtist]
 * that belongs to this stimulus should render the frame and upload it's
 * texture.
 *
 * Returns: %TRUE if parameters that indicate a redraw have changed. %FALSE
 *          otherwise.
 */
gboolean
psy_text_get_is_dirty(PsyText *self)
{
    g_return_val_if_fail(PSY_IS_TEXT(self), FALSE);

    PsyTextPrivate *priv = psy_text_get_instance_private(self);
    return priv->is_dirty;
}

/**
 * psy_text_set_is_dirty:(skip):
 * @self: an instance of [class@Text]
 *
 * When some parameters of a PsyText instance change, the texture that was
 * previously stored is out of date. This indicates that [class@TextArtist]
 * that belongs to this stimulus should render the frame and upload it's
 * texture.
 *
 * Stability(private):
 */
void
psy_text_set_is_dirty(PsyText *self, gboolean dirty)
{
    g_return_if_fail(PSY_IS_TEXT(self));

    PsyTextPrivate *priv = psy_text_get_instance_private(self);
    priv->is_dirty       = dirty;
}

/**
 * psy_text_get_font_description:(skip):
 * @self: an instance of [class@Text].
 *
 * Stability(private):
 * Returns: the font description of this text.
 */
PangoFontDescription *
psy_text_get_font_description(PsyText *self)
{
    g_return_val_if_fail(PSY_IS_TEXT(self), NULL);

    PsyTextPrivate *priv = psy_text_get_instance_private(self);
    return priv->font_description;
}

void
psy_text_set_font_family(PsyText *self, const gchar *font_fam)
{
    g_return_if_fail(PSY_IS_TEXT(self));
    g_return_if_fail(font_fam != NULL);

    PsyTextPrivate *priv = psy_text_get_instance_private(self);

    pango_font_description_set_family(priv->font_description, font_fam);
    priv->is_dirty = TRUE;
}

const gchar *
psy_text_get_font_family(PsyText *self)
{
    g_return_val_if_fail(PSY_IS_TEXT(self), NULL);

    PsyTextPrivate *priv = psy_text_get_instance_private(self);

    return pango_font_description_get_family(priv->font_description);
}

static void
psy_text_draw_stimulus(PsyText *text, PsyImage *img)
{
    PangoLayout  *layout  = NULL;
    PangoContext *context = NULL;

    g_return_if_fail(PSY_IS_TEXT(text) && PSY_IS_IMAGE(img));
    g_return_if_fail(psy_image_get_format(img) == PSY_IMAGE_FORMAT_RGBA);

    const gint width  = psy_image_get_width(img);
    const gint height = psy_image_get_height(img);

    PsyColor *bg_col = psy_visual_stimulus_get_color(PSY_VISUAL_STIMULUS(text));
    PsyColor *font_col = psy_text_get_font_color(text);

    cairo_surface_t *surf
        = cairo_image_surface_create(CAIRO_FORMAT_RGB24, width, height);
    g_assert(surf);

    cairo_t *cr = cairo_create(surf);

    // Draw background

    cairo_save(cr);

    cairo_set_source_rgba(cr,
                          psy_color_get_red(bg_col),
                          psy_color_get_green(bg_col),
                          psy_color_get_blue(bg_col),
                          psy_color_get_alpha(bg_col));
    cairo_rectangle(cr, 0, 0, width, height);
    cairo_fill(cr);

    cairo_restore(cr);

    // draw text

    layout  = pango_cairo_create_layout(cr);
    context = pango_layout_get_context(layout);

    pango_context_set_base_gravity(context, PANGO_GRAVITY_AUTO);
    g_print("width = %d    height = %d\n", width, height);
    pango_layout_set_width(layout, width * PANGO_SCALE);
    pango_layout_set_height(layout, height * PANGO_SCALE);
    g_print("width = %d    height = %d\n",
            pango_layout_get_width(layout),
            pango_layout_get_height(layout));

    pango_layout_set_font_description(layout,
                                      psy_text_get_font_description(text));

    if (psy_text_get_use_markup(text)) {
        pango_layout_set_markup(layout, psy_text_get_content(text), -1);
    }
    else {
        pango_layout_set_text(layout, psy_text_get_content(text), -1);
    }

    cairo_set_source_rgba(cr,
                          psy_color_get_red(font_col),
                          psy_color_get_green(font_col),
                          psy_color_get_blue(font_col),
                          psy_color_get_alpha(font_col));

    pango_cairo_show_layout(cr, layout);

    cairo_surface_flush(surf);

    const guint8 *surf_data = cairo_image_surface_get_data(surf);
    guint8       *img_data  = psy_image_get_ptr(img);

    for (gint r = 0; r < cairo_image_surface_get_height(surf); r++) {
        const guint8 *in_row
            = surf_data + r * cairo_image_surface_get_stride(surf);
        guint8 *out_row = img_data + r * psy_image_get_stride(img);
        for (gint c = 0; c < cairo_image_surface_get_width(surf); c++) {
            const guint8 *ipix        = in_row + c * 4;
            const guint  *cairo_pixel = (const guint *) ipix;
            guint8       *opix        = out_row + c * 4;

            // ARGB to RGBA
            opix[3] = (*cairo_pixel & CAIRO_ALPHA_MASK) >> 24;
            opix[0] = (*cairo_pixel & CAIRO_RED_MASK) >> 16;
            opix[1] = (*cairo_pixel & CAIRO_GREEN_MASK) >> 8;
            opix[2] = (*cairo_pixel & CAIRO_BLUE_MASK) >> 0;
        }
    }

    cairo_surface_destroy(surf);
    g_object_unref(layout);
}

/**
 * psy_text_create_stimulus:
 * @self: An instance of[class@Text]
 *
 * This creates an image of the stimulus. This function is used by the
 * PsyTextArtist in order to have a picture to draw as [class@Psy.Texture]
 * You may find it handy to save the resulting images to files. Then the
 * files can be loaded as an instance of [class@Image]. You should take care,
 * that the pixel format of these stimuli is CAIRO_ARGB.
 *
 * Returns:(transfer full): an image that represents the text.
 */
PsyImage *
psy_text_create_stimulus(PsyText *self)
{
    PsyClock     *clk;
    PsyTimePoint *tp0 = NULL, *tp1 = NULL;
    PsyDuration  *dur = NULL;
    clk               = psy_clock_new();
    tp0               = psy_clock_now(clk);

    g_return_val_if_fail(PSY_IS_TEXT(self), NULL);

    PsyImage *img = NULL;

    gfloat rect_width  = psy_rectangle_get_width(PSY_RECTANGLE(self));
    gfloat rect_height = psy_rectangle_get_height(PSY_RECTANGLE(self));

    PsyCanvas *canvas
        = psy_visual_stimulus_get_canvas(PSY_VISUAL_STIMULUS(self));

    gfloat canv_width  = psy_canvas_get_width(canvas);
    gfloat canv_height = psy_canvas_get_height(canvas);
    gint   style       = psy_canvas_get_projection_style(canvas);

    gfloat width_pu;  // n pixels per unit.
    gfloat height_pu; // n pixels per unit.

    if (style & PSY_CANVAS_PROJECTION_STYLE_PIXELS) {
        height_pu = width_pu = 1.0;
    }
    else if (style & PSY_CANVAS_PROJECTION_STYLE_METER) {
        gfloat wm = psy_canvas_get_width_mm(canvas) / 1000.0;
        gfloat hm = psy_canvas_get_height_mm(canvas) / 1000.0;
        width_pu  = wm / canv_width;
        height_pu = hm / canv_height;
    }
    else if (style & PSY_CANVAS_PROJECTION_STYLE_MILLIMETER) {
        gint wmm  = psy_canvas_get_width_mm(canvas);
        gint hmm  = psy_canvas_get_height_mm(canvas);
        width_pu  = wmm / canv_width;
        height_pu = hmm / canv_height;
    }
    else if (style & PSY_CANVAS_PROJECTION_STYLE_VISUAL_DEGREES) {
        gfloat wvd = psy_canvas_get_width_vd(canvas);
        gfloat hvd = psy_canvas_get_height_vd(canvas);
        width_pu   = wvd / canv_width;
        height_pu  = hvd / canv_height;
    }
    else {
        // Forgotten or invalid projection style?
        g_assert_not_reached();
    }

    gint img_width  = (gint) round(rect_width * width_pu);
    gint img_height = (gint) round(rect_height * height_pu);

    if (img_width < 0 || img_height < 0) {
        g_critical(
            "creating an image with dimensions %d * %d", img_width, img_height);
        return NULL;
    }

    img = psy_image_new(
        (guint) img_width, (guint) img_height, PSY_IMAGE_FORMAT_RGBA);

    psy_text_draw_stimulus(self, img);

    tp1 = psy_clock_now(clk);
    dur = psy_time_point_subtract(tp1, tp0);

    g_print("Rendering text stimulus takes %lfs.\n",
            psy_duration_get_seconds(dur));

    g_object_unref(dur);
    g_object_unref(tp1);
    g_object_unref(tp0);
    g_object_unref(clk);

    return img;
}
