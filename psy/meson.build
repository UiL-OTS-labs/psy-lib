
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
    input         : 'ddd-config.h.in',
    output        : 'ddd-config.h',
    configuration : cdata
)

config_incdirs= include_directories('.')
libpsy_incdirs= include_directories('external_libs')


libpsy_headers = files(
    'ddd-clock.h',
    'ddd-duration.h',
    'ddd-gl-error.h',
    'ddd-gl-fragment-shader.h',
    'ddd-gl-program.h',
    'ddd-gl-shader.h',
    'ddd-gl-texture.h',
    'ddd-gl-vbuffer.h',
    'ddd-loop.h',
    'ddd-matrix4.h',
    'ddd-program.h',
    'ddd-shader.h',
    'ddd-step.h',
    'ddd-stepping-stones.h',
    'ddd-stimulus.h',
    'ddd-texture.h',
    'ddd-time-point.h',
    'ddd-trial.h',
    'ddd-vbuffer.h',
    'ddd-vector.h',
    'ddd-vector4.h',
    'ddd-widget.h',
    'ddd-window.h',
)

libpsy_header_private = files(
    'ddd-safe-int-private.h'
)

libpsyfiles = files (
    'ddd-clock.c',
    'ddd-duration.c',
    'ddd-gl-error.c',
    'ddd-gl-fragment-shader.c',
    'ddd-gl-program.c',
    'ddd-gl-shader.c',
    'ddd-gl-texture.c',
    'ddd-gl-vbuffer.c',
    'ddd-gl-vertex-shader.c',
    'ddd-gl-vertex-shader.h',
    'ddd-loop.c',
    'ddd-matrix4.cpp',
    'ddd-program.c',
    'ddd-safe-int-private.c',
    'ddd-shader.c',
    'ddd-step.c',
    'ddd-stepping-stones.c',
    'ddd-stimulus.c',
    'ddd-texture.c',
    'ddd-time-point.c',
    'ddd-trial.c',
    'ddd-vbuffer.c',
    'ddd-vector.c',
    'ddd-vector4.cpp',
    'ddd-widget.c',
    'ddd-window.c',
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



