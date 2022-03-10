
#ifndef DDD_WIDGET_H
#define DDD_WIDGET_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define DDD_TYPE_WIDGET ddd_widget_get_type()
G_DECLARE_DERIVABLE_TYPE(DddWidget, ddd_widget, DDD, WIDGET, GtkGLArea)

typedef struct _DddWidget DddWidget;
typedef struct _DddWidgetClass DddWidgetClass;

struct _DddWidgetClass {
    GtkGLAreaClass parent_class;

    gpointer padding [16];
};

DddWidget* ddd_widget_new(void);

G_END_DECLS


#endif
