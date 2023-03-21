

#include <psylib.h>

#include "unit-test-utilities.h"

static GRand  *g_random_dev;
static guint32 g_seed;

gboolean
init_random(void)
{
    if (g_random_dev) {
        g_warning("Random device already initialized");
        return TRUE;
    }

    g_seed = psy_random_uint32();
    g_print("seed %u\n", g_seed);

    g_random_dev = g_rand_new_with_seed(g_seed);
    if (G_LIKELY(g_random_dev))
        return TRUE;
    return FALSE;
}

gboolean
init_random_with_seed(guint32 seed)
{
    if (g_random_dev) {
        g_warning("Random device already initialized");
        return TRUE;
    }

    g_seed = seed;
    g_print("seed %u\n", seed);
    g_random_dev = g_rand_new_with_seed(seed);
    if (G_LIKELY(g_random_dev))
        return TRUE;
    return FALSE;
}

gint
random_int(void)
{
    if (G_UNLIKELY(!g_random_dev))
        g_critical("g_random = %p, have you initialized the random library?",
                   (gpointer) g_random_dev);

    return g_rand_int(g_random_dev);
}

gint
random_int_range(gint lower, gint upper)
{
    if (G_UNLIKELY(!g_random_dev))
        g_critical("g_random = %p, have you initialized the random library?",
                   (gpointer) g_random_dev);

    return g_rand_int_range(g_random_dev, lower, upper);
}

gdouble
random_double(void)
{
    if (G_UNLIKELY(!g_random_dev))
        g_critical("g_random = %p, have you initialized the random library?",
                   (gpointer) g_random_dev);

    return g_rand_double(g_random_dev);
}

gdouble
random_double_range(gdouble lower, gdouble upper)
{
    if (G_UNLIKELY(!g_random_dev))
        g_critical("g_random = %p, have you initialized the random library?",
                   (gpointer) g_random_dev);

    return g_rand_double_range(g_random_dev, lower, upper);
}

gboolean
random_boolean(void)
{
    if (G_UNLIKELY(!g_random_dev))
        g_critical("g_random = %p, have you initialized the random library?",
                   (gpointer) g_random_dev);

    return g_rand_boolean(g_random_dev);
}
