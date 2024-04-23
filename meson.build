project('libpsy',
    ['c', 'cpp'],
    version : '0.1',
    default_options : [
        'warning_level=3',
#        'werror=true',
        'buildtype=debugoptimized',
        'c_std=gnu11,c11', # this makes it easier to compile sub projects
        'cpp_std=gnu++11,c++11' # use c++ 11
    ],
    license : 'MIT'
)

# dependency version requirements
#
glib_req = '>=2.66.0' # based on what gtk requires august 2022
gtk_req = '>4.0' # based on what gtk requires august 2022
alsa_req = '>=1.2'
jack2_req = '>=1.9'
gstreamer_req = '>=1.18'
portaudio_req = '>=19'

# dependencies

cc = meson.get_compiler('c')
cxx = meson.get_compiler('cpp')

m_dep = cc.find_library('m', required : false)

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

gstreamer_dep = dependency(
    'gstreamer-1.0',
    version : gstreamer_req,
)

gstreamer_app_dep = dependency(
    'gstreamer-app-1.0',
    version : gstreamer_req
  )

gtk_dep = dependency(
    'gtk4',
    version: gtk_req,
    fallback: ['gtk', 'gtk_dep'],
    default_options: [
        'warning_level=0',          # match gtk's internal warning level
#        'werror=false',             # No point in stopping if gtk-build fails on warnings
        'wayland-backend=false',    # difficult to build on 20.04: wayland-protocols found: NO found 1.20 but need: '>= 1.25'
        'media-gstreamer=disabled', # idem
        'build-tests=false'
    ]
)

# OpenGL function loader helper
epoxy_dep = dependency('epoxy', version : '>1.0')

cairo_dep = dependency('cairo', version : '>1.0')
pango_dep = dependency('pango', version : '>1.0')
pangocairo_dep = dependency('pangocairo', version : '>1.0')

psy_deps = [
    glib_dep,
    gobject_dep,
    gtk_dep,
    epoxy_dep,
    gstreamer_dep,
    gstreamer_app_dep
]

# add optional dependencies

if get_option('alsa')
    message('Using alsa')
    alsa_dep = dependency (
        'alsa',
        version : alsa_req,
        required : true
    )
    psy_deps += alsa_dep
endif

if get_option('jack2')
    message('Using jack')
    jack2_dep = dependency (
        'jack',
        version : jack2_req,
        required : true
    )
    psy_deps += jack2_dep
endif

if get_option('portaudio')
    message('Using portaudio')
    portaudio_dep = dependency (
        'portaudio-2.0',
        version : portaudio_req,
        required : true
    )
    psy_deps += portaudio_dep
endif

message('the depencies are: ')
foreach dep : psy_deps
    message(' - ', dep.name())
endforeach

psy_deps += m_dep

gnome = import('gnome')

subdir('psy')
subdir('share')

psy_include = include_directories('psy')
psy_dep = declare_dependency(
    link_with : libpsy,
    include_directories : psy_include,
    dependencies : psy_deps
)

subdir('tests')
subdir('programs')

if get_option('build-examples')
    subdir('examples')
endif

if get_option('documentation')
    subdir('doc')
endif


