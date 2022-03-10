
#ifndef DDD_STEP_H
#define DDD_STEP_H

#include <glib-object.h>

G_BEGIN_DECLS

#define DDD_TYPE_STEP ddd_step_get_type()
G_DECLARE_DERIVABLE_TYPE(DddStep, ddd_step, DDD, STEP, GObject)

struct _DddStepClass {
    GObjectClass parent;

    void (*on_enter)   (DddStep* self, gint64 timestamp);

    void (*on_leave)   (DddStep* self, gint64 timestamp);

    void (*activate)   (DddStep* self, gint64 timestamp);
    void (*deactivate) (DddStep* self, gint64 timestamp);

    gpointer padding[12];
};

void
ddd_step_enter(DddStep* self, gint64 tstamp);

void
ddd_step_leave(DddStep* self, gint64 tstamp);

void
ddd_step_set_parent(DddStep* self, DddStep* parent);

DddStep*
ddd_step_get_parent(DddStep* self);

void
ddd_step_activate(DddStep* self, gint64 timestamp);

GMainContext*
ddd_step_get_main_context(DddStep* self);

G_END_DECLS

#endif
