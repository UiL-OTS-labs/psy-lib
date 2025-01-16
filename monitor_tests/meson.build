
if sdl3_dep.found()
    sdl_vsync = executable(
        'sdl_vsync',
        files('sdl_vsync.c', 'cmd_vsync_opts.c'),
        dependencies : psy_dep)
    sdl_enumerate_monitors = executable(
        'sdl_enumerate_monitors',
        files('sdl_enumerate_monitors.c', 'cmd_enum_opts.c'),
        dependencies : psy_dep
    )
endif
