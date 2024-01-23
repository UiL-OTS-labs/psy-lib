
#include "psy-audio-channel-map.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"

G_DEFINE_BOXED_TYPE(PsyAudioChannelMapping,
                    psy_audio_channel_mapping,
                    psy_audio_channel_mapping_dup,
                    psy_audio_channel_mapping_free)

G_DEFINE_BOXED_TYPE(PsyAudioChannelMap,
                    psy_audio_channel_map,
                    psy_audio_channel_map_dup,
                    psy_audio_channel_map_free)

#pragma GCC diagnostic pop

/**
 * psy_audio_channel_mapping_new:
 * sink_channel: the channel of the sink is valid for the range[0, G_MAXINT]
 * source_channel: the source channel to be mapped to the sink_channel
 *    the valid range is [0, G_MAXINT]
 *
 * Create a new channel mapping for a sink channel and an source channel
 * the source channel provide the input for an operation and the sink is an
 * output for the operation. This allows for very large channel numbers,
 * however note that a PsyAudioChannelMap will not allow to add/set
 * PsyAudioChannelMappings that contain values larger than their
 * num_source_channels, num_sink_channels.
 */
PsyAudioChannelMapping *
psy_audio_channel_mapping_new(gint sink_channel, gint source_channel)
{
    g_return_val_if_fail(sink_channel >= 0, NULL);
    g_return_val_if_fail(source_channel >= 0, NULL);
    PsyAudioChannelMapping *new = g_malloc(sizeof(PsyAudioChannelMapping));
    new->sink_channel           = sink_channel;
    new->mapped_source          = source_channel;
    return new;
}

/**
 * psy_audio_channel_mapping_free:
 *
 * frees an mapping previously allocated with psy_audio_channel_mapping_new.
 */
void
psy_audio_channel_mapping_free(PsyAudioChannelMapping *self)
{
    g_free(self);
}

/**
 * psy_audio_channel_mapping_dup:
 * @self: an instance of [struct@PsyAudioChannelMapping]
 *
 * Create a deep copy of a mapping
 *
 * Returns:(transfer full): a new instance of [struct@PsyAudioChannelMapping]
 */
PsyAudioChannelMapping *
psy_audio_channel_mapping_dup(PsyAudioChannelMapping *self)
{
    g_return_val_if_fail(self != NULL, FALSE);

    PsyAudioChannelMapping *new = g_malloc(sizeof(PsyAudioChannelMapping));
    new->sink_channel           = self->sink_channel;
    new->mapped_source          = self->mapped_source;
    return new;
}

/**
 * psy_audio_channel_mapping_eq:
 * @self: The left hand side of the equation
 * @other: The right hand side of the equation
 *
 * check whether self == other by comparison
 *
 * Returns: TRUE when the mapping self is equal to the other mapping. FALSE
 * otherwise
 */
gboolean
psy_audio_channel_mapping_eq(PsyAudioChannelMapping *self,
                             PsyAudioChannelMapping *other)
{
    g_return_val_if_fail(self != NULL, FALSE);
    g_return_val_if_fail(other != NULL, FALSE);

    if (self == other) {
        return TRUE;
    }
    else {
        return self->mapped_source == other->mapped_source
               && self->sink_channel == other->sink_channel;
    }
}

/**
 * psy_audio_channel_mapping_ne:
 * @self: The left hand side of the equation
 * @other: The right hand side of the equation
 *
 * Check whether self != other by comparison
 *
 * Returns: TRUE when the mapping self is NOT equal to the other mapping. False
 * otherwise
 */
gboolean
psy_audio_channel_mapping_ne(PsyAudioChannelMapping *self,
                             PsyAudioChannelMapping *other)
{
    return !psy_audio_channel_mapping_eq(self, other);
}

/* ************* PsyAudioChannelMap ********************* */

/**
 * psy_audio_channel_map_new:(constructor)
 * @num_sink_channels: The number of channels of the sink
 *  0 < num_sink_channels < %G_MAXINT
 * @num_source_channels: The number of channels of the source
 *  0 < num_source_channels < G_MAXINT
 *
 * Construct an empty map, you'll have to add mappings yourself.
 *
 * Returns:(transfer full): A new instance of [struct@PsyAudioChannelMap]
 */
PsyAudioChannelMap *
psy_audio_channel_map_new(guint num_sink_channels, guint num_source_channels)
{
    g_return_val_if_fail(
        num_source_channels > 0 && num_source_channels < G_MAXINT, NULL);
    g_return_val_if_fail(num_sink_channels > 0 && num_sink_channels < G_MAXINT,
                         NULL);

    PsyAudioChannelMap *new = g_malloc(sizeof(PsyAudioChannelMap));

    new->mapping = g_ptr_array_new_full(
        0, (GDestroyNotify) &psy_audio_channel_mapping_free);

    new->num_source_channels = num_source_channels;
    new->num_sink_channels   = num_sink_channels;
    new->strategy            = PSY_AUDIO_CHANNEL_STRATEGY_CUSTOM;

    return new;
}

/**
 * psy_audio_channel_map_new_strategy:(constructor)
 * @num_sink_channels: The number of channels of the sink
 *  0 < num_sink_channels < %G_MAXINT
 * @num_source_channels: The number of channels of the source
 *  0 < num_source_channels < G_MAXINT
 * @strategy: The strategy used to initialize the mapping, You shouldn't set the
 *            PSY_AUDIO_CHANNEL_STRATEGY_CUSTOM bit, as it is ignored.
 *
 * Construct a new instance of [struct@AudioChannelMap]. The channels are
 * organized according the chosen strategy
 *
 * Returns:(transfer full): A new instance of [struct@PsyAudioChannelMap]
 */
PsyAudioChannelMap *
psy_audio_channel_map_new_strategy(guint                   num_sink_channels,
                                   guint                   num_source_channels,
                                   PsyAudioChannelStrategy strategy)
{
    PsyAudioChannelMap *new
        = psy_audio_channel_map_new(num_sink_channels, num_source_channels);

    if (!new)
        return NULL;

    psy_audio_channel_map_set_strategy(new, strategy);

    return new;
}

/**
 * psy_audio_channel_map_free:
 * @self: An instance of [struct@AudioChannelMap] previously allocated with:
 *        psy_audio_channel_map_new*.
 *
 * Frees an previously constructed instance.
 */
void
psy_audio_channel_map_free(PsyAudioChannelMap *self)
{
    g_ptr_array_free(self->mapping, TRUE);
    g_free(self);
}

static gpointer
copy_mapping(gconstpointer src, gpointer data)
{
    (void) data;
    PsyAudioChannelMapping *original = (PsyAudioChannelMapping *) src;
    return psy_audio_channel_mapping_dup(original);
}

/**
 * psy_audio_channel_map_dup:
 * @self: the instance of [struct@AudioChannelMap] to copy
 *
 * Makes a deep copy of @self.
 *
 * Returns: A deep copy of self.
 */
PsyAudioChannelMap *
psy_audio_channel_map_dup(PsyAudioChannelMap *self)
{
    PsyAudioChannelMap *new = g_malloc(sizeof(PsyAudioChannelMap));

    new->mapping = g_ptr_array_copy(self->mapping, copy_mapping, NULL);
    new->num_sink_channels   = self->num_sink_channels;
    new->num_source_channels = self->num_sink_channels;
    new->strategy            = self->strategy;

    return new;
}

/**
 * psy_audio_channel_map_set_strategy:
 * @self: an instance of [struct@AudioChannelMap]
 * @strategy: the stategy to implement the used mappings
 *
 * This functions clears the existing mapping and applies the mappings
 * according to the strategy.
 */
void
psy_audio_channel_map_set_strategy(PsyAudioChannelMap     *self,
                                   PsyAudioChannelStrategy strategy)
{
    const gint ninputs  = (gint) self->num_source_channels;
    const gint noutputs = (gint) self->num_sink_channels;

    g_assert(ninputs > 0);
    g_assert(noutputs > 0);

    if (G_UNLIKELY(strategy & PSY_AUDIO_CHANNEL_STRATEGY_CUSTOM)) {
        g_warning("PSY_AUDIO_CHANNEL_STRATEGY_CUSTOM bit is ignored");
    }

    g_ptr_array_set_size(self->mapping, 0);

    // Default channel assignment
    for (gint i = 0; i < ninputs && i < noutputs; i++) {
        g_ptr_array_add(self->mapping, psy_audio_channel_mapping_new(i, i));
    }

    // Duplicate input channels
    if ((ninputs < noutputs)
        && ((strategy & PSY_AUDIO_CHANNEL_STRATEGY_DUPLICATE_INPUTS) != 0)) {
        for (gint i = ninputs; i < noutputs; i++) {
            g_assert(i < noutputs);
            g_ptr_array_add(self->mapping,
                            psy_audio_channel_mapping_new(i, i % ninputs));
        }
    }

    // Map/mix inputs to outputs that already have another mapping
    if ((noutputs < ninputs)
        && (strategy & PSY_AUDIO_CHANNEL_STRATEGY_MIX_TRAILING_INPUTS) != 0) {
        for (gint i = noutputs; i < ninputs; i++) {
            g_ptr_array_add(self->mapping,
                            psy_audio_channel_mapping_new(i % noutputs, i));
        }
    }

    // Remove the PSY_AUDIO_CHANNEL_STRATEGY_CUSTOM flag
    self->strategy = strategy
                     & (PSY_AUDIO_CHANNEL_STRATEGY_DUPLICATE_INPUTS
                        | PSY_AUDIO_CHANNEL_STRATEGY_MIX_TRAILING_INPUTS);
}

/**
 * psy_audio_channel_map_get_size:
 * @self: an instnance of [struct@PsyAudioChannelMap]
 *
 * Get the number of mappings stored in this instance of
 * [struct@PsyAudioChannelMap]
 *
 * Returns: an integer representing the number of mappings store in @self.
 */
guint
psy_audio_channel_map_get_size(PsyAudioChannelMap *self)
{
    g_return_val_if_fail(self != NULL, 0);

    return self->mapping->len;
}

/**
 * psy_audio_channel_map_set_size:
 * @self: an instance of [struct@PsyAudioChannelMap]
 * @size: The desired size of the number of mapping that may be set.
 *
 * Set the number of mappings stored in this instance of
 * [struct@PsyAudioChannelMap], if the size shrinks, mappings will be deleted
 * if the size is greater than before, NULL/None items are added, which
 * should be set before use.
 */
void
psy_audio_channel_map_set_size(PsyAudioChannelMap *self, guint size)
{
    g_return_if_fail(self != NULL);
    g_return_if_fail(size < G_MAXINT);

    g_ptr_array_set_size(self->mapping, (gint) size);
}

/**
 * psy_audio_channel_map_get_mapping:
 * @self: an instance of of [struct@PsyAudioChannelMap]
 * @index: the index of the mapping should be in the range:
 *       0 <= index < psy_audio_channel_get_size(@self)
 *
 * Returns the mapping at the specified index. Make sure the index
 * is in the valid range. Thus function may return null when the
 * index is wrong, in which case a critical message should be emitted, but
 * may also occur when the size is change but not for all indices a mapping
 * has been set.
 *
 * Returns:(transfer full)(nullable): A copy of a ChannelMapping
 */
PsyAudioChannelMapping *
psy_audio_channel_map_get_mapping(PsyAudioChannelMap *self, guint index)
{
    g_return_val_if_fail(self != NULL, NULL);
    g_return_val_if_fail(index < self->mapping->len, NULL);

    PsyAudioChannelMapping *original = self->mapping->pdata[index];

    return psy_audio_channel_mapping_dup(original);
}

/**
 * psy_audio_channel_map_add:
 * @self: an instance of [struct@PsyAudioChannelMap
 * @mapping:(transfer none): An instance of [struct@PsyAudioChannelMapping]
 *
 * Add a new mapping to the map.
 */
gboolean
psy_audio_channel_map_add(PsyAudioChannelMap     *self,
                          PsyAudioChannelMapping *mapping)
{
    g_return_val_if_fail(self != NULL, FALSE);
    g_return_val_if_fail(mapping != NULL, FALSE);

    g_return_val_if_fail(
        mapping->sink_channel >= 0 && mapping->mapped_source >= 0, FALSE);
    g_return_val_if_fail(mapping->sink_channel < (gint) self->num_sink_channels,
                         FALSE);
    g_return_val_if_fail(
        mapping->mapped_source < (gint) self->num_source_channels, FALSE);

    g_ptr_array_add(self->mapping, psy_audio_channel_mapping_dup(mapping));

    return TRUE;
}

/**
 * psy_audio_channel_map_set:
 * @self: an instance of [struct@AudioChannelMap] to which to add a mapping from
 *      a sound source to a sound sink
 * @index: a 0-based index that should be smaller than the size of @self index
 *         should be smaller than or equal to G_MAXINT.
 * @mapping:(transfer none): a mapping that is valid for @self
 *     eg mapping->mapped_source < self->num_source_channels and
 *     mapping->sink_channel < self->num_source_channels.
 *
 * Clears an existing mapping from @self and adds @mapping to the map.
 *
 * Returns: TRUE when everything works out, FALSE when the index or mapping
 *          isn't valid.
 */
gboolean
psy_audio_channel_map_set(PsyAudioChannelMap     *self,
                          guint                   index,
                          PsyAudioChannelMapping *mapping)
{
    g_return_val_if_fail(self != NULL, FALSE);
    g_return_val_if_fail(mapping != NULL, FALSE);
    g_return_val_if_fail(index < self->mapping->len, FALSE);
    g_return_val_if_fail(index < G_MAXINT, FALSE);

    g_return_val_if_fail(
        mapping->mapped_source < (gint) self->num_source_channels, FALSE);
    g_return_val_if_fail(mapping->sink_channel < (gint) self->num_sink_channels,
                         FALSE);

    psy_audio_channel_mapping_free(self->mapping->pdata[index]);
    self->mapping->pdata[index] = psy_audio_channel_mapping_dup(mapping);
    return TRUE;
}
