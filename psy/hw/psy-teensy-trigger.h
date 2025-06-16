
#pragma once

#include <gio/gio.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define PSY_TYPE_TEENSY_TRIGGER psy_teensy_trigger_get_type()

G_MODULE_EXPORT
G_DECLARE_FINAL_TYPE(
    PsyTeensyTrigger, psy_teensy_trigger, PSY, TEENSY_TRIGGER, GObject)

G_MODULE_EXPORT PsyTeensyTrigger *
psy_teensy_trigger_new(const gchar *name);

G_MODULE_EXPORT void
psy_teensy_trigger_open(PsyTeensyTrigger *self, GError **error);

G_END_DECLS
