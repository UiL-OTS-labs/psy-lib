project('libpsy',
    ['c', 'cpp'],
    version : '0.1',
    default_options : [
        'warning_level=3',
        'werror=true',
        'buildtype=debugoptimized',
        'c_std=c11'
    ],
    license : 'MIT'
)

cc = meson.get_compiler('c')

m_dep = cc.find_library('m', required : false)

# dependency version requirements
glib_req = '>=2.66.0' # based on what gtk requires august 2022
gtk_req = '>4.0' # based on what gtk requires august 2022

glib_dep = dependency(
    'glib',
    version: glib_req,
    fallback: ['glib', 'glib_dep'],
    default_options : [
        'tests=false'
    ]
)
gobject_dep = dependency(
    'glib',
    version: glib_dep,
    fallback: ['glib', 'gobject_dep'],
    default_options : [
        'tests=false' # currently some test fails
    ]
)
gio_dep = dependency(
    'glib',
    version: glib_dep,
    fallback: ['glib', 'gio_dep'],
    default_options : [
        'tests=false'
    ]
)

gtk_dep = dependency(
    'gtk4',
    version : gtk_req,
    fallback:['gtk', 'gtk_dep'],
    default_options:[
        'wayland-backend=false',    # difficult to build on 20.04: wayland-protocols found: NO found 1.20 but need: '>= 1.25'
        'media-gstreamer=disabled'  # idem 
    ]
)

epoxy_dep = dependency('epoxy', version : '>1.0')

exe = executable(
    'gtk_hello',
    files('gtk-hello.c'),
    install : true,
    dependencies : [gtk_dep]
)

gnome = import('gnome')

subdir('psy')
subdir('share')

psy_include = include_directories('psy')
psy_dep = declare_dependency(
    link_with : libpsy,
    include_directories : psy_include,
    dependencies : [gtk_dep, epoxy_dep, m_dep]
)

subdir('tests')

test('basic', exe)

