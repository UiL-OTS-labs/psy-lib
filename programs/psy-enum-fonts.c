
#include <psylib.h>

int
main(void)
{

    gint    n;
    gchar **families = NULL;

    psy_enumerate_font_families(&families, &n);

    for (int i = 0; i < n; i++) {
        g_print("%d: %s\n", i + 1, families[i]);
        g_free(families[i]);
    }
    g_free(families);
}
