/**
 * PsyArtist:
 *
 * Instances of `PsyArtist` are the objects that are finally responsible
 * that Instances of `PsyVisualStimulus` are drawn to a canvas. The
 * canvas will be the surface of a window, or theoretically an other type of
 * buffer (in memory or file e.g.) To which this specific artist is drawing.
 *
 * The difference between a [class@VisualStimulus] and an [class@Artist] is
 * that the stimulus, contains the parameters about how, when and where a
 * stimulus should be presented, whereas an Artist is the one that is actually
 * doing the work of converting those parameters to the drawing on the canvas.
 * You could see it as the that a `PsyStimulus` is the set of instructions that
 * a `PsyArtist` gets in order to draw its work of art.
 *
 * So a `PsyVisualStimulus` is a kind of a general description of a stimulus,
 * whereas a `PsyArtist` knows how to transform these descriptions to the actual
 * sculpture, painting, etc. In this analogy, One artist might be able to sculpt
 * OpenGL style of stimuli, whereas an other artist knows Vulcan or Direct3D.
 */

#include <epoxy/gl.h>

#include "psy-artist.h"
#include "psy-canvas.h"
#include "psy-matrix4.h"
#include "psy-vector3.h"
#include "psy-visual-stimulus.h"

typedef struct _PsyArtistPrivate {
    PsyCanvas         *canvas;
    PsyDrawingContext *context;
    PsyVisualStimulus *stimulus;
    PsyMatrix4        *model;
} PsyArtistPrivate;

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE(PsyArtist, psy_artist, G_TYPE_OBJECT)

typedef enum {
    PROP_NULL, // not used required by GObject
    PROP_CANVAS,
    PROP_STIMULUS,
    NUM_PROPERTIES
} ArtistProperty;

static GParamSpec *artist_properties[NUM_PROPERTIES] = {0};

static void
artist_dispose(GObject *object)
{
    PsyArtistPrivate *priv
        = psy_artist_get_instance_private(PSY_ARTIST(object));

    g_clear_object(&priv->canvas);
    g_clear_object(&priv->stimulus);
    g_clear_object(&priv->model);
    g_clear_object(&priv->context);

    G_OBJECT_CLASS(psy_artist_parent_class)->dispose(object);
}

static void
artist_constructed(GObject *obj)
{
    PsyArtist        *self = PSY_ARTIST(obj);
    PsyArtistPrivate *priv = psy_artist_get_instance_private(self);

    priv->context = psy_canvas_get_context(priv->canvas);
    g_object_ref(priv->context);

    G_OBJECT_CLASS(psy_artist_parent_class)->constructed(obj);
}

static void
artist_set_property(GObject      *object,
                    guint         property_id,
                    const GValue *value,
                    GParamSpec   *pspec)
{
    PsyArtist *self = PSY_ARTIST(object);

    switch ((ArtistProperty) property_id) {
    case PROP_CANVAS:
        psy_artist_set_canvas(self, g_value_get_object(value));
        break;
    case PROP_STIMULUS:
        psy_artist_set_stimulus(self, g_value_get_object(value));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

static void
artist_get_property(GObject    *object,
                    guint       property_id,
                    GValue     *value,
                    GParamSpec *pspec)
{
    PsyArtist        *self = PSY_ARTIST(object);
    PsyArtistPrivate *priv = psy_artist_get_instance_private(self);

    switch ((ArtistProperty) property_id) {
    case PROP_CANVAS:
        g_value_set_object(value, priv->canvas);
        break;
    case PROP_STIMULUS:
        g_value_set_object(value, priv->stimulus);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

static PsyProgram *
artist_get_program(PsyArtist *self)
{
    PsyArtistPrivate *priv = psy_artist_get_instance_private(self);
    return psy_drawing_context_get_program(priv->context,
                                           PSY_UNIFORM_COLOR_PROGRAM_NAME);
}

static void
psy_artist_init(PsyArtist *self)
{
    PsyArtistPrivate *priv = psy_artist_get_instance_private(self);

    priv->model = psy_matrix4_new_identity();
}

static void
artist_draw(PsyArtist *self)
{
    PsyArtistPrivate *priv       = psy_artist_get_instance_private(self);
    PsyProgram       *program    = psy_artist_get_program(self);
    GError           *error      = NULL;
    const gchar      *model_name = "model";

    gfloat x = psy_visual_stimulus_get_x(priv->stimulus);
    gfloat y = psy_visual_stimulus_get_y(priv->stimulus);
    gfloat z = -psy_visual_stimulus_get_z(priv->stimulus);

    PsyVector3 *translation
        = g_object_new(PSY_TYPE_VECTOR3, "x", x, "y", y, "z", z, NULL);

    gfloat scale_x = psy_visual_stimulus_get_scale_x(priv->stimulus);
    gfloat scale_y = psy_visual_stimulus_get_scale_y(priv->stimulus);

    PsyVector3 *scale_vec
        = g_object_new(PSY_TYPE_VECTOR3, "x", scale_x, "y", scale_y, NULL);

    gfloat rotation_degrees = psy_visual_stimulus_get_rotation(priv->stimulus);
    PsyVector3 *z_axis
        = g_object_new(PSY_TYPE_VECTOR3, "x", 0.0, "y", 0.0, "z", 1.0, NULL);

    psy_matrix4_set_identity(priv->model);

    psy_matrix4_translate(priv->model, translation);
    psy_matrix4_rotate(priv->model, rotation_degrees, z_axis);
    psy_matrix4_scale(priv->model, scale_vec);

    psy_program_use(psy_artist_get_program(self), &error);
    if (error) {
        g_critical("Unable to use program: %s", error->message);
        g_clear_error(&error);
    }
    else {
        psy_program_set_uniform_matrix4(
            program, model_name, priv->model, &error);
        if (error) {
            static int once = 0;
            if (!once) {
                once = 1;
                g_critical(
                    "Unable to set the %s matrix: '%s'. "
                    "This warning is emitted only once. "
                    "The Artist.draw expects the shader program to define "
                    "a 4*4 matrix that handles the model transformations",
                    model_name,
                    error->message);
                g_error_free(error);
                error = NULL;
            }
        }
    }

    g_object_unref(translation);
    g_object_unref(scale_vec);
    g_object_unref(z_axis);
}

static void
psy_artist_class_init(PsyArtistClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    object_class->get_property = artist_get_property;
    object_class->set_property = artist_set_property;
    object_class->dispose      = artist_dispose;
    object_class->constructed  = artist_constructed;

    klass->draw        = artist_draw;
    klass->get_program = artist_get_program;

    /**
     * Artist:stimulus:
     *
     * This is the `PsyVisualStimulus` that this instance of PsyArtist
     * is responsible of drawing.
     */
    artist_properties[PROP_STIMULUS]
        = g_param_spec_object("stimulus",
                              "Stimulus",
                              "The stimulus that this artist is going to draw.",
                              PSY_TYPE_VISUAL_STIMULUS,
                              G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
    /**
     * Artist:canvas:
     *
     * This is the [class@Canvas] on which this instance of PsyArtist is
     * drawing.
     */
    artist_properties[PROP_CANVAS]
        = g_param_spec_object("canvas",
                              "Canvas",
                              "The canvas on which this artist is going to act",
                              PSY_TYPE_CANVAS,
                              G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

    g_object_class_install_properties(
        object_class, NUM_PROPERTIES, artist_properties);
}

/**
 * psy_artist_set_stimulus:
 * @self: an instance of `PsyArtist`
 * @stimulus: an instance of `PsyVisualStimulus`
 *
 * set the contained object.
 */
void
psy_artist_set_stimulus(PsyArtist *self, PsyVisualStimulus *stimulus)
{
    g_return_if_fail(PSY_IS_ARTIST(self));
    g_return_if_fail(PSY_IS_VISUAL_STIMULUS(stimulus));
    PsyArtistPrivate *priv = psy_artist_get_instance_private(self);

    if (priv->stimulus) {
        g_object_unref(priv->stimulus);
    }

    priv->stimulus = g_object_ref(stimulus);
}

/**
 * psy_artist_get_stimulus:
 * @self: An instance of `PsyArtist`
 *
 * Returns:(transfer none): an instance of `PsyVisualStimulus`
 */
PsyVisualStimulus *
psy_artist_get_stimulus(PsyArtist *self)
{
    g_return_val_if_fail(PSY_IS_ARTIST(self), NULL);
    PsyArtistPrivate *priv = psy_artist_get_instance_private(self);

    return priv->stimulus;
}

/**
 * psy_artist_set_canvas:
 * @self: an instance of `PsyArtist`
 * @canvas: an instance of `PsyCanvas`
 *
 * set the contained object.
 */
void
psy_artist_set_canvas(PsyArtist *self, PsyCanvas *canvas)
{
    g_return_if_fail(PSY_IS_ARTIST(self));
    g_return_if_fail(PSY_IS_CANVAS(canvas));
    PsyArtistPrivate *priv = psy_artist_get_instance_private(self);

    if (priv->canvas) {
        g_object_unref(priv->canvas);
    }

    priv->canvas = g_object_ref(canvas);
}

/**
 * psy_artist_get_canvas:
 * @self: An instance of `PsyArtist`
 *
 * Returns:(transfer none): an instance of `PsyCanvas`
 */
PsyCanvas *
psy_artist_get_canvas(PsyArtist *self)
{
    g_return_val_if_fail(PSY_IS_ARTIST(self), NULL);
    PsyArtistPrivate *priv = psy_artist_get_instance_private(self);

    return priv->canvas;
}

/**
 * psy_artist_draw:
 * @self: an instance of `PsyArtist`
 *
 * This is the function that draws the stimulus
 */
void
psy_artist_draw(PsyArtist *self)
{
    g_return_if_fail(PSY_IS_ARTIST(self));

    PsyArtistClass *klass = PSY_ARTIST_GET_CLASS(self);
    g_return_if_fail(klass->draw);

    klass->draw(self);
}

/**
 * psy_artist_get_program:
 * @self: an instance of `PsyArtist`
 *
 * This function calls the virtual get_shader_program function from the
 * class. It is designed to fetch a default shader, or deriving class,
 * might want a shader of there own. Deriving class may choose to use
 * for there own drawing.
 *
 * Returns:(transfer none): The shader for this object, deriving classes
 * may pick a shader to there suiting.
 */
PsyProgram *
psy_artist_get_program(PsyArtist *self)
{
    g_return_val_if_fail(PSY_IS_ARTIST(self), NULL);
    PsyArtistClass *cls = PSY_ARTIST_GET_CLASS(self);

    g_return_val_if_fail(cls->get_program, NULL);
    return cls->get_program(self);
}

/**
 * psy_artist_get_context:
 * @self: an instance of `PsyArtist`
 *
 * This function returns the drawing context for this artist, it is
 * mainly intended for deriving classes to get a reference to the
 * context.
 *
 * Returns:(transfer none): the instance of [class@DrawingContext] that
 * belongs to this visual stimulus.
 */
PsyDrawingContext *
psy_artist_get_context(PsyArtist *self)
{
    PsyArtistPrivate *priv = psy_artist_get_instance_private(self);

    g_return_val_if_fail(PSY_IS_ARTIST(self), NULL);

    return priv->context;
}
