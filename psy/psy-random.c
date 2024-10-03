
#include "psy-random.h"
#include "psy-config.h"

#include <glib.h>

#if defined(HAVE_SYS_RANDOM_H)
    #include <sys/random.h>
#endif

guint32
psy_random_uint32(void)
{
    guint32 ret;

#if defined(HAVE_SYS_RANDOM_H)

    char   *ptr          = (char *) &ret;
    ssize_t num_read_tot = 0;

    while (num_read_tot < (ssize_t) sizeof(ret)) {
        ssize_t read = 0;
        read         = getrandom(ptr, sizeof(ret) - num_read_tot, 0);
        num_read_tot += read;
        ptr += read;
    }
#else
    union {
        gint32 int_val;
        guint32 uint_val;
    } temp_union;

    temp_union.int_val = g_random_int_range(G_MININT32, G_MAXINT32);
    ret = temp_union.uint_val;

#endif


    return ret;
}
