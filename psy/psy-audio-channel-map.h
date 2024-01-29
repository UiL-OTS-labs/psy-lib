
#pragma once

#include <gio/gio.h>
#include <glib-object.h>

#include "psy-enums.h"

G_BEGIN_DECLS

#define PSY_TYPE_AUDIO_CHANNEL_MAPPING (psy_audio_channel_mapping_get_type())

/**
 * PsyAudioChannelMapping:
 * @sink_channel: the output channel for which we would like to mix an input
 *    The value should be regarded as an 0-based index, hence a value
 *    of 0 will map to the first channel.
 * @mapped_source: The channel of the source which we'll link to the sink
 *    channel described by the @sink_channel
 *
 * This is a mapping that links a single source channel to a single
 * sink(output) channel.
 */
typedef struct {
    gint sink_channel;
    gint mapped_source;
} PsyAudioChannelMapping;

G_MODULE_EXPORT GType
psy_audio_channel_mapping_get_type(void);

G_MODULE_EXPORT PsyAudioChannelMapping *
psy_audio_channel_mapping_new(gint sink_channel, gint source_channel);

G_MODULE_EXPORT void
psy_audio_channel_mapping_free(PsyAudioChannelMapping *self);

G_MODULE_EXPORT PsyAudioChannelMapping *
psy_audio_channel_mapping_copy(PsyAudioChannelMapping *self);

G_MODULE_EXPORT gboolean
psy_audio_channel_mapping_eq(PsyAudioChannelMapping *self,
                             PsyAudioChannelMapping *other);

#define PSY_TYPE_AUDIO_CHANNEL_MAP (psy_audio_channel_map_get_type())

/**
 * PsyAudioChannelMap:
 * @mapping: The mapping between the output
 *    channel and the input channel.
 * @num_source_channels: the number of input channels for this operation
 *    e.g. the number of channels of a PsyAuditoryStimulus or input (caputure)
 *    channels of a PsyAudioDevice.
 * @num_sink_channels: the number of output channels for this operation, e.g.
 *    the number of playback channels on a output device.
 *
 * A PsyAudioChannelMap is used to map the source (input) channels to the
 * sink (output) channels of an operation. So psylib knows where to map the
 * input channels on an output channel when conducting an operation on audio
 * streams.
 */
typedef struct {
    /* <private> */
    GPtrArray *mapping;
    /* <public> */
    PsyAudioChannelStrategy strategy;
    guint                   num_source_channels;
    guint                   num_sink_channels;
} PsyAudioChannelMap;

G_MODULE_EXPORT GType
psy_audio_channel_map_get_type(void);

G_MODULE_EXPORT PsyAudioChannelMap *
psy_audio_channel_map_copy(PsyAudioChannelMap *self);

G_MODULE_EXPORT void
psy_audio_channel_map_free(PsyAudioChannelMap *self);

G_MODULE_EXPORT PsyAudioChannelMap *
psy_audio_channel_map_new(guint num_sink_channels, guint num_source_channels);

G_MODULE_EXPORT PsyAudioChannelMap *
psy_audio_channel_map_new_strategy(guint                   num_sink_channels,
                                   guint                   num_source_channels,
                                   PsyAudioChannelStrategy strategy);

G_MODULE_EXPORT void
psy_audio_channel_map_set_strategy(PsyAudioChannelMap     *self,
                                   PsyAudioChannelStrategy strategy);

G_MODULE_EXPORT guint
psy_audio_channel_map_get_size(PsyAudioChannelMap *self);

G_MODULE_EXPORT void
psy_audio_channel_map_set_size(PsyAudioChannelMap *self, guint size);

G_MODULE_EXPORT gboolean
psy_audio_channel_map_add(PsyAudioChannelMap     *self,
                          PsyAudioChannelMapping *mapping);

G_MODULE_EXPORT gboolean
psy_audio_channel_map_set(PsyAudioChannelMap     *self,
                          guint                   index,
                          PsyAudioChannelMapping *mapping);

G_MODULE_EXPORT PsyAudioChannelMapping *
psy_audio_channel_map_get_mapping(PsyAudioChannelMap *self, guint index);

G_END_DECLS
