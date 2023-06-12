
#include <pango/pangocairo.h>

/**
 * psy_enumerate_fonts:
 * @families:(out)(array length=n): An array with the available
 *     font families free with g_free.
 * @n:(out): the size of fonts.
 *
 * Enumerate all fonts. According the pango-cairo docs, the PangoFontMap
 * should not be freed according the pango docs, however, valgrind reports
 * it as an error.
 */
void
psy_enumerate_font_families(gchar ***families, gsize *n)
{
    PangoFontMap *fm = pango_cairo_font_map_get_default();

    PangoFontFamily **font_fams = NULL;
    gint              num_families;

    pango_font_map_list_families(fm, &font_fams, &num_families);
    *n = num_families;

    gchar **ret = g_malloc(sizeof(gchar *) * num_families);

    for (int i = 0; i < num_families; i++) {
        const char *fam = pango_font_family_get_name(font_fams[i]);
        ret[i]          = g_strdup(fam);
    }
    *families = ret;
}
