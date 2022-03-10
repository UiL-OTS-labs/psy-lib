
#ifndef PSY_STEPPING_STONES_H
#define PSY_STEPPING_STONES_H

#include <psy-step.h>
#include <gmodule.h>

G_BEGIN_DECLS

#define PSY_STEPPING_STONES_ERROR psy_stepping_stones_error_quark()
G_MODULE_EXPORT GQuark
psy_stepping_stones_error_quark();

typedef enum {
    PSY_STEPPING_STONES_ERROR_KEY_EXISTS,
    PSY_STEPPING_STONES_ERROR_INVALID_INDEX,
    PSY_STEPPING_STONES_ERROR_NO_SUCH_KEY
} PsySteppingStoneError;

#define PSY_TYPE_STEPPING_STONES psy_stepping_stones_get_type()
G_DECLARE_DERIVABLE_TYPE(
        PsySteppingStones, psy_stepping_stones, PSY, STEPPING_STONES, PsyStep
        )

struct _PsySteppingStonesClass {
    PsyStepClass parent;

    gpointer padding[12];
};

PsySteppingStones*
psy_stepping_stones_new();

void
psy_stepping_stones_destroy();

void
psy_stepping_stones_add_step(PsySteppingStones* self, PsyStep* step);

void
psy_stepping_stones_add_step_by_name(PsySteppingStones *self,
                                     const gchar       *name,
                                     PsyStep           *step,
                                     GError           **error);

void
psy_stepping_stones_activate_next_by_index(PsySteppingStones  *self,
                                           guint               index,
                                           GError            **error);

void
psy_stepping_stones_activate_next_by_name(PsySteppingStones *self,
                                          const gchar       *name,
                                          GError           **error);

guint
psy_stepping_stones_get_num_steps(PsySteppingStones* self);

G_END_DECLS

#endif
