
#include "psy-random.h"
#include "psy-config.h"

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
#endif

    return ret;
}
