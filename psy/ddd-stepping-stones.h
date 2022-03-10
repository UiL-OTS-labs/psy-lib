
#ifndef DDD_STEPPING_STONES_H
#define DDD_STEPPING_STONES_H

#include <ddd-step.h>
#include <gmodule.h>

G_BEGIN_DECLS

#define DDD_STEPPING_STONES_ERROR ddd_stepping_stones_error_quark()
G_MODULE_EXPORT GQuark
ddd_stepping_stones_error_quark();

typedef enum {
    DDD_STEPPING_STONES_ERROR_KEY_EXISTS,
    DDD_STEPPING_STONES_ERROR_INVALID_INDEX,
    DDD_STEPPING_STONES_ERROR_NO_SUCH_KEY
} DddSteppingStoneError;

#define DDD_TYPE_STEPPING_STONES ddd_stepping_stones_get_type()
G_DECLARE_DERIVABLE_TYPE(
        DddSteppingStones, ddd_stepping_stones, DDD, STEPPING_STONES, DddStep
        )

struct _DddSteppingStonesClass {
    DddStepClass parent;

    gpointer padding[12];
};

DddSteppingStones*
ddd_stepping_stones_new();

void
ddd_stepping_stones_destroy();

void
ddd_stepping_stones_add_step(DddSteppingStones* self, DddStep* step);

void
ddd_stepping_stones_add_step_by_name(DddSteppingStones *self,
                                     const gchar       *name,
                                     DddStep           *step,
                                     GError           **error);

void
ddd_stepping_stones_activate_next_by_index(DddSteppingStones  *self,
                                           guint               index,
                                           GError            **error);

void
ddd_stepping_stones_activate_next_by_name(DddSteppingStones *self,
                                          const gchar       *name,
                                          GError           **error);

guint
ddd_stepping_stones_get_num_steps(DddSteppingStones* self);

G_END_DECLS

#endif
