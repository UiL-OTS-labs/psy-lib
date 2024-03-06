
#ifndef PSY_STEP_H
#define PSY_STEP_H

#include "psy-time-point.h"
#include <glib-object.h>

G_BEGIN_DECLS

#define PSY_STEP_ERROR psy_step_error_quark()

G_MODULE_EXPORT GQuark
psy_step_error_quark(void);

#define PSY_TYPE_STEP psy_step_get_type()

G_MODULE_EXPORT
G_DECLARE_DERIVABLE_TYPE(PsyStep, psy_step, PSY, STEP, GObject)

/**
 * PsyStepClass:
 * @on_enter: A virtual method that is called when entering a step,
 *            This. You should chain up this method to the parent as
 *            that will make sure the step is also activated.
 * @on_leave: This virtual function is called when you are leaving the current
 *            step. It will notify it's parent that you are done, so a parent
 *            loop may start a new iteration or a SteppingStones will step
 *            to it's next added step. You can do something to
 * @activate: This method is called to do something. The activated step,
 *            for a trial means to present stimuli, whereas for a
 *            [class@SteppingStones] or [class@Loop] it would mean to
 *            enter one of its children
 * @post_activate: This is triggered when step is about to be deactivated
 *            This allows loops for example to update the index of the loop.
 *            So when the trial is
 * @deactivate: A function that is called when you are leaving the current
 *            trial. Eventually this will reactivate its parent.
 *
 */
struct _PsyStepClass {
    GObjectClass parent;

    void (*on_enter)(PsyStep *self, PsyTimePoint *timestamp);

    void (*on_leave)(PsyStep *self, PsyTimePoint *timestamp);

    void (*activate)(PsyStep *self, PsyTimePoint *timestamp);
    void (*post_activate)(PsyStep *self);
    void (*deactivate)(PsyStep *self, PsyTimePoint *timestamp);

    gpointer padding[12];
};

G_MODULE_EXPORT void
psy_step_enter(PsyStep *self, PsyTimePoint *tstamp);

G_MODULE_EXPORT void
psy_step_leave(PsyStep *self, PsyTimePoint *tstamp);

G_MODULE_EXPORT void
psy_step_set_parent(PsyStep *self, PsyStep *parent);

G_MODULE_EXPORT PsyStep *
psy_step_get_parent(PsyStep *self);

G_MODULE_EXPORT void
psy_step_activate(PsyStep *self, PsyTimePoint *timestamp);

G_MODULE_EXPORT gboolean
psy_step_get_active(PsyStep *self);

G_MODULE_EXPORT gint64
psy_step_get_loop_index(PsyStep *self, guint nth_loop, GError **error);

G_MODULE_EXPORT void
psy_step_get_loop_indices(PsyStep *self, gint64 **result, guint *size);

G_MODULE_EXPORT GMainContext *
psy_step_get_main_context(PsyStep *self);

G_END_DECLS

#endif
