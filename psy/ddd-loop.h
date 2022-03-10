
#ifndef DDD_LOOP_H
#define DDD_LOOP_H

#include "ddd-step.h"

G_BEGIN_DECLS

#define DDD_TYPE_LOOP ddd_loop_get_type()

G_DECLARE_DERIVABLE_TYPE(DddLoop, ddd_loop, DDD, LOOP, DddStep)

typedef enum {
    DDD_LOOP_CONDITION_LESS,
    DDD_LOOP_CONDITION_LESS_EQUAL,
    DDD_LOOP_CONDITION_EQUAL,
    DDD_LOOP_CONDITION_GREATER_EQUAL,
    DDD_LOOP_CONDITION_GREATER
} DddLoopCondition;


struct _DddLoopClass {
    DddStepClass parent;

    //void (*iterate)   (DddLoop* self, gint64 timestamp);
    void (*iteration) (DddLoop* self, gint64 index, gint64 timestamp);

    gpointer padding[12];
};

DddLoop*
ddd_loop_new();

DddLoop*
ddd_loop_new_full(gint64 index,
                  gint64 stop,
                  gint64 increment,
                  DddLoopCondition condition
                  );

void
ddd_loop_destroy(DddLoop* loop);

void
ddd_loop_iterate(DddLoop* self, gint64 timestamp);

void
ddd_loop_set_index(DddLoop* self, gint64 index);

gint64
ddd_loop_get_index(DddLoop* self);

void
ddd_loop_set_stop(DddLoop* self, gint64 index);

gint64
ddd_loop_get_stop(DddLoop* self);

void
ddd_loop_set_increment(DddLoop* self, gint64 index);

gint64
ddd_loop_get_increment(DddLoop* self);

void
ddd_loop_set_condition(DddLoop* self, DddLoopCondition condition);

DddLoopCondition
ddd_loop_get_condition(DddLoop* self);

gboolean
ddd_loop_test(DddLoop* self);

G_END_DECLS

#endif
