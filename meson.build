project('gtk_try_out', ['c', 'cpp'],
    version : '0.1',
    default_options : [
        'warning_level=3',
        'werror=true',
        'buildtype=debugoptimized',
        'c_std=c11'
    ]
)

cc = meson.get_compiler('c')

m_dep = cc.find_library('m', required : false)

gtk_dep = dependency('gtk4', version : '>4.0')
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

