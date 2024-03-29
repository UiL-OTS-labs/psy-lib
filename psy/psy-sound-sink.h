
#pragma once

#include <gio/gio.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define PSY_TYPE_SINK psy_sound_sink_get_type()
G_MODULE_EXPORT
G_DECLARE_INTERFACE(PsySoundSink, psy_sound_sink, PSY, SOUND_SINK, GObject)

struct _PsySoundSink {
    GTypeInterface parent_iface;

    gsize (*write)(PsySoundSink *self, gfloat *sound_signals, gsize nframes);
    void (*pull_audio)(PsySoundSink *self);
};

G_MODULE_EXPORT gsize
psy_sound_sink_write(PsySoundSink *self, gfloat *sound_signals, gsize nframes);

G_END_DECLS
