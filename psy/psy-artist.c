
/**
 * PsyArtist:
 *
 * Instances of `PsyArtist` are the objects that are finally responsible
 * that Instances of `PsyVisualStimulus` are drawn to a canvas. The
 * canvas will be the surface of a window, or theoretically an other type of
 * buffer (in memory or file e.g.) To which this specific artist is drawing.
 *
 * The difference between a `PsyVisualStimulus` and an `PsyArtist` is that the
 * stimulus, contains the parameters about how, when and where a stimulus should
 * be presented, whereas an Artist is the one that is actually doing the work of
 * converting those parameters to the drawing on the canvas. You could see it
 * as the that a `PsyStimulus` is the set of instructions that a `PsyArtist` gets
 * in order to draw its work of art.
 *
 * So a `PsyVisualStimulus` is a kind of a general description of a stimulus,
 * whereas a `PsyArtist` knows how to transform these descriptions to the actual
 * sculpture, painting, etc. In this analogy, One artist might be able to sculpt
 * OpenGL style of stimuli, whereas an other artist knows Vulcan or Direct3D.
 */

#include "psy-artist.h"

typedef struct _PsyArtistPrivate {
    PsyVisualStimulus* stimulus;
} PsyArtistPrivate;

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE(
        PsyArtist, psy_artist, G_TYPE_OBJECT
        )

typedef enum {
    PROP_NULL,          // not used required by GObject
    PROP_STIMULUS,
    NUM_PROPERTIES
} ArtistProperty;

static GParamSpec*  artist_properties[NUM_PROPERTIES] = {0};

static void
artist_set_property(GObject       *object,
                    guint          property_id,
                    const GValue  *value,
                    GParamSpec    *pspec
                    )
{
    PsyArtist* self = PSY_ARTIST(object);

    switch((ArtistProperty) property_id) {
        case PROP_STIMULUS:
            psy_artist_set_stimulus(self, g_value_get_object(value));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

static void
artist_get_property(GObject       *object,
                    guint          property_id,
                    GValue        *value,
                    GParamSpec    *pspec
                    )
{
    PsyArtist* self = PSY_ARTIST(object);
    PsyArtistPrivate* priv = psy_artist_get_instance_private(self);

    switch((ArtistProperty) property_id) {
        case PROP_STIMULUS:
            g_value_set_object(value, priv->stimulus);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

static void
psy_artist_init(PsyArtist* self)
{
    (void) self;
}

static void
psy_artist_class_init(PsyArtistClass* klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    object_class->get_property = artist_get_property;
    object_class->set_property = artist_set_property;

    /**
     * Artist:stimulus:
     *
     * This is the `PsyVisualStimulus` that this instance of PsyArtist is
     * responsible of drawing.
     */
    artist_properties[PROP_STIMULUS] = g_param_spec_object(
            "stimulus",
            "Stimulus",
            "The stimulus that this artist is going to draw.",
            PSY_TYPE_VISUAL_STIMULUS,
            G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY
            );

    g_object_class_install_properties(
            object_class, NUM_PROPERTIES, artist_properties 
            );
}

/**
 * psy_artist_set_stimulus:
 * @self: an instance of `PsyArtist` 
 * @stimulus: an instance of `PsyVisualStimulus`
 *
 * set the contained object.
 */
void
psy_artist_set_stimulus(PsyArtist* self, PsyVisualStimulus* stimulus)
{
    g_return_if_fail(PSY_IS_ARTIST(self));
    PsyArtistPrivate* priv = psy_artist_get_instance_private(self);

    if (priv->stimulus) {
        g_object_unref(priv->stimulus);
    }

    priv->stimulus = g_object_ref(stimulus);
}

/**
 * psy_artist_get_stimulus:
 * @self: An instance of `PsyArtist`
 *
 * Returns:(tranfer none): an instance of `PsyVisualStimulus`
 */
PsyVisualStimulus*
psy_artist_get_stimulus(PsyArtist* self)
{
    g_return_val_if_fail(PSY_IS_ARTIST(self), NULL);
    PsyArtistPrivate* priv = psy_artist_get_instance_private(self);

    return priv->stimulus;
}

