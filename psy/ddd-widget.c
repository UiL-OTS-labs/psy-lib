
#include "ddd-widget.h"


G_DEFINE_TYPE(DddWidget, ddd_widget, GTK_TYPE_GL_AREA)

static gboolean
render(GtkGLArea *area, GdkGLContext *context)
{
    (void) area, (void) context;
    return FALSE;
}

static void
ddd_widget_init(DddWidget* self)
{
    gtk_gl_area_set_required_version(GTK_GL_AREA(self), 3, 3);
}

static void
ddd_widget_class_init(DddWidgetClass* klass)
{
    GtkGLAreaClass* parent_class = GTK_GL_AREA_CLASS(klass);
    parent_class->render = render;
}

DddWidget*
ddd_widget_new(void)
{
    DddWidget* widget = g_object_new(DDD_TYPE_WIDGET, NULL);
    return widget;
}

