
#ifndef PSY_STEPPING_STONES_H
#define PSY_STEPPING_STONES_H

#include <gmodule.h>
#include <psy-enums.h>
#include <psy-step.h>

G_BEGIN_DECLS

#define PSY_STEPPING_STONES_ERROR psy_stepping_stones_error_quark()

G_MODULE_EXPORT GQuark
psy_stepping_stones_error_quark(void);

#define PSY_TYPE_STEPPING_STONES psy_stepping_stones_get_type()

G_MODULE_EXPORT
G_DECLARE_DERIVABLE_TYPE(
    PsySteppingStones, psy_stepping_stones, PSY, STEPPING_STONES, PsyStep)

struct _PsySteppingStonesClass {
    PsyStepClass parent;

    gpointer padding[12];
};

G_MODULE_EXPORT PsySteppingStones *
psy_stepping_stones_new(void);

G_MODULE_EXPORT void
psy_stepping_stones_free(PsySteppingStones *self);

G_MODULE_EXPORT gboolean
psy_stepping_stones_add_step(PsySteppingStones *self, PsyStep *step);

G_MODULE_EXPORT gboolean
psy_stepping_stones_add_step_by_name(PsySteppingStones *self,
                                     const gchar       *name,
                                     PsyStep           *step,
                                     GError           **error);

G_MODULE_EXPORT void
psy_stepping_stones_activate_next_by_index(PsySteppingStones *self,
                                           guint              index,
                                           GError           **error);

G_MODULE_EXPORT void
psy_stepping_stones_activate_next_by_name(PsySteppingStones *self,
                                          const gchar       *name,
                                          GError           **error);

G_MODULE_EXPORT guint
psy_stepping_stones_get_num_steps(PsySteppingStones *self);

G_END_DECLS

#endif
