
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

config_incdirs = include_directories('.')
libpsy_incdirs = include_directories(
    'external_libs',
    'gl',
    'backend_gtk',
)


libpsy_headers = files(
    'psy-artist.h',
    'psy-circle.h',
    'psy-circle-artist.h',
    'psy-clock.h',
    'psy-color.h',
    'psy-cross-artist.h',
    'psy-cross.h',
    'psy-drawing-context.h',
    'psy-duration.h',
    'psy-enums.h',
    'psy-loop.h',
    'psy-matrix4.h',
    'psy-program.h',
    'psy-rectangle.h',
    'psy-rectangle-artist.h',
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
    'psy-visual-stimulus.h',
    'psy-widget.h',
    'psy-window.h',
    'psy-window-toy.h',
)

libpsy_header_private = files(
    'psy-safe-int-private.h'
)

libpsyfiles = files (
    'psy-artist.c',
    'psy-circle.c',
    'psy-circle-artist.c',
    'psy-clock.c',
    'psy-color.c',
    'psy-cross-artist.c',
    'psy-cross.c',
    'psy-drawing-context.c',
    'psy-duration.c',
    'psy-loop.c',
    'psy-matrix4.cpp',
    'psy-program.c',
    'psy-rectangle.c',
    'psy-rectangle-artist.c',
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
    'psy-visual-stimulus.c',
    'psy-widget.c',
    'psy-window.c',
    'psy-window-toy.c',
)

subdir('gl')
subdir('backend_gtk')
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
    c_args : '-DG_LOG_DOMAIN="Psy"',
    install: true
)

libpsy_dep = declare_dependency(
    link_with : libpsy,
    include_directories : [libpsy_incdirs, config_incdirs],
    dependencies : [glib_dep, gobject_dep, gtk_dep, epoxy_dep, m_dep]
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


