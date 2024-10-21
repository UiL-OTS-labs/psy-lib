
#pragma once

#include <gio/gio.h>
#include <glib-object.h>

G_BEGIN_DECLS

G_MODULE_EXPORT
G_DECLARE_FINAL_TYPE(PsyInitializer, psy_initializer, PSY, INITIALIZER, GObject)

G_MODULE_EXPORT PsyInitializer *
psy_initializer_new(void);

G_MODULE_EXPORT void
psy_initializer_free(PsyInitializer *self);

G_MODULE_EXPORT void
psy_init(void);

G_MODULE_EXPORT void
psy_deinit(void);

G_END_DECLS
