
#ifndef PSY_LOOP_H
#define PSY_LOOP_H

#include "psy-step.h"

G_BEGIN_DECLS

#define PSY_TYPE_LOOP psy_loop_get_type()

G_DECLARE_DERIVABLE_TYPE(PsyLoop, psy_loop, PSY, LOOP, PsyStep)

typedef enum {
    PSY_LOOP_CONDITION_LESS,
    PSY_LOOP_CONDITION_LESS_EQUAL,
    PSY_LOOP_CONDITION_EQUAL,
    PSY_LOOP_CONDITION_GREATER_EQUAL,
    PSY_LOOP_CONDITION_GREATER
} PsyLoopCondition;


struct _PsyLoopClass {
    PsyStepClass parent;

    //void (*iterate)   (PsyLoop* self, gint64 timestamp);
    void (*iteration) (PsyLoop* self, gint64 index, gint64 timestamp);

    gpointer padding[12];
};

PsyLoop*
psy_loop_new();

PsyLoop*
psy_loop_new_full(gint64 index,
                  gint64 stop,
                  gint64 increment,
                  PsyLoopCondition condition
                  );

void
psy_loop_destroy(PsyLoop* loop);

void
psy_loop_iterate(PsyLoop* self, gint64 timestamp);

void
psy_loop_set_index(PsyLoop* self, gint64 index);

gint64
psy_loop_get_index(PsyLoop* self);

void
psy_loop_set_stop(PsyLoop* self, gint64 index);

gint64
psy_loop_get_stop(PsyLoop* self);

void
psy_loop_set_increment(PsyLoop* self, gint64 index);

gint64
psy_loop_get_increment(PsyLoop* self);

void
psy_loop_set_condition(PsyLoop* self, PsyLoopCondition condition);

PsyLoopCondition
psy_loop_get_condition(PsyLoop* self);

gboolean
psy_loop_test(PsyLoop* self);

G_END_DECLS

#endif
