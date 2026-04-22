

#include <criterion/criterion.h>
#include <criterion/new/assert.h>
#include <psylib.h>

Test(audio_channel, mapping)
{
    const int source = 2;
    const int sink   = 1;

    PsyAudioChannelMapping *mapping
        = psy_audio_channel_mapping_new(sink, source);

    cr_assert(ne(mapping, NULL));

    cr_expect(eq(mapping->mapped_source, source));
    cr_expect(eq(mapping->sink_channel, sink));

    psy_audio_channel_mapping_free(mapping);
}

Test(audio_channel, map)
{
    guint num_sinks   = 2;
    guint num_sources = 2;

    PsyAudioChannelMap *map = psy_audio_channel_map_new(num_sinks, num_sources);

    cr_assert(ne(map, NULL));

    cr_expect(eq(u32, psy_audio_channel_map_get_size(map), 0));
    cr_expect(eq(u32, map->strategy, PSY_AUDIO_CHANNEL_STRATEGY_CUSTOM));
    cr_expect(eq(u32, map->num_sink_channels, 2));
    cr_expect(eq(u32, map->num_source_channels, 2));

    psy_audio_channel_map_free(map);
}

Test(audio_channel_map, strategy_default22)
{
    const guint num_sinks   = 2;
    const guint num_sources = 2;

    const PsyAudioChannelStrategy strategy = PSY_AUDIO_CHANNEL_STRATEGY_DEFAULT;

    PsyAudioChannelMap *map
        = psy_audio_channel_map_new_strategy(num_sinks, num_sources, strategy);
    cr_assert(ne(map, NULL));

    cr_expect(eq(uint, psy_audio_channel_map_get_size(map), 2));

    PsyAudioChannelMapping *m0 = psy_audio_channel_map_get_mapping(map, 0);
    PsyAudioChannelMapping *m1 = psy_audio_channel_map_get_mapping(map, 1);

    cr_expect(ne(m0, NULL));
    cr_expect(ne(m1, NULL));

    cr_expect(eq(uint, m0->sink_channel, 0));
    cr_expect(eq(uint, m0->mapped_source, 0));

    cr_expect(eq(uint, m1->sink_channel, 1));
    cr_expect(eq(uint, m1->mapped_source, 1));

    psy_audio_channel_map_free(map);
    psy_audio_channel_mapping_free(m0);
    psy_audio_channel_mapping_free(m1);
}

Test(audio_channel_map, strategy_default21)
{
    const guint num_sinks   = 2;
    const guint num_sources = 1;

    const PsyAudioChannelStrategy strategy = PSY_AUDIO_CHANNEL_STRATEGY_DEFAULT;

    PsyAudioChannelMap *map
        = psy_audio_channel_map_new_strategy(num_sinks, num_sources, strategy);

    cr_assert(ne(map, NULL));
    cr_expect(eq(uint, psy_audio_channel_map_get_size(map), 2));

    PsyAudioChannelMapping *m0 = psy_audio_channel_map_get_mapping(map, 0);
    PsyAudioChannelMapping *m1 = psy_audio_channel_map_get_mapping(map, 1);

    cr_assert(ne(m0, NULL));
    cr_assert(ne(m1, NULL));

    cr_expect(eq(uint, m0->sink_channel, 0));
    cr_expect(eq(uint, m0->mapped_source, 0));

    cr_expect(eq(uint, m1->sink_channel, 1));
    cr_expect(eq(uint, m1->mapped_source, 0));

    psy_audio_channel_map_free(map);
    psy_audio_channel_mapping_free(m0);
    psy_audio_channel_mapping_free(m1);
}

Test(audio_channel_map_strategy, default12)
{
    const guint num_sinks   = 1;
    const guint num_sources = 2;

    const PsyAudioChannelStrategy strategy = PSY_AUDIO_CHANNEL_STRATEGY_DEFAULT;

    PsyAudioChannelMap *map
        = psy_audio_channel_map_new_strategy(num_sinks, num_sources, strategy);

    cr_assert(ne(map, NULL));
    cr_assert(eq(uint, psy_audio_channel_map_get_size(map), 1));

    PsyAudioChannelMapping *m0 = psy_audio_channel_map_get_mapping(map, 0);
    PsyAudioChannelMapping *m1 = psy_audio_channel_map_get_mapping(map, 1);

    cr_assert(ne(m0, NULL));
    cr_expect(zero(m1));

    cr_expect(eq(m0->sink_channel, 0));
    cr_expect(eq(m0->mapped_source, 0));

    psy_audio_channel_map_free(map);
    psy_audio_channel_mapping_free(m0);
}

Test(audio_channel_map, strategy_duplicate_inputs22)
{
    const guint num_sinks   = 2;
    const guint num_sources = 2;

    const PsyAudioChannelStrategy strategy
        = PSY_AUDIO_CHANNEL_STRATEGY_DUPLICATE_INPUTS;

    PsyAudioChannelMap *map
        = psy_audio_channel_map_new_strategy(num_sinks, num_sources, strategy);

    cr_assert(eq(psy_audio_channel_map_get_size(map), 2u));
    cr_assert(ne(map, NULL));

    PsyAudioChannelMapping *m0 = psy_audio_channel_map_get_mapping(map, 0);
    PsyAudioChannelMapping *m1 = psy_audio_channel_map_get_mapping(map, 1);

    cr_assert(ne(m0, NULL));
    cr_assert(ne(m1, NULL));

    cr_expect(eq(m0->sink_channel, 0));
    cr_expect(eq(m0->mapped_source, 0));

    cr_expect(eq(m1->sink_channel, 1));
    cr_expect(eq(m1->mapped_source, 1));

    psy_audio_channel_map_free(map);
    psy_audio_channel_mapping_free(m0);
    psy_audio_channel_mapping_free(m1);
}

Test(audio_channel_map, strategy_duplicate_inputs21)
{
    const guint num_sinks   = 2;
    const guint num_sources = 1;

    const PsyAudioChannelStrategy strategy
        = PSY_AUDIO_CHANNEL_STRATEGY_DUPLICATE_INPUTS;

    PsyAudioChannelMap *map
        = psy_audio_channel_map_new_strategy(num_sinks, num_sources, strategy);

    cr_assert(ne(map, NULL));
    cr_assert(eq(psy_audio_channel_map_get_size(map), 2u));

    PsyAudioChannelMapping *m0 = psy_audio_channel_map_get_mapping(map, 0);
    PsyAudioChannelMapping *m1 = psy_audio_channel_map_get_mapping(map, 1);

    cr_assert(ne(m0, NULL));
    cr_assert(ne(m1, NULL));

    cr_assert(eq(m0->sink_channel, 0));
    cr_assert(eq(m0->mapped_source, 0));

    cr_assert(eq(m1->sink_channel, 1));
    cr_assert(eq(m1->mapped_source, 0));

    psy_audio_channel_map_free(map);
    psy_audio_channel_mapping_free(m0);
    psy_audio_channel_mapping_free(m1);
}

Test(audio_channel_map, strategy_duplicate_inputs12)
{
    const guint num_sinks   = 1;
    const guint num_sources = 2;

    const PsyAudioChannelStrategy strategy
        = PSY_AUDIO_CHANNEL_STRATEGY_DUPLICATE_INPUTS;

    PsyAudioChannelMap *map
        = psy_audio_channel_map_new_strategy(num_sinks, num_sources, strategy);

    cr_assert(ne(map, NULL));
    cr_assert(eq(psy_audio_channel_map_get_size(map), 1u));

    PsyAudioChannelMapping *m0 = psy_audio_channel_map_get_mapping(map, 0);
    PsyAudioChannelMapping *m1 = psy_audio_channel_map_get_mapping(map, 1);

    cr_assert(ne(m0, NULL));
    cr_assert(eq(m1, NULL));

    cr_expect(zero(m0->sink_channel));
    cr_expect(eq(m0->mapped_source, 0));

    psy_audio_channel_map_free(map);
    psy_audio_channel_mapping_free(m0);
    psy_audio_channel_mapping_free(m1);
}

Test(audio_channel_map, strategy_mix_trailing22)
{
    const guint num_sinks   = 2;
    const guint num_sources = 2;

    const PsyAudioChannelStrategy strategy
        = PSY_AUDIO_CHANNEL_STRATEGY_MIX_TRAILING_INPUTS;

    PsyAudioChannelMap *map
        = psy_audio_channel_map_new_strategy(num_sinks, num_sources, strategy);

    cr_assert(ne(map, NULL));
    cr_assert(eq(psy_audio_channel_map_get_size(map), 2u));

    PsyAudioChannelMapping *m0 = psy_audio_channel_map_get_mapping(map, 0);
    PsyAudioChannelMapping *m1 = psy_audio_channel_map_get_mapping(map, 1);

    cr_assert(ne(m0, NULL));
    cr_assert(ne(m1, NULL));

    cr_expect(eq(m0->sink_channel, 0));
    cr_expect(eq(m0->mapped_source, 0));

    cr_expect(eq(m1->sink_channel, 1));
    cr_expect(eq(m1->mapped_source, 1));

    psy_audio_channel_map_free(map);
    psy_audio_channel_mapping_free(m0);
    psy_audio_channel_mapping_free(m1);
}

Test(audio_channel_map, strategy_mix_trailing21)
{
    const guint num_sinks   = 2;
    const guint num_sources = 1;

    const PsyAudioChannelStrategy strategy
        = PSY_AUDIO_CHANNEL_STRATEGY_MIX_TRAILING_INPUTS;

    PsyAudioChannelMap *map
        = psy_audio_channel_map_new_strategy(num_sinks, num_sources, strategy);

    cr_assert(ne(map, NULL));
    cr_expect(eq(psy_audio_channel_map_get_size(map), 1u));

    PsyAudioChannelMapping *m0 = psy_audio_channel_map_get_mapping(map, 0);
    PsyAudioChannelMapping *m1 = psy_audio_channel_map_get_mapping(map, 1);

    cr_expect(ne(m0, NULL));
    cr_expect(zero(m1));

    cr_expect(eq(m0->sink_channel, 0));
    cr_expect(eq(m0->mapped_source, 0));

    psy_audio_channel_map_free(map);
    psy_audio_channel_mapping_free(m0);
    psy_audio_channel_mapping_free(m1);
}

Test(audio_channel_map, strategy_mix_trailing12)
{
    const guint num_sinks   = 1;
    const guint num_sources = 2;

    const PsyAudioChannelStrategy strategy
        = PSY_AUDIO_CHANNEL_STRATEGY_MIX_TRAILING_INPUTS;

    PsyAudioChannelMap *map
        = psy_audio_channel_map_new_strategy(num_sinks, num_sources, strategy);

    cr_assert(ne(map, NULL));
    cr_assert(eq(psy_audio_channel_map_get_size(map), 2u));

    PsyAudioChannelMapping *m0 = psy_audio_channel_map_get_mapping(map, 0);
    PsyAudioChannelMapping *m1 = psy_audio_channel_map_get_mapping(map, 1);

    cr_assert(ne(m0, NULL));
    cr_assert(ne(m1, NULL));

    cr_assert(eq(m0->sink_channel, 0));
    cr_assert(eq(m0->mapped_source, 0));

    cr_assert(eq(m1->sink_channel, 0));
    cr_assert(eq(m1->mapped_source, 1));

    psy_audio_channel_map_free(map);
    psy_audio_channel_mapping_free(m0);
    psy_audio_channel_mapping_free(m1);
}

Test(audio_channel_map, add_mapping)
{
    gboolean            result;
    PsyAudioChannelMap *map = psy_audio_channel_map_new(2, 2);

    // reverses audio channels
    PsyAudioChannelMapping *m1 = psy_audio_channel_mapping_new(0, 1);
    PsyAudioChannelMapping *m2 = psy_audio_channel_mapping_new(1, 0);

    // Invalid mappings
    PsyAudioChannelMapping *i1 = psy_audio_channel_mapping_new(0, 2);
    PsyAudioChannelMapping *i2 = psy_audio_channel_mapping_new(0, 2);

    // returned that should be equal to the ones added.
    PsyAudioChannelMapping *rm1 = NULL;
    PsyAudioChannelMapping *rm2 = NULL;

    result = psy_audio_channel_map_add(map, m1);
    cr_assert(result);
    result = psy_audio_channel_map_add(map, m2);
    cr_assert(result);
    cr_assert(eq(psy_audio_channel_map_get_size(map), 2u));

    result = psy_audio_channel_map_add(map, i1);
    cr_expect(none(result));
    result = psy_audio_channel_map_add(map, i2);
    cr_expect(none(result));
    cr_expect(eq(psy_audio_channel_map_get_size(map), 2u));

    rm1 = psy_audio_channel_map_get_mapping(map, 0);
    rm2 = psy_audio_channel_map_get_mapping(map, 1);

    cr_assert(ne(rm1, NULL));
    cr_assert(ne(rm2, NULL));

    cr_assert(eq(m1->sink_channel, rm1->sink_channel));
    cr_assert(eq(m2->sink_channel, rm2->sink_channel));
    cr_assert(eq(m1->mapped_source, rm1->mapped_source));
    cr_assert(eq(m2->mapped_source, rm2->mapped_source));

    psy_audio_channel_map_free(map);
    psy_audio_channel_mapping_free(m1);
    psy_audio_channel_mapping_free(m2);
    psy_audio_channel_mapping_free(i1);
    psy_audio_channel_mapping_free(i2);
    psy_audio_channel_mapping_free(rm1);
    psy_audio_channel_mapping_free(rm2);
}

Test(audio_channel_map, set_mapping)
{
    gboolean            result;
    PsyAudioChannelMap *map = psy_audio_channel_map_new(2, 2);

    cr_assert(ne(map, NULL));

    PsyAudioChannelMapping *m1 = psy_audio_channel_mapping_new(0, 0);
    PsyAudioChannelMapping *m2 = psy_audio_channel_mapping_new(1, 1);
    PsyAudioChannelMapping *r1 = NULL;
    PsyAudioChannelMapping *r2 = NULL;

    result = psy_audio_channel_map_set(map, 0, m1);
    cr_assert(none(result));
    result = psy_audio_channel_map_set(map, 1, m2);
    cr_assert(none(result));

    psy_audio_channel_map_set_size(map, 2);

    result = psy_audio_channel_map_set(map, 0, m1);
    cr_assert(result);
    result = psy_audio_channel_map_set(map, 1, m2);
    cr_assert(result);

    r1 = psy_audio_channel_map_get_mapping(map, 0);
    r2 = psy_audio_channel_map_get_mapping(map, 1);

    cr_assert(psy_audio_channel_mapping_eq(m1, r1));
    cr_assert(psy_audio_channel_mapping_eq(m2, r2));

    g_clear_pointer(&r1, psy_audio_channel_mapping_free);
    g_clear_pointer(&r2, psy_audio_channel_mapping_free);

    PsyAudioChannelMapping *i1 = psy_audio_channel_mapping_new(0, 2);
    PsyAudioChannelMapping *i2 = psy_audio_channel_mapping_new(2, 0);
    PsyAudioChannelMapping *i3 = psy_audio_channel_mapping_new(-1, 0);
    PsyAudioChannelMapping *i4 = psy_audio_channel_mapping_new(0, -1);

    // It should not be possible to add mapping with negative of to tall values
    result = psy_audio_channel_map_set(map, 0, i1);
    cr_assert(none(result));
    result = psy_audio_channel_map_set(map, 0, i2);
    cr_assert(none(result));
    result = psy_audio_channel_map_set(map, 0, i3);
    cr_assert(none(result));
    result = psy_audio_channel_map_set(map, 0, i4);
    cr_assert(none(result));

    g_clear_pointer(&i1, psy_audio_channel_mapping_free);
    g_clear_pointer(&i2, psy_audio_channel_mapping_free);
    g_clear_pointer(&i3, psy_audio_channel_mapping_free);
    g_clear_pointer(&i4, psy_audio_channel_mapping_free);

    r1 = psy_audio_channel_map_get_mapping(map, 0);
    r2 = psy_audio_channel_map_get_mapping(map, 1);

    // invalid adding of mapping should not touch existing
    cr_assert(psy_audio_channel_mapping_eq(m1, r1));
    cr_assert(psy_audio_channel_mapping_eq(m2, r2));

    g_clear_pointer(&r1, psy_audio_channel_mapping_free);
    g_clear_pointer(&r2, psy_audio_channel_mapping_free);
    g_clear_pointer(&m1, psy_audio_channel_mapping_free);
    g_clear_pointer(&m2, psy_audio_channel_mapping_free);

    g_clear_pointer(&map, psy_audio_channel_map_free);
}
