
#ifndef PSY_STEP_H
#define PSY_STEP_H

#include <glib-object.h>
#include "psy-time-point.h"

G_BEGIN_DECLS

#define PSY_TYPE_STEP psy_step_get_type()
G_DECLARE_DERIVABLE_TYPE(PsyStep, psy_step, PSY, STEP, GObject)

struct _PsyStepClass {
    GObjectClass parent;

    void (*on_enter)   (PsyStep* self, PsyTimePoint* timestamp);

    void (*on_leave)   (PsyStep* self, PsyTimePoint* timestamp);

    void (*activate)   (PsyStep* self, PsyTimePoint* timestamp);
    void (*deactivate) (PsyStep* self, PsyTimePoint* timestamp);

    gpointer padding[12];
};

void
psy_step_enter(PsyStep* self, PsyTimePoint* tstamp);

void
psy_step_leave(PsyStep* self, PsyTimePoint* tstamp);

void
psy_step_set_parent(PsyStep* self, PsyStep* parent);

PsyStep*
psy_step_get_parent(PsyStep* self);

void
psy_step_activate(PsyStep* self, PsyTimePoint* timestamp);

GMainContext*
psy_step_get_main_context(PsyStep* self);

G_END_DECLS

#endif
