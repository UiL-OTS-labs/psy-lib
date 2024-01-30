
#pragma once

#include "../psy-duration.h"
#include "../psy-enums.h"
#include "../psy-time-point.h"
#include "psy-parallel-port.h"

G_BEGIN_DECLS

#define PSY_PARALLEL_TRIGGER_ERROR psy_parallel_trigger_error_quark()

G_MODULE_EXPORT GQuark
psy_parallel_trigger_error_quark(void);

#define PSY_TYPE_PARALLEL_TRIGGER psy_parallel_trigger_get_type()
G_DECLARE_DERIVABLE_TYPE(
    PsyParallelTrigger, psy_parallel_trigger, PSY, PARALLEL_TRIGGER, GObject)

/**
 * PsyParallelTriggerClass:
 */
typedef struct _PsyParallelTriggerClass {
    GObjectClass parent_class;

    gpointer padding[8];

} PsyParallelTriggerClass;

G_MODULE_EXPORT PsyParallelTrigger *
psy_parallel_trigger_new(void);

G_MODULE_EXPORT void
psy_parallel_trigger_open(PsyParallelTrigger *self,
                          gint                dev_num,
                          GError            **error);

G_MODULE_EXPORT void
psy_parallel_trigger_close(PsyParallelTrigger *self);

G_MODULE_EXPORT gboolean
psy_parallel_trigger_is_open(PsyParallelTrigger *self);

G_MODULE_EXPORT const gchar *
psy_parallel_trigger_get_port_name(PsyParallelTrigger *self);

G_MODULE_EXPORT void
psy_parallel_trigger_write(PsyParallelTrigger *self,
                           guint8              mask,
                           PsyTimePoint       *tstart,
                           PsyDuration        *dur,
                           GError            **error);

G_MODULE_EXPORT void
psy_parallel_trigger_write_pin(PsyParallelTrigger *self,
                               gint                pin,
                               PsyIoLevel          level,
                               PsyTimePoint       *tstart,
                               PsyDuration        *duration,
                               GError            **error);

G_MODULE_EXPORT void
psy_parallel_trigger_cancel(PsyParallelTrigger *self);

G_END_DECLS
