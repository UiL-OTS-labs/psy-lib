
#ifndef PSYLIB_H
#define PSYLIB_H

#include "psy-config.h"

#include "psy-artist.h"
#include "psy-audio-device.h"
#include "psy-canvas.h"
#include "psy-circle-artist.h"
#include "psy-circle.h"
#include "psy-clock.h"
#include "psy-color.h"
#include "psy-cross-artist.h"
#include "psy-cross.h"
#include "psy-drawing-context.h"
#include "psy-duration.h"
#include "psy-enums.h"
#include "psy-font-utils.h"
#include "psy-image-canvas.h"
#include "psy-image.h"
#include "psy-loop.h"
#include "psy-matrix4.h"
#include "psy-picture-artist.h"
#include "psy-picture.h"
#include "psy-program.h"
#include "psy-random.h"
#include "psy-rectangle-artist.h"
#include "psy-rectangle.h"
#include "psy-shader.h"
#include "psy-step.h"
#include "psy-stepping-stones.h"
#include "psy-stimulus.h"
#include "psy-text-artist.h"
#include "psy-text.h"
#include "psy-texture.h"
#include "psy-time-point.h"
#include "psy-trial.h"
#include "psy-utils.h"
#include "psy-vbuffer.h"
#include "psy-vector.h"
#include "psy-vector4.h"
#include "psy-visual-stimulus.h"
#include "psy-widget.h"
#include "psy-window.h"

#include "backend_gtk/psy-gtk-window.h"

#if defined HAVE_JACK2
    #include "jack/psy-jack-audio-device.h"
#endif

#if defined HAVE_PORTAUDIO
    #include "portaudio/psy-pa-device.h"
#endif

#include "gl/psy-gl-canvas.h"
#include "gl/psy-gl-context.h"
#include "gl/psy-gl-error.h"
#include "gl/psy-gl-fragment-shader.h"
#include "gl/psy-gl-program.h"
#include "gl/psy-gl-shader.h"
#include "gl/psy-gl-texture.h"
#include "gl/psy-gl-vbuffer.h"
#include "gl/psy-gl-vertex-shader.h"

#include "hw/psy-parallel-port.h"
#include "hw/psy-parallel-trigger.h"
#include "hw/psy-parport.h"

#endif
