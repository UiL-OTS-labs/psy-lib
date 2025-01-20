
config = configuration_data()
config.set_quoted('PSY_BUILD_PSY_DIR', meson.project_source_root() + '/psy')

configure_file(
    input:'monitor_shader_paths.h.in',
    output:'monitor_shader_paths.h',
    configuration: config)

if sdl3_dep.found()
    incdir = include_directories('.')
    sdl_vsync = executable(
        'sdl_vsync',
        files('sdl_vsync.c', 'cmd_vsync_opts.c'),
        dependencies : psy_dep,
        include_directories : incdir)
    sdl_enumerate_monitors = executable(
        'sdl_enumerate_monitors',
        files('sdl_enumerate_monitors.c', 'cmd_enum_opts.c'),
        dependencies : psy_dep
    )
endif
