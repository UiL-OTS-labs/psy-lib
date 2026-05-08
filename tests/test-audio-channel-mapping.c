#include <psylib.h>

#include "unit-test-utilities.h"

static gboolean
test_fatal_hander(const gchar   *domain,
                  GLogLevelFlags level,
                  const char    *message,
                  gpointer       data)
{
    (void) domain;
    (void) level;
    (void) message;
    (void) data;
    if (g_strcmp0("Psy", domain) == 0) {
        if (g_strrstr(message, "psy_audio_channel_map_get_mapping") != NULL)
            return FALSE;
        if (g_strrstr(message, "psy_audio_channel_map_add") != NULL)
            return FALSE;
        if (g_strrstr(message, "psy_audio_channel_map_set") != NULL)
            return FALSE;
        if (g_strrstr(message, "psy_audio_channel_mapping_new") != NULL)
            return FALSE;
    }
    g_print("message = %s", message);
    return TRUE;
}

static void
audio_channel_mapping(void)
{
    const int source = 2;
    const int sink   = 1;

    PsyAudioChannelMapping *mapping
        = psy_audio_channel_mapping_new(sink, source);

    g_assert_nonnull(mapping);

    g_assert_cmpint(mapping->mapped_source, ==, source);
    g_assert_cmpint(mapping->sink_channel, ==, sink);

    psy_audio_channel_mapping_free(mapping);
}

static void
audio_channel_map(void)
{
    guint num_sinks   = 2;
    guint num_sources = 2;

    PsyAudioChannelMap *map = psy_audio_channel_map_new(num_sinks, num_sources);

    g_assert_nonnull(map);

    g_assert_cmpuint(psy_audio_channel_map_get_size(map), ==, 0);
    g_assert_cmpuint(map->strategy, ==, PSY_AUDIO_CHANNEL_STRATEGY_CUSTOM);
    g_assert_cmpuint(map->num_sink_channels, ==, 2);
    g_assert_cmpuint(map->num_source_channels, ==, 2);

    psy_audio_channel_map_free(map);
}

static void
audio_channel_map_strategy_default22(void)
{
    const guint num_sinks   = 2;
    const guint num_sources = 2;

    const PsyAudioChannelStrategy strategy = PSY_AUDIO_CHANNEL_STRATEGY_DEFAULT;

    PsyAudioChannelMap *map
        = psy_audio_channel_map_new_strategy(num_sinks, num_sources, strategy);
    g_assert_nonnull(map);

    g_assert_cmpuint(psy_audio_channel_map_get_size(map), ==, 2);

    PsyAudioChannelMapping *m0 = psy_audio_channel_map_get_mapping(map, 0);
    PsyAudioChannelMapping *m1 = psy_audio_channel_map_get_mapping(map, 1);

    g_assert_nonnull(m0);
    g_assert_nonnull(m1);

    g_assert_cmpuint(m0->sink_channel, ==, 0);
    g_assert_cmpuint(m0->mapped_source, ==, 0);

    g_assert_cmpuint(m1->sink_channel, ==, 1);
    g_assert_cmpuint(m1->mapped_source, ==, 1);

    psy_audio_channel_map_free(map);
    psy_audio_channel_mapping_free(m0);
    psy_audio_channel_mapping_free(m1);
}

static void
audio_channel_map_strategy_default21(void)
{
    const guint num_sinks   = 2;
    const guint num_sources = 1;

    const PsyAudioChannelStrategy strategy = PSY_AUDIO_CHANNEL_STRATEGY_DEFAULT;

    PsyAudioChannelMap *map
        = psy_audio_channel_map_new_strategy(num_sinks, num_sources, strategy);

    g_assert_nonnull(map);
    g_assert_cmpuint(psy_audio_channel_map_get_size(map), ==, 2);

    PsyAudioChannelMapping *m0 = psy_audio_channel_map_get_mapping(map, 0);
    PsyAudioChannelMapping *m1 = psy_audio_channel_map_get_mapping(map, 1);

    g_assert_nonnull(m0);
    g_assert_nonnull(m1);

    g_assert_cmpuint(m0->sink_channel, ==, 0);
    g_assert_cmpuint(m0->mapped_source, ==, 0);

    g_assert_cmpuint(m1->sink_channel, ==, 1);
    g_assert_cmpuint(m1->mapped_source, ==, 0);

    psy_audio_channel_map_free(map);
    psy_audio_channel_mapping_free(m0);
    psy_audio_channel_mapping_free(m1);
}

static void
audio_channel_map_strategy_default12(void)
{
    g_test_log_set_fatal_handler(test_fatal_hander, NULL);
    const guint num_sinks   = 1;
    const guint num_sources = 2;

    const PsyAudioChannelStrategy strategy = PSY_AUDIO_CHANNEL_STRATEGY_DEFAULT;

    PsyAudioChannelMap *map
        = psy_audio_channel_map_new_strategy(num_sinks, num_sources, strategy);

    g_assert_nonnull(map);
    g_assert_cmpuint(psy_audio_channel_map_get_size(map), ==, 1);

    PsyAudioChannelMapping *m0 = psy_audio_channel_map_get_mapping(map, 0);
    PsyAudioChannelMapping *m1 = psy_audio_channel_map_get_mapping(map, 1);

    g_assert_nonnull(m0);
    g_assert_null(m1);

    g_assert_cmpuint(m0->sink_channel, ==, 0);
    g_assert_cmpuint(m0->mapped_source, ==, 0);

    psy_audio_channel_map_free(map);
    psy_audio_channel_mapping_free(m0);
}

static void
audio_channel_map_strategy_duplicate_inputs22(void)
{
    const guint num_sinks   = 2;
    const guint num_sources = 2;

    const PsyAudioChannelStrategy strategy
        = PSY_AUDIO_CHANNEL_STRATEGY_DUPLICATE_INPUTS;

    PsyAudioChannelMap *map
        = psy_audio_channel_map_new_strategy(num_sinks, num_sources, strategy);

    g_assert_cmpuint(psy_audio_channel_map_get_size(map), ==, 2u);
    g_assert_nonnull(map);

    PsyAudioChannelMapping *m0 = psy_audio_channel_map_get_mapping(map, 0);
    PsyAudioChannelMapping *m1 = psy_audio_channel_map_get_mapping(map, 1);

    g_assert_nonnull(m0);
    g_assert_nonnull(m1);

    g_assert_cmpuint(m0->sink_channel, ==, 0);
    g_assert_cmpuint(m0->mapped_source, ==, 0);

    g_assert_cmpuint(m1->sink_channel, ==, 1);
    g_assert_cmpuint(m1->mapped_source, ==, 1);

    psy_audio_channel_map_free(map);
    psy_audio_channel_mapping_free(m0);
    psy_audio_channel_mapping_free(m1);
}

static void
audio_channel_map_strategy_duplicate_inputs21(void)
{
    const guint num_sinks   = 2;
    const guint num_sources = 1;

    const PsyAudioChannelStrategy strategy
        = PSY_AUDIO_CHANNEL_STRATEGY_DUPLICATE_INPUTS;

    PsyAudioChannelMap *map
        = psy_audio_channel_map_new_strategy(num_sinks, num_sources, strategy);

    g_assert_nonnull(map);
    g_assert_cmpuint(psy_audio_channel_map_get_size(map), ==, 2u);

    PsyAudioChannelMapping *m0 = psy_audio_channel_map_get_mapping(map, 0);
    PsyAudioChannelMapping *m1 = psy_audio_channel_map_get_mapping(map, 1);

    g_assert_nonnull(m0);
    g_assert_nonnull(m1);

    g_assert_cmpuint(m0->sink_channel, ==, 0);
    g_assert_cmpuint(m0->mapped_source, ==, 0);

    g_assert_cmpuint(m1->sink_channel, ==, 1);
    g_assert_cmpuint(m1->mapped_source, ==, 0);

    psy_audio_channel_map_free(map);
    psy_audio_channel_mapping_free(m0);
    psy_audio_channel_mapping_free(m1);
}

static void
audio_channel_map_strategy_duplicate_inputs12(void)
{
    g_test_log_set_fatal_handler(test_fatal_hander, NULL);
    const guint num_sinks   = 1;
    const guint num_sources = 2;

    const PsyAudioChannelStrategy strategy
        = PSY_AUDIO_CHANNEL_STRATEGY_DUPLICATE_INPUTS;

    PsyAudioChannelMap *map
        = psy_audio_channel_map_new_strategy(num_sinks, num_sources, strategy);

    g_assert_nonnull(map);
    g_assert_cmpuint(psy_audio_channel_map_get_size(map), ==, 1u);

    PsyAudioChannelMapping *m0 = psy_audio_channel_map_get_mapping(map, 0);
    PsyAudioChannelMapping *m1 = psy_audio_channel_map_get_mapping(map, 1);

    g_assert_nonnull(m0);
    g_assert_null(m1);

    g_assert_cmpuint(m0->sink_channel, ==, 0);
    g_assert_cmpuint(m0->mapped_source, ==, 0);

    psy_audio_channel_map_free(map);
    psy_audio_channel_mapping_free(m0);
    psy_audio_channel_mapping_free(m1);
}

static void
audio_channel_map_strategy_mix_trailing22(void)
{
    const guint num_sinks   = 2;
    const guint num_sources = 2;

    const PsyAudioChannelStrategy strategy
        = PSY_AUDIO_CHANNEL_STRATEGY_MIX_TRAILING_INPUTS;

    PsyAudioChannelMap *map
        = psy_audio_channel_map_new_strategy(num_sinks, num_sources, strategy);

    g_assert_nonnull(map);
    g_assert_cmpuint(psy_audio_channel_map_get_size(map), ==, 2u);

    PsyAudioChannelMapping *m0 = psy_audio_channel_map_get_mapping(map, 0);
    PsyAudioChannelMapping *m1 = psy_audio_channel_map_get_mapping(map, 1);

    g_assert_nonnull(m0);
    g_assert_nonnull(m1);

    g_assert_cmpuint(m0->sink_channel, ==, 0);
    g_assert_cmpuint(m0->mapped_source, ==, 0);

    g_assert_cmpuint(m1->sink_channel, ==, 1);
    g_assert_cmpuint(m1->mapped_source, ==, 1);

    psy_audio_channel_map_free(map);
    psy_audio_channel_mapping_free(m0);
    psy_audio_channel_mapping_free(m1);
}

static void
audio_channel_map_strategy_mix_trailing21(void)
{
    const guint num_sinks   = 2;
    const guint num_sources = 1;

    g_test_log_set_fatal_handler(test_fatal_hander, NULL);

    const PsyAudioChannelStrategy strategy
        = PSY_AUDIO_CHANNEL_STRATEGY_MIX_TRAILING_INPUTS;

    PsyAudioChannelMap *map
        = psy_audio_channel_map_new_strategy(num_sinks, num_sources, strategy);

    g_assert_nonnull(map);
    g_assert_cmpuint(psy_audio_channel_map_get_size(map), ==, 1u);

    PsyAudioChannelMapping *m0 = psy_audio_channel_map_get_mapping(map, 0);
    PsyAudioChannelMapping *m1 = psy_audio_channel_map_get_mapping(map, 1);

    g_assert_nonnull(m0);
    g_assert_null(m1);

    g_assert_cmpuint(m0->sink_channel, ==, 0);
    g_assert_cmpuint(m0->mapped_source, ==, 0);

    psy_audio_channel_map_free(map);
    psy_audio_channel_mapping_free(m0);
    psy_audio_channel_mapping_free(m1);
}

static void
audio_channel_map_strategy_mix_trailing12(void)
{
    const guint num_sinks   = 1;
    const guint num_sources = 2;

    const PsyAudioChannelStrategy strategy
        = PSY_AUDIO_CHANNEL_STRATEGY_MIX_TRAILING_INPUTS;

    PsyAudioChannelMap *map
        = psy_audio_channel_map_new_strategy(num_sinks, num_sources, strategy);

    g_assert_nonnull(map);
    g_assert_cmpuint(psy_audio_channel_map_get_size(map), ==, 2u);

    PsyAudioChannelMapping *m0 = psy_audio_channel_map_get_mapping(map, 0);
    PsyAudioChannelMapping *m1 = psy_audio_channel_map_get_mapping(map, 1);

    g_assert_nonnull(m0);
    g_assert_nonnull(m1);

    g_assert_cmpuint(m0->sink_channel, ==, 0);
    g_assert_cmpuint(m0->mapped_source, ==, 0);

    g_assert_cmpuint(m1->sink_channel, ==, 0);
    g_assert_cmpuint(m1->mapped_source, ==, 1);

    psy_audio_channel_map_free(map);
    psy_audio_channel_mapping_free(m0);
    psy_audio_channel_mapping_free(m1);
}

static void
audio_channel_map_add_mapping(void)
{
    gboolean result;

    g_test_log_set_fatal_handler(test_fatal_hander, NULL);

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
    g_assert_true(result);
    result = psy_audio_channel_map_add(map, m2);
    g_assert_true(result);
    g_assert_cmpuint(psy_audio_channel_map_get_size(map), ==, 2u);

    result = psy_audio_channel_map_add(map, i1);
    g_assert_false(result);
    result = psy_audio_channel_map_add(map, i2);
    g_assert_false(result);
    g_assert_cmpuint(psy_audio_channel_map_get_size(map), ==, 2u);

    rm1 = psy_audio_channel_map_get_mapping(map, 0);
    rm2 = psy_audio_channel_map_get_mapping(map, 1);

    g_assert_nonnull(rm1);
    g_assert_nonnull(rm2);

    g_assert_cmpuint(m1->sink_channel, ==, rm1->sink_channel);
    g_assert_cmpuint(m2->sink_channel, ==, rm2->sink_channel);
    g_assert_cmpuint(m1->mapped_source, ==, rm1->mapped_source);
    g_assert_cmpuint(m2->mapped_source, ==, rm2->mapped_source);

    psy_audio_channel_map_free(map);
    psy_audio_channel_mapping_free(m1);
    psy_audio_channel_mapping_free(m2);
    psy_audio_channel_mapping_free(i1);
    psy_audio_channel_mapping_free(i2);
    psy_audio_channel_mapping_free(rm1);
    psy_audio_channel_mapping_free(rm2);
}

static void
audio_channel_map_set_mapping(void)
{
    gboolean result;

    g_test_log_set_fatal_handler(test_fatal_hander, NULL);

    PsyAudioChannelMap *map = psy_audio_channel_map_new(2, 2);

    g_assert_nonnull(map);

    PsyAudioChannelMapping *m1 = psy_audio_channel_mapping_new(0, 0);
    PsyAudioChannelMapping *m2 = psy_audio_channel_mapping_new(1, 1);
    PsyAudioChannelMapping *r1 = NULL;
    PsyAudioChannelMapping *r2 = NULL;

    result = psy_audio_channel_map_set(map, 0, m1);
    g_assert_false(result);
    result = psy_audio_channel_map_set(map, 1, m2);
    g_assert_false(result);

    psy_audio_channel_map_set_size(map, 2);

    result = psy_audio_channel_map_set(map, 0, m1);
    g_assert_true(result);
    result = psy_audio_channel_map_set(map, 1, m2);
    g_assert_true(result);

    r1 = psy_audio_channel_map_get_mapping(map, 0);
    r2 = psy_audio_channel_map_get_mapping(map, 1);

    g_assert_true(psy_audio_channel_mapping_eq(m1, r1));
    g_assert_true(psy_audio_channel_mapping_eq(m2, r2));

    g_clear_pointer(&r1, psy_audio_channel_mapping_free);
    g_clear_pointer(&r2, psy_audio_channel_mapping_free);

    PsyAudioChannelMapping *i1 = psy_audio_channel_mapping_new(0, 2);
    PsyAudioChannelMapping *i2 = psy_audio_channel_mapping_new(2, 0);
    PsyAudioChannelMapping *i3 = psy_audio_channel_mapping_new(-1, 0);
    PsyAudioChannelMapping *i4 = psy_audio_channel_mapping_new(0, -1);

    // It should not be possible to add mapping with negative of to tall values
    result = psy_audio_channel_map_set(map, 0, i1);
    g_assert_false(result);
    result = psy_audio_channel_map_set(map, 0, i2);
    g_assert_false(result);
    result = psy_audio_channel_map_set(map, 0, i3);
    g_assert_false(result);
    result = psy_audio_channel_map_set(map, 0, i4);
    g_assert_false(result);

    g_clear_pointer(&i1, psy_audio_channel_mapping_free);
    g_clear_pointer(&i2, psy_audio_channel_mapping_free);
    g_clear_pointer(&i3, psy_audio_channel_mapping_free);
    g_clear_pointer(&i4, psy_audio_channel_mapping_free);

    r1 = psy_audio_channel_map_get_mapping(map, 0);
    r2 = psy_audio_channel_map_get_mapping(map, 1);

    // invalid adding of mapping should not touch existing
    g_assert_true(psy_audio_channel_mapping_eq(m1, r1));
    g_assert_true(psy_audio_channel_mapping_eq(m2, r2));

    g_clear_pointer(&r1, psy_audio_channel_mapping_free);
    g_clear_pointer(&r2, psy_audio_channel_mapping_free);
    g_clear_pointer(&m1, psy_audio_channel_mapping_free);
    g_clear_pointer(&m2, psy_audio_channel_mapping_free);

    g_clear_pointer(&map, psy_audio_channel_map_free);
}

int
main(int argc, char **argv)
{
    g_test_init(&argc, &argv, NULL);

    g_test_add_func("/audio-channel/mapping", audio_channel_mapping);
    g_test_add_func("/audio-channel/map", audio_channel_map);

    g_test_add_func("/audio-channel/stategy-default22",
                    audio_channel_map_strategy_default22);
    g_test_add_func("/audio-channel/stategy-default21",
                    audio_channel_map_strategy_default21);
    g_test_add_func("/audio-channel/stategy-default12",
                    audio_channel_map_strategy_default12);

    g_test_add_func("/audio-channel/stategy-duplicate-inputs22",
                    audio_channel_map_strategy_duplicate_inputs22);
    g_test_add_func("/audio-channel/stategy-duplicate-inputs21",
                    audio_channel_map_strategy_duplicate_inputs21);
    g_test_add_func("/audio-channel/stategy-duplicate-inputs12",
                    audio_channel_map_strategy_duplicate_inputs12);

    g_test_add_func("/audio-channel/stategy-mix-trailing22",
                    audio_channel_map_strategy_mix_trailing22);
    g_test_add_func("/audio-channel/stategy-mix-trailing21",
                    audio_channel_map_strategy_mix_trailing21);
    g_test_add_func("/audio-channel/stategy-mix-trailing12",
                    audio_channel_map_strategy_mix_trailing12);

    g_test_add_func("/audio-channel-map/add-mapping",
                    audio_channel_map_add_mapping);
    g_test_add_func("/audio-channel-map/set-mapping",
                    audio_channel_map_set_mapping);

    return g_test_run();
}
