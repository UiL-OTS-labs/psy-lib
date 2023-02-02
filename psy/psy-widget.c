
#include "psy-widget.h"

G_DEFINE_TYPE(PsyWidget, psy_widget, GTK_TYPE_GL_AREA)

static gboolean
render(GtkGLArea *area, GdkGLContext *context)
{
    (void) area, (void) context;
    return FALSE;
}

static void
psy_widget_init(PsyWidget *self)
{
    gtk_gl_area_set_required_version(GTK_GL_AREA(self), 3, 3);
}

static void
psy_widget_class_init(PsyWidgetClass *klass)
{
    GtkGLAreaClass *parent_class = GTK_GL_AREA_CLASS(klass);
    parent_class->render         = render;
}

PsyWidget *
psy_widget_new(void)
{
    PsyWidget *widget = g_object_new(PSY_TYPE_WIDGET, NULL);
    return widget;
}
