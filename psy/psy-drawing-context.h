#ifndef PSY_DRAWING_CONTEXT_H
#define PSY_DRAWING_CONTEXT_H

#include "psy-matrix4.h"
#include "psy-shader.h"
#include <gio/gio.h>

G_BEGIN_DECLS

#define PSY_TYPE_DRAWING_CONTEXT psy_drawing_context_get_type()
G_DECLARE_DERIVABLE_TYPE(PsyDrawingContext, psy_drawing_context, PSY, DRAWING_CONTEXT, GObject)

typedef struct _PsyDrawingContextClass {
    GObjectClass parent_class;


} PsyDrawingContextClass;




G_END_DECLS

#endif
