
# Directory for external libraries for used to build
# libpsy.

cdata = configuration_data({
})

cdata.set_quoted('PSY_VERSION', meson.project_version())
cdata.set_quoted('PSY_SOURCE_ROOT', meson.project_source_root())
cdata.set_quoted('PSY_BUILD_ROOT', meson.project_build_root())

# check for compiler buildins

if cc.has_function('__builtin_add_overflow')
    cdata.set('HAVE_BUILTIN_ADD_OVERFLOW', 1)
endif

if cc.has_function('__builtin_sub_overflow')
    cdata.set('HAVE_BUILTIN_SUB_OVERFLOW', 1)
endif

if cc.has_function('__builtin_mul_overflow')
    cdata.set('HAVE_BUILTIN_MUL_OVERFLOW', 1)
endif

# check for headers
if cc.has_header('unistd.h')
    cdata.set('HAVE_UNISTD_H', true)
endif

if cc.has_header('linux/parport.h')
    cdata.set('HAVE_LINUX_PARPORT_H', true)
endif

if cc.has_header('linux/ppdev.h')
    cdata.set('HAVE_LINUX_PPDEV_H', true)
endif

if cc.has_header('sys/random.h')
    cdata.set('HAVE_SYS_RANDOM_H', true)
endif

if cc.has_header('jack/jack.h')
    cdata.set('HAVE_JACK_H', true)
endif

if cc.has_header('portaudio.h')
    cdata.set('HAVE_PORTAUDIO_H', true)
endif

if cc.has_header('pa_linux_alsa.h')
    cdata.set('HAVE_PA_LINUX_ALSA_H', true)
endif

if cc.has_header('alsa/asoundlib.h')
    cdata.set('HAVE_ASOUNDLIB_H', true)
endif

if cxx.has_header('boost/lockfree/spsc_queue.hpp')
    cdata.set('HAVE_BOOST_LOCKFREE_SPSC_QUEUE_HPP', true)
endif

# set predefines whether psylib is compile with the following backends/options

if get_option('jack2') == true
    cdata.set('HAVE_JACK2', true)
endif

if get_option('alsa')
    cdata.set('HAVE_ALSA', true)
endif

if get_option('portaudio')
    cdata.set('HAVE_PORTAUDIO', true)
endif

configure_file (
    input         : 'psy-config.h.in',
    output        : 'psy-config.h',
    configuration : cdata
)

config_incdirs = include_directories('.')
libpsy_incdirs = include_directories(
    'external_libs',
    'jack',
    'gl',
    'hw',
    'backend_gtk',
    'portaudio',
)


libpsy_headers = files(
    'psy-artist.h',
    'psy-audio-channel-map.h',
    'psy-audio-device.h',
    'psy-audio-mixer.h',
    # 'psy-audio-output-mixer.h',
    'psy-audio-utils.h',
    'psy-auditory-stimulus.h',
    'psy-canvas.h',
    'psy-circle.h',
    'psy-circle-artist.h',
    'psy-clock.h',
    'psy-color.h',
    'psy-cross-artist.h',
    'psy-cross.h',
    'psy-drawing-context.h',
    'psy-duration.h',
    'psy-enums.h',
    'psy-font-utils.h',
    'psy-gst-stimulus.h',
    'psy-image.h',
    'psy-image-canvas.h',
    'psy-loop.h',
    'psy-matrix4.h',
    'psy-picture-artist.h',
    'psy-picture.h',
    'psy-program.h',
    'psy-queue.h',
    'psy-rectangle.h',
    'psy-rectangle-artist.h',
    'psy-shader.h',
    'psy-step.h',
    'psy-stepping-stones.h',
    'psy-canvas.h',
    'psy-stimulus.h',
    'psy-text.h',
    'psy-text-artist.h',
    'psy-texture.h',
    'psy-time-point.h',
    'psy-trial.h',
    'psy-utils.h',
    'psy-vbuffer.h',
    'psy-vector.h',
    'psy-vector3.h',
    'psy-vector4.h',
    'psy-visual-stimulus.h',
    'psy-wave.h',
    'psy-widget.h',
    'psy-window.h',
    'psy-window-toy.h',
)

libpsy_header_private = files(
    'psy-safe-int-private.h',
    'psy-vector3-private.h'
)

libpsyfiles = files (
    'psy-artist.c',
    'psy-audio-channel-map.c',
    'psy-audio-device.c',
    'psy-audio-mixer.c',
    # 'psy-audio-output-mixer.c',
    'psy-audio-utils.c',
    'psy-auditory-stimulus.c',
    'psy-canvas.c',
    'psy-circle.c',
    'psy-circle-artist.c',
    'psy-clock.c',
    'psy-color.c',
    'psy-cross-artist.c',
    'psy-cross.c',
    'psy-drawing-context.c',
    'psy-duration.c',
    'psy-font-utils.c',
    'psy-gst-stimulus.c',
    'psy-image.c',
    'psy-image-canvas.c',
    'psy-loop.c',
    'psy-matrix4.cpp',
    'psy-picture.c',
    'psy-picture-artist.c',
    'psy-program.c',
    'psy-queue.cpp',
    'psy-random.c',
    'psy-rectangle.c',
    'psy-rectangle-artist.c',
    'psy-safe-int-private.c',
    'psy-shader.c',
    'psy-step.c',
    'psy-stepping-stones.c',
    'psy-stimulus.c',
    'psy-text.c',
    'psy-text-artist.c',
    'psy-texture.c',
    'psy-time-point.c',
    'psy-trial.c',
    'psy-utils.c',
    'psy-vbuffer.c',
    'psy-vector.c',
    'psy-vector3.cpp',
    'psy-vector4.cpp',
    'psy-visual-stimulus.c',
    'psy-wave.c',
    'psy-widget.c',
    'psy-window.c',
    'psy-window-toy.c',
)

subdir('gl')
subdir('backend_gtk')
subdir('hw')

if get_option('jack2')
    subdir('jack')
endif

if get_option('portaudio')
    subdir('portaudio')
endif


enum_files = gnome.mkenums_simple('enum-types', sources : libpsy_headers)

configure_file(input : 'v-shader.vert',
               output: 'v-shader.vert',
               copy : true)
configure_file(input : 'f-shader.frag',
               output: 'f-shader.frag',
               copy : true)
configure_file(input : 'uniform-color.vert',
               output: 'uniform-color.vert',
               copy : true)
configure_file(input : 'uniform-color.frag',
               output: 'uniform-color.frag',
               copy : true)
configure_file(input : 'picture.vert',
               output: 'picture.vert',
               copy : true)
configure_file(input : 'picture.frag',
               output: 'picture.frag',
               copy : true)

#preprocessor args
PP_ARGS = [
    '-DG_LOG_DOMAIN="Psy"',
    '-DG_LOG_USE_STRUCTURED'
]

buildtype = get_option('buildtype')
if buildtype != 'debug' and buildtype != 'debugoptimized'
    # flags for release builds (disable asserts)
    PP_ARGS += '-DG_DISABLE_ASSERTS=1'
    PP_ARGS += '-DNDEBUG=1'
endif

libpsy = library (
    'psy',
    libpsyfiles + enum_files,
    dependencies : psy_deps,
    include_directories : [libpsy_incdirs, config_incdirs],
    c_args : PP_ARGS,
    cpp_args : PP_ARGS,
    install: true
)

libpsy_dep = declare_dependency(
    link_with : libpsy,
    include_directories : [libpsy_incdirs, config_incdirs],
    dependencies : psy_deps 
)

gir = gnome.generate_gir(
    libpsy,
    sources : libpsyfiles + libpsy_headers + enum_files,
    namespace : 'Psy',
    nsversion : meson.project_version(),
    identifier_prefix : 'Psy',
    export_packages : 'psy',
    symbol_prefix : 'psy',
    includes : ['GObject-2.0', 'GLib-2.0', 'Gio-2.0'],
    include_directories : '/opt/gtk/share/gir-1.0',
    dependencies : [libpsy_dep],
#    header : libpsy_public_header,
    install : true,
    extra_args : ['--warn-all'],
    fatal_warnings : true
)
gir_target = gir[0]
typelib_target = gir[1]


