
config = configuration_data()
config.set_quoted('PSY_BUILD_PSY_DIR', meson.project_source_root() + '/psy')

configure_file(
    input:'monitor_shader_paths.h.in',
    output:'monitor_shader_paths.h',
    configuration: config)

mon_utils_sources = files(
    'cmd_enum_opts.c',
    'cmd_enum_opts.h',
    'cmd_vsync_opts.c',
    'cmd_vsync_opts.h',
    'frame-stats.c',
    'frame-stats.h',
    'stimulus.c',
    'stimulus.h',
)


mon_utils = static_library('mon_utils', mon_utils_sources, dependencies: psy_dep)

if sdl3_dep.found()
    incdir = include_directories('.')
    sdl_vsync = executable(
        'sdl_vsync',
        files('sdl_vsync.c'),
        dependencies : [psy_dep],
        link_with: mon_utils,
        include_directories : incdir
    )
    sdl_enumerate_monitors = executable(
        'sdl_enumerate_monitors',
        files('sdl_enumerate_monitors.c'),
        link_with: mon_utils,
        dependencies : [psy_dep]
    )
endif
