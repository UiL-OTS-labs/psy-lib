/**
 * PsyWindow:
 *
 * PsyWindow is an abstract class. It provides an base class for platform or
 * toolkit specific backends. A window in PsyLib is typically a fullscreen
 * window that is placed on a specific monitor.
 *
 * From the point of a psychological experiment tool-kit it doesn't make
 * sense to allow window that are not fullscreen, hence every window will
 * be created at full screen resolution.
 *
 * A derived window will know when it is ready to draw the window, as such
 * it will call the PsyWindow.draw method. The draw method will call two
 * functions.
 *
 * * It will call the PsyWindowClass::clear function to clear the background
 * to the default color.
 * * It will call the draw_stimuli function.
 *
 * The draw stimuli function does two things:
 *
 * 1. It will check the scheduled stimuli, to see whether there is
 *    a stimulus ready to present.
 * 2. It will update the stimuli that should be presented and make
 *    sure that the `PsyArtist`s will actually draw every stimulus.
 */
#include "psy-window.h"
#include "enum-types.h"
#include "psy-artist.h"
#include "psy-circle-artist.h"
#include "psy-circle.h"
#include "psy-cross-artist.h"
#include "psy-cross.h"
#include "psy-drawing-context.h"
#include "psy-duration.h"
#include "psy-matrix4.h"
#include "psy-picture-artist.h"
#include "psy-picture.h"
#include "psy-program.h"
#include "psy-rectangle-artist.h"
#include "psy-rectangle.h"
#include "psy-stimulus.h"
#include "psy-time-point.h"
#include "psy-visual-stimulus.h"

typedef struct PsyWindowPrivate {
    gint monitor;
} PsyWindowPrivate;

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE(PsyWindow, psy_window, PSY_TYPE_CANVAS)

typedef enum { N_MONITOR = 1, N_PROPS } PsyWindowProperty;

static void
psy_window_set_property(GObject      *object,
                        guint         property_id,
                        const GValue *value,
                        GParamSpec   *spec)
{
    PsyWindow *self = PSY_WINDOW(object);

    switch ((PsyWindowProperty) property_id) {
    case N_MONITOR:
        psy_window_set_monitor(self, g_value_get_int(value));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, spec);
    }
}

static void
psy_window_get_property(GObject    *object,
                        guint       property_id,
                        GValue     *value,
                        GParamSpec *spec)
{
    PsyWindow *self = PSY_WINDOW(object);

    switch ((PsyWindowProperty) property_id) {
    case N_MONITOR:
        g_value_set_int(value, psy_window_get_monitor(self));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, spec);
    }
}

static void
psy_window_init(PsyWindow *self)
{
    PsyWindowPrivate *priv = psy_window_get_instance_private(self);
    (void) priv;
}

static void
psy_window_dispose(GObject *gobject)
{
    PsyWindowPrivate *priv
        = psy_window_get_instance_private(PSY_WINDOW(gobject));

    (void) priv;

    G_OBJECT_CLASS(psy_window_parent_class)->dispose(gobject);
}

static void
psy_window_finalize(GObject *gobject)
{
    PsyWindowPrivate *priv
        = psy_window_get_instance_private(PSY_WINDOW(gobject));
    (void) priv;

    G_OBJECT_CLASS(psy_window_parent_class)->finalize(gobject);
}

static GParamSpec *obj_properties[N_PROPS];

static gint
get_monitor(PsyWindow *self)
{
    PsyWindowPrivate *priv = psy_window_get_instance_private(self);
    return priv->monitor;
}

static void
set_monitor(PsyWindow *self, gint nth_monitor)
{
    PsyWindowPrivate *priv = psy_window_get_instance_private(self);
    priv->monitor          = nth_monitor;
}

static void
psy_window_class_init(PsyWindowClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    object_class->set_property = psy_window_set_property;
    object_class->get_property = psy_window_get_property;
    object_class->dispose      = psy_window_dispose;
    object_class->finalize     = psy_window_finalize;

    klass->get_monitor = get_monitor;
    klass->set_monitor = set_monitor;

    /**
     * PsyWindow:n-monitor:
     *
     * The number of the monitor on which the PsyWindow should be
     * presented.
     */
    obj_properties[N_MONITOR]
        = g_param_spec_int("n-monitor",
                           "nmonitor",
                           "The number of the monitor to use for this window",
                           -1,
                           G_MAXINT32,
                           0,
                           G_PARAM_CONSTRUCT | G_PARAM_READWRITE);

    g_object_class_install_properties(object_class, N_PROPS, obj_properties);
}

/**
 * psy_window_get_monitor:
 * @self: a #PsyWindow instance
 *
 * Returns the number of the monitor this window is being presented.
 * The result should b 0 for the first monitor to n - 1 for the last
 * where n is the number of monitors available to psy-lib.
 * Once there are multiple backends, it could be the case that that
 * the number is specific for this backend, and the number represents another
 * monitor for another backend.
 *
 * Returns: 0 to n - 1 where n is the number of monitors connected or
 *          -1 in case of an error.
 */
gint
psy_window_get_monitor(PsyWindow *self)
{
    g_return_val_if_fail(PSY_IS_WINDOW(self), -1);

    PsyWindowClass *class = PSY_WINDOW_GET_CLASS(self);
    g_return_val_if_fail(class->get_monitor, -1);

    return class->get_monitor(self);
}

/**
 * psy_window_set_monitor:
 * @self: a #PsyWindow instance
 * @nth_monitor: the number of the monitor on which to display the window
 *
 * Returns the number of the monitor this window is being presented.
 * The result should b 0 for the first monitor to n - 1 for the last
 * where n is the number of monitors available to psy-lib.
 * Once there are multiple backends, it could be the case that that
 * the number is specific for this backend, and the number represents another
 * monitor for another backend.
 *
 */
void
psy_window_set_monitor(PsyWindow *self, gint nth_monitor)
{
    g_return_if_fail(PSY_IS_WINDOW(self));

    PsyWindowClass *class = PSY_WINDOW_GET_CLASS(self);
    g_return_if_fail(class->set_monitor);

    class->set_monitor(self, nth_monitor);
}
