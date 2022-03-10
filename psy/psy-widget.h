
#ifndef PSY_WIDGET_H
#define PSY_WIDGET_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define PSY_TYPE_WIDGET psy_widget_get_type()
G_DECLARE_DERIVABLE_TYPE(PsyWidget, psy_widget, PSY, WIDGET, GtkGLArea)

typedef struct _PsyWidget PsyWidget;
typedef struct _PsyWidgetClass PsyWidgetClass;

struct _PsyWidgetClass {
    GtkGLAreaClass parent_class;

    gpointer padding [16];
};

PsyWidget* psy_widget_new(void);

G_END_DECLS


#endif
