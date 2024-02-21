
#ifndef PSY_LOOP_H
#define PSY_LOOP_H

#include "psy-enums.h"
#include "psy-step.h"

G_BEGIN_DECLS

#define PSY_TYPE_LOOP psy_loop_get_type()

G_DECLARE_DERIVABLE_TYPE(PsyLoop, psy_loop, PSY, LOOP, PsyStep)

struct _PsyLoopClass {
    PsyStepClass parent;

    void (*iteration)(PsyLoop *self, gint64 index, PsyTimePoint *timestamp);

    gpointer padding[12];
};

G_MODULE_EXPORT PsyLoop *
psy_loop_new(void);

G_MODULE_EXPORT PsyLoop *
psy_loop_new_full(gint64           index,
                  gint64           stop,
                  gint64           increment,
                  PsyLoopCondition condition);

G_MODULE_EXPORT void
psy_loop_free(PsyLoop *self);

G_MODULE_EXPORT void
psy_loop_iterate(PsyLoop *self, PsyTimePoint *timestamp);

G_MODULE_EXPORT void
psy_loop_set_index(PsyLoop *self, gint64 index);

G_MODULE_EXPORT gint64
psy_loop_get_index(PsyLoop *self);

G_MODULE_EXPORT void
psy_loop_set_stop(PsyLoop *self, gint64 stop);

G_MODULE_EXPORT gint64
psy_loop_get_stop(PsyLoop *self);

G_MODULE_EXPORT void
psy_loop_set_increment(PsyLoop *self, gint64 increment);

G_MODULE_EXPORT gint64
psy_loop_get_increment(PsyLoop *self);

G_MODULE_EXPORT void
psy_loop_set_condition(PsyLoop *self, PsyLoopCondition condition);

G_MODULE_EXPORT PsyLoopCondition
psy_loop_get_condition(PsyLoop *self);

G_MODULE_EXPORT void
psy_loop_set_child(PsyLoop *self, PsyStep *child);

G_MODULE_EXPORT PsyStep *
psy_loop_get_child(PsyLoop *self);

G_MODULE_EXPORT gboolean
psy_loop_test(PsyLoop *self);

G_END_DECLS

#endif
