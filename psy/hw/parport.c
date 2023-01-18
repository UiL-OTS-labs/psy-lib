

#include <error.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "parport.h"

/**
 * PsyParport:
 *
 * PsyParport is a final class for parallel ports on Linux. It derives from
 * PsyParallePort and implements it.
 */

typedef struct _PsyParport {
    PsyParallelPort parent;
    int             fd;
} PsyParport;

G_DEFINE_TYPE(PsyParport, psy_parport, PSY_TYPE_PARALLEL_PORT)

static void
psy_parport_init(PsyParport *self)
{
    (void) self;
}

static void
parport_open(PsyParallelPort *self, gint port_num, GError **error)
{
    gchar        buffer[64];
    PsyParport  *pp       = PSY_PARPORT(self);
    const gchar *dev_name = NULL;

    PsyParallelPortClass *parallel_cls = PSY_PARALLEL_PORT_GET_CLASS(self);

    psy_parallel_port_close(self);

    PSY_PARALLEL_PORT_CLASS(psy_parport_parent_class)
        ->open(self, port_num, error);

    g_snprintf(buffer, sizeof(buffer), "/dev/parport%u", port_num);
    parallel_cls->set_port_name(self, buffer);

    dev_name = psy_parallel_port_get_port_name(self);

    errno  = 0;
    pp->fd = open(dev_name, O_RDWR);
    if (pp->fd < 0) {
        g_set_error(error,
                    PSY_PARALLEL_PORT_ERROR,
                    PSY_PARALLEL_PORT_ERROR_OPEN,
                    "Unable to open %s: %s",
                    dev_name,
                    g_strerror(errno));
        psy_parallel_port_close(self);
        return;
    }
}

static void
parport_close(PsyParallelPort *self)
{
    PSY_PARALLEL_PORT_CLASS(psy_parport_parent_class)->close(self);
}

static void
psy_parport_class_init(PsyParportClass *cls)
{
    GObjectClass *obj_cls = G_OBJECT_CLASS(cls);
    (void) obj_cls;

    PsyParallelPortClass *parallel_cls = PSY_PARALLEL_PORT_CLASS(cls);

    parallel_cls->open  = parport_open;
    parallel_cls->close = parport_close;
}
