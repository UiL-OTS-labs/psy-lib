
#include <CUnit/CUnit.h>
#include <psylib.h>

static void
test_audio_channel_mapping(void)
{
    const int source = 2;
    const int sink   = 1;

    PsyAudioChannelMapping *mapping
        = psy_audio_channel_mapping_new(sink, source);

    CU_ASSERT_PTR_NOT_NULL_FATAL(mapping);

    CU_ASSERT_EQUAL(mapping->mapped_source, source);
    CU_ASSERT_EQUAL(mapping->sink_channel, sink);

    psy_audio_channel_mapping_free(mapping);
}

static void
test_audio_channel_map(void)
{
    guint num_sinks   = 2;
    guint num_sources = 2;

    PsyAudioChannelMap *map = psy_audio_channel_map_new(num_sinks, num_sources);

    CU_ASSERT_EQUAL(psy_audio_channel_map_get_size(map), 0);
    CU_ASSERT_EQUAL(map->strategy, PSY_AUDIO_CHANNEL_STRATEGY_CUSTOM);
    CU_ASSERT_EQUAL(map->num_sink_channels, 2);
    CU_ASSERT_EQUAL(map->num_source_channels, 2);

    psy_audio_channel_map_free(map);
}

static void
test_audio_channel_map_strategy_default22(void)
{
    const guint num_sinks   = 2;
    const guint num_sources = 2;

    const PsyAudioChannelStrategy strategy = PSY_AUDIO_CHANNEL_STRATEGY_DEFAULT;

    PsyAudioChannelMap *map
        = psy_audio_channel_map_new_strategy(num_sinks, num_sources, strategy);

    CU_ASSERT_EQUAL(psy_audio_channel_map_get_size(map), 2);
    CU_ASSERT_PTR_NOT_NULL_FATAL(map);

    PsyAudioChannelMapping *m0 = psy_audio_channel_map_get_mapping(map, 0);
    PsyAudioChannelMapping *m1 = psy_audio_channel_map_get_mapping(map, 1);

    CU_ASSERT_PTR_NOT_NULL_FATAL(m0);
    CU_ASSERT_PTR_NOT_NULL_FATAL(m1);

    CU_ASSERT_EQUAL(m0->sink_channel, 0);
    CU_ASSERT_EQUAL(m0->mapped_source, 0);

    CU_ASSERT_EQUAL(m1->sink_channel, 1);
    CU_ASSERT_EQUAL(m1->mapped_source, 1);

    psy_audio_channel_map_free(map);
    psy_audio_channel_mapping_free(m0);
    psy_audio_channel_mapping_free(m1);
}

static void
test_audio_channel_map_strategy_default21(void)
{
    const guint num_sinks   = 2;
    const guint num_sources = 1;

    const PsyAudioChannelStrategy strategy = PSY_AUDIO_CHANNEL_STRATEGY_DEFAULT;

    PsyAudioChannelMap *map
        = psy_audio_channel_map_new_strategy(num_sinks, num_sources, strategy);

    CU_ASSERT_PTR_NOT_NULL_FATAL(map);
    CU_ASSERT_EQUAL(psy_audio_channel_map_get_size(map), 2);

    PsyAudioChannelMapping *m0 = psy_audio_channel_map_get_mapping(map, 0);
    PsyAudioChannelMapping *m1 = psy_audio_channel_map_get_mapping(map, 1);

    CU_ASSERT_PTR_NOT_NULL_FATAL(m0);
    CU_ASSERT_PTR_NOT_NULL_FATAL(m1);

    CU_ASSERT_EQUAL(m0->sink_channel, 0);
    CU_ASSERT_EQUAL(m0->mapped_source, 0);

    CU_ASSERT_EQUAL(m1->sink_channel, 1);
    CU_ASSERT_EQUAL(m1->mapped_source, 0);

    psy_audio_channel_map_free(map);
    psy_audio_channel_mapping_free(m0);
    psy_audio_channel_mapping_free(m1);
}

static void
test_audio_channel_map_strategy_default12(void)
{
    const guint num_sinks   = 1;
    const guint num_sources = 2;

    const PsyAudioChannelStrategy strategy = PSY_AUDIO_CHANNEL_STRATEGY_DEFAULT;

    PsyAudioChannelMap *map
        = psy_audio_channel_map_new_strategy(num_sinks, num_sources, strategy);

    CU_ASSERT_EQUAL(psy_audio_channel_map_get_size(map), 1);
    CU_ASSERT_PTR_NOT_NULL_FATAL(map);

    PsyAudioChannelMapping *m0 = psy_audio_channel_map_get_mapping(map, 0);
    PsyAudioChannelMapping *m1 = psy_audio_channel_map_get_mapping(map, 1);

    CU_ASSERT_PTR_NOT_NULL_FATAL(m0);
    CU_ASSERT_PTR_NULL(m1);

    CU_ASSERT_EQUAL(m0->sink_channel, 0);
    CU_ASSERT_EQUAL(m0->mapped_source, 0);

    psy_audio_channel_map_free(map);
    psy_audio_channel_mapping_free(m0);
}

static void
test_audio_channel_map_strategy_duplicate_inputs22(void)
{
    const guint num_sinks   = 2;
    const guint num_sources = 2;

    const PsyAudioChannelStrategy strategy
        = PSY_AUDIO_CHANNEL_STRATEGY_DUPLICATE_INPUTS;

    PsyAudioChannelMap *map
        = psy_audio_channel_map_new_strategy(num_sinks, num_sources, strategy);

    CU_ASSERT_EQUAL(psy_audio_channel_map_get_size(map), 2);
    CU_ASSERT_PTR_NOT_NULL_FATAL(map);

    PsyAudioChannelMapping *m0 = psy_audio_channel_map_get_mapping(map, 0);
    PsyAudioChannelMapping *m1 = psy_audio_channel_map_get_mapping(map, 1);

    CU_ASSERT_PTR_NOT_NULL_FATAL(m0);
    CU_ASSERT_PTR_NOT_NULL_FATAL(m1);

    CU_ASSERT_EQUAL(m0->sink_channel, 0);
    CU_ASSERT_EQUAL(m0->mapped_source, 0);

    CU_ASSERT_EQUAL(m1->sink_channel, 1);
    CU_ASSERT_EQUAL(m1->mapped_source, 1);

    psy_audio_channel_map_free(map);
    psy_audio_channel_mapping_free(m0);
    psy_audio_channel_mapping_free(m1);
}

static void
test_audio_channel_map_strategy_duplicate_inputs21(void)
{
    const guint num_sinks   = 2;
    const guint num_sources = 1;

    const PsyAudioChannelStrategy strategy
        = PSY_AUDIO_CHANNEL_STRATEGY_DUPLICATE_INPUTS;

    PsyAudioChannelMap *map
        = psy_audio_channel_map_new_strategy(num_sinks, num_sources, strategy);

    CU_ASSERT_EQUAL(psy_audio_channel_map_get_size(map), 2);
    CU_ASSERT_PTR_NOT_NULL_FATAL(map);

    PsyAudioChannelMapping *m0 = psy_audio_channel_map_get_mapping(map, 0);
    PsyAudioChannelMapping *m1 = psy_audio_channel_map_get_mapping(map, 1);

    CU_ASSERT_PTR_NOT_NULL_FATAL(m0);
    CU_ASSERT_PTR_NOT_NULL_FATAL(m1);

    CU_ASSERT_EQUAL(m0->sink_channel, 0);
    CU_ASSERT_EQUAL(m0->mapped_source, 0);

    CU_ASSERT_EQUAL(m1->sink_channel, 1);
    CU_ASSERT_EQUAL(m1->mapped_source, 0);

    psy_audio_channel_map_free(map);
    psy_audio_channel_mapping_free(m0);
    psy_audio_channel_mapping_free(m1);
}

static void
test_audio_channel_map_strategy_duplicate_inputs12(void)
{
    const guint num_sinks   = 1;
    const guint num_sources = 2;

    const PsyAudioChannelStrategy strategy
        = PSY_AUDIO_CHANNEL_STRATEGY_DUPLICATE_INPUTS;

    PsyAudioChannelMap *map
        = psy_audio_channel_map_new_strategy(num_sinks, num_sources, strategy);

    CU_ASSERT_EQUAL(psy_audio_channel_map_get_size(map), 1);
    CU_ASSERT_PTR_NOT_NULL_FATAL(map);

    PsyAudioChannelMapping *m0 = psy_audio_channel_map_get_mapping(map, 0);
    PsyAudioChannelMapping *m1 = psy_audio_channel_map_get_mapping(map, 1);

    CU_ASSERT_PTR_NOT_NULL_FATAL(m0);
    CU_ASSERT_PTR_NULL(m1);

    CU_ASSERT_EQUAL(m0->sink_channel, 0);
    CU_ASSERT_EQUAL(m0->mapped_source, 0);

    psy_audio_channel_map_free(map);
    psy_audio_channel_mapping_free(m0);
    psy_audio_channel_mapping_free(m1);
}

static void
test_audio_channel_map_strategy_mix_trailing22(void)
{
    const guint num_sinks   = 2;
    const guint num_sources = 2;

    const PsyAudioChannelStrategy strategy
        = PSY_AUDIO_CHANNEL_STRATEGY_MIX_TRAILING_INPUTS;

    PsyAudioChannelMap *map
        = psy_audio_channel_map_new_strategy(num_sinks, num_sources, strategy);

    CU_ASSERT_EQUAL(psy_audio_channel_map_get_size(map), 2);
    CU_ASSERT_PTR_NOT_NULL_FATAL(map);

    PsyAudioChannelMapping *m0 = psy_audio_channel_map_get_mapping(map, 0);
    PsyAudioChannelMapping *m1 = psy_audio_channel_map_get_mapping(map, 1);

    CU_ASSERT_PTR_NOT_NULL_FATAL(m0);
    CU_ASSERT_PTR_NOT_NULL_FATAL(m1);

    CU_ASSERT_EQUAL(m0->sink_channel, 0);
    CU_ASSERT_EQUAL(m0->mapped_source, 0);

    CU_ASSERT_EQUAL(m1->sink_channel, 1);
    CU_ASSERT_EQUAL(m1->mapped_source, 1);

    psy_audio_channel_map_free(map);
    psy_audio_channel_mapping_free(m0);
    psy_audio_channel_mapping_free(m1);
}

static void
test_audio_channel_map_strategy_mix_trailing21(void)
{
    const guint num_sinks   = 2;
    const guint num_sources = 1;

    const PsyAudioChannelStrategy strategy
        = PSY_AUDIO_CHANNEL_STRATEGY_MIX_TRAILING_INPUTS;

    PsyAudioChannelMap *map
        = psy_audio_channel_map_new_strategy(num_sinks, num_sources, strategy);

    CU_ASSERT_EQUAL(psy_audio_channel_map_get_size(map), 1);
    CU_ASSERT_PTR_NOT_NULL_FATAL(map);

    PsyAudioChannelMapping *m0 = psy_audio_channel_map_get_mapping(map, 0);
    PsyAudioChannelMapping *m1 = psy_audio_channel_map_get_mapping(map, 1);

    CU_ASSERT_PTR_NOT_NULL_FATAL(m0);
    CU_ASSERT_PTR_NULL(m1);

    CU_ASSERT_EQUAL(m0->sink_channel, 0);
    CU_ASSERT_EQUAL(m0->mapped_source, 0);

    psy_audio_channel_map_free(map);
    psy_audio_channel_mapping_free(m0);
    psy_audio_channel_mapping_free(m1);
}

static void
test_audio_channel_map_strategy_mix_trailing12(void)
{
    const guint num_sinks   = 1;
    const guint num_sources = 2;

    const PsyAudioChannelStrategy strategy
        = PSY_AUDIO_CHANNEL_STRATEGY_MIX_TRAILING_INPUTS;

    PsyAudioChannelMap *map
        = psy_audio_channel_map_new_strategy(num_sinks, num_sources, strategy);

    CU_ASSERT_EQUAL(psy_audio_channel_map_get_size(map), 2);
    CU_ASSERT_PTR_NOT_NULL_FATAL(map);

    PsyAudioChannelMapping *m0 = psy_audio_channel_map_get_mapping(map, 0);
    PsyAudioChannelMapping *m1 = psy_audio_channel_map_get_mapping(map, 1);

    CU_ASSERT_PTR_NOT_NULL_FATAL(m0);
    CU_ASSERT_PTR_NOT_NULL_FATAL(m1);

    CU_ASSERT_EQUAL(m0->sink_channel, 0);
    CU_ASSERT_EQUAL(m0->mapped_source, 0);

    CU_ASSERT_EQUAL(m1->sink_channel, 0);
    CU_ASSERT_EQUAL(m1->mapped_source, 1);

    psy_audio_channel_map_free(map);
    psy_audio_channel_mapping_free(m0);
    psy_audio_channel_mapping_free(m1);
}

static void
test_audio_channel_map_add_mapping(void)
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
    CU_ASSERT_TRUE(result);
    result = psy_audio_channel_map_add(map, m2);
    CU_ASSERT_TRUE(result);
    CU_ASSERT_EQUAL(psy_audio_channel_map_get_size(map), 2);

    result = psy_audio_channel_map_add(map, i1);
    CU_ASSERT_FALSE(result);
    result = psy_audio_channel_map_add(map, i2);
    CU_ASSERT_FALSE(result);
    CU_ASSERT_EQUAL(psy_audio_channel_map_get_size(map), 2);

    rm1 = psy_audio_channel_map_get_mapping(map, 0);
    rm2 = psy_audio_channel_map_get_mapping(map, 1);

    CU_ASSERT_PTR_NOT_NULL_FATAL(rm1);
    CU_ASSERT_PTR_NOT_NULL_FATAL(rm2);

    CU_ASSERT_EQUAL(m1->sink_channel, rm1->sink_channel);
    CU_ASSERT_EQUAL(m2->sink_channel, rm2->sink_channel);
    CU_ASSERT_EQUAL(m1->mapped_source, rm1->mapped_source);
    CU_ASSERT_EQUAL(m2->mapped_source, rm2->mapped_source);

    psy_audio_channel_map_free(map);
    psy_audio_channel_mapping_free(m1);
    psy_audio_channel_mapping_free(m2);
    psy_audio_channel_mapping_free(i1);
    psy_audio_channel_mapping_free(i2);
    psy_audio_channel_mapping_free(rm1);
    psy_audio_channel_mapping_free(rm2);
}

static void
test_audio_channel_map_set_mapping(void)
{
    gboolean            result;
    PsyAudioChannelMap *map = psy_audio_channel_map_new(2, 2);

    CU_ASSERT_PTR_NOT_NULL_FATAL(map);

    PsyAudioChannelMapping *m1 = psy_audio_channel_mapping_new(0, 0);
    PsyAudioChannelMapping *m2 = psy_audio_channel_mapping_new(1, 1);
    PsyAudioChannelMapping *r1 = NULL;
    PsyAudioChannelMapping *r2 = NULL;

    result = psy_audio_channel_map_set(map, 0, m1);
    CU_ASSERT_FALSE(result);
    result = psy_audio_channel_map_set(map, 1, m2);
    CU_ASSERT_FALSE(result);

    psy_audio_channel_map_set_size(map, 2);

    result = psy_audio_channel_map_set(map, 0, m1);
    CU_ASSERT_TRUE(result);
    result = psy_audio_channel_map_set(map, 1, m2);
    CU_ASSERT_TRUE(result);

    r1 = psy_audio_channel_map_get_mapping(map, 0);
    r2 = psy_audio_channel_map_get_mapping(map, 1);

    CU_ASSERT_TRUE(psy_audio_channel_mapping_eq(m1, r1));
    CU_ASSERT_TRUE(psy_audio_channel_mapping_eq(m2, r2));

    g_clear_pointer(&r1, psy_audio_channel_mapping_free);
    g_clear_pointer(&r2, psy_audio_channel_mapping_free);

    PsyAudioChannelMapping *i1 = psy_audio_channel_mapping_new(0, 2);
    PsyAudioChannelMapping *i2 = psy_audio_channel_mapping_new(2, 0);
    PsyAudioChannelMapping *i3 = psy_audio_channel_mapping_new(-1, 0);
    PsyAudioChannelMapping *i4 = psy_audio_channel_mapping_new(0, -1);

    // It should not be possible to add mapping with negative of to tall values
    result = psy_audio_channel_map_set(map, 0, i1);
    CU_ASSERT_FALSE(result);
    result = psy_audio_channel_map_set(map, 0, i2);
    CU_ASSERT_FALSE(result);
    result = psy_audio_channel_map_set(map, 0, i3);
    CU_ASSERT_FALSE(result);
    result = psy_audio_channel_map_set(map, 0, i4);
    CU_ASSERT_FALSE(result);

    g_clear_pointer(&i1, psy_audio_channel_mapping_free);
    g_clear_pointer(&i2, psy_audio_channel_mapping_free);
    g_clear_pointer(&i3, psy_audio_channel_mapping_free);
    g_clear_pointer(&i4, psy_audio_channel_mapping_free);

    r1 = psy_audio_channel_map_get_mapping(map, 0);
    r2 = psy_audio_channel_map_get_mapping(map, 1);

    // invalid adding of mapping should not touch existing
    CU_ASSERT_TRUE(psy_audio_channel_mapping_eq(m1, r1));
    CU_ASSERT_TRUE(psy_audio_channel_mapping_eq(m2, r2));

    g_clear_pointer(&r1, psy_audio_channel_mapping_free);
    g_clear_pointer(&r2, psy_audio_channel_mapping_free);
    g_clear_pointer(&m1, psy_audio_channel_mapping_free);
    g_clear_pointer(&m2, psy_audio_channel_mapping_free);

    g_clear_pointer(&map, psy_audio_channel_map_free);
}

int
add_audio_channel_mapping_suite(void)
{
    CU_Suite *suite = CU_add_suite("Audio channel mapping tests", NULL, NULL);
    CU_Test  *test  = NULL;

    if (!suite)
        return 1;

    test = CU_ADD_TEST(suite, test_audio_channel_mapping);
    if (!test)
        return 1;

    test = CU_ADD_TEST(suite, test_audio_channel_map);
    if (!test)
        return 1;

    test = CU_ADD_TEST(suite, test_audio_channel_map_strategy_default22);
    if (!test)
        return 1;

    test = CU_ADD_TEST(suite, test_audio_channel_map_strategy_default21);
    if (!test)
        return 1;

    test = CU_ADD_TEST(suite, test_audio_channel_map_strategy_default12);
    if (!test)
        return 1;

    test = CU_ADD_TEST(suite,
                       test_audio_channel_map_strategy_duplicate_inputs22);
    if (!test)
        return 1;

    test = CU_ADD_TEST(suite,
                       test_audio_channel_map_strategy_duplicate_inputs21);
    if (!test)
        return 1;

    test = CU_ADD_TEST(suite,
                       test_audio_channel_map_strategy_duplicate_inputs12);
    if (!test)
        return 1;

    test = CU_ADD_TEST(suite, test_audio_channel_map_strategy_mix_trailing22);
    if (!test)
        return 1;

    test = CU_ADD_TEST(suite, test_audio_channel_map_strategy_mix_trailing21);
    if (!test)
        return 1;

    test = CU_ADD_TEST(suite, test_audio_channel_map_strategy_mix_trailing12);
    if (!test)
        return 1;

    test = CU_ADD_TEST(suite, test_audio_channel_map_add_mapping);
    if (!test)
        return 1;

    test = CU_ADD_TEST(suite, test_audio_channel_map_set_mapping);
    if (!test)
        return 1;

    return 0;
}
