project('libpsy',
    ['c', 'cpp'],
    version : '0.1',
    default_options : [
        'warning_level=3',
#        'werror=true',
        'buildtype=debugoptimized',
        'c_std=gnu11', # this makes it easier to compile sub projects
        'cxx_std=gnu11' # this makes it easier to compile sub projects
    ],
    license : 'MIT'
)

cc = meson.get_compiler('c')

m_dep = cc.find_library('m', required : false)

# dependency version requirements
glib_req = '>=2.66.0' # based on what gtk requires august 2022
gtk_req = '>4.0' # based on what gtk requires august 2022

glib_dep = dependency(
    'glib-2.0',
    version: glib_req,
    fallback: ['glib', 'libglib_dep'],
    default_options : [
        'tests=false'
    ]
)

gobject_dep = dependency(
    'gobject-2.0',
    version: glib_req,
    fallback: ['glib', 'libgobject_dep'],
    default_options : [
        'tests=false'
    ]
)

gio_dep = dependency(
    'gio-2.0',
    version: glib_req,
    fallback: ['glib', 'libgio_dep'],
    default_options : [
        'tests=false'
    ]
)

gtk_dep = dependency(
    'gtk4',
    version : gtk_req,
    fallback:['gtk', 'gtk_dep'],
    default_options:[
        'warning_level=0',          # match gtk's internal warning level
#        'werror=false',             # No point in stopping if gtk-build fails on warnings
        'wayland-backend=false',    # difficult to build on 20.04: wayland-protocols found: NO found 1.20 but need: '>= 1.25'
        'media-gstreamer=disabled', # idem
        'build-tests=false'
    ]
)

# OpenGL function loader helper
epoxy_dep = dependency('epoxy', version : '>1.0')

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

if get_option('documentation')
    subdir('doc')
endif


