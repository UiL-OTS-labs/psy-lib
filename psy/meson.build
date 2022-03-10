
# Directory for external libraries for used to build
# libpsy.

cdata = configuration_data({
    'PSY_VERSION'                   : meson.project_version()
})

if cc.has_function('__builtin_add_overflow')
    cdata.set('HAVE_BUILTIN_ADD_OVERFLOW', 1)
endif
if cc.has_function('__builtin_sub_overflow')
    cdata.set('HAVE_BUILTIN_SUB_OVERFLOW', 1)
endif
if cc.has_function('__builtin_mul_overflow')
    cdata.set('HAVE_BUILTIN_MUL_OVERFLOW', 1)
endif


configure_file (
    input         : 'psy-config.h.in',
    output        : 'psy-config.h',
    configuration : cdata
)

config_incdirs= include_directories('.')
libpsy_incdirs= include_directories('external_libs')


libpsy_headers = files(
    'psy-clock.h',
    'psy-duration.h',
    'psy-gl-error.h',
    'psy-gl-fragment-shader.h',
    'psy-gl-program.h',
    'psy-gl-shader.h',
    'psy-gl-texture.h',
    'psy-gl-vbuffer.h',
    'psy-loop.h',
    'psy-matrix4.h',
    'psy-program.h',
    'psy-shader.h',
    'psy-step.h',
    'psy-stepping-stones.h',
    'psy-stimulus.h',
    'psy-texture.h',
    'psy-time-point.h',
    'psy-trial.h',
    'psy-vbuffer.h',
    'psy-vector.h',
    'psy-vector4.h',
    'psy-widget.h',
    'psy-window.h',
)

libpsy_header_private = files(
    'psy-safe-int-private.h'
)

libpsyfiles = files (
    'psy-clock.c',
    'psy-duration.c',
    'psy-gl-error.c',
    'psy-gl-fragment-shader.c',
    'psy-gl-program.c',
    'psy-gl-shader.c',
    'psy-gl-texture.c',
    'psy-gl-vbuffer.c',
    'psy-gl-vertex-shader.c',
    'psy-gl-vertex-shader.h',
    'psy-loop.c',
    'psy-matrix4.cpp',
    'psy-program.c',
    'psy-safe-int-private.c',
    'psy-shader.c',
    'psy-step.c',
    'psy-stepping-stones.c',
    'psy-stimulus.c',
    'psy-texture.c',
    'psy-time-point.c',
    'psy-trial.c',
    'psy-vbuffer.c',
    'psy-vector.c',
    'psy-vector4.cpp',
    'psy-widget.c',
    'psy-window.c',
)

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

libpsy = library (
    'psy',
    libpsyfiles + enum_files,
    dependencies : [gtk_dep, epoxy_dep, m_dep],
    include_directories : [libpsy_incdirs, config_incdirs],
    c_args : '-DG_LOG_DOMAIN="Psy"'
)



