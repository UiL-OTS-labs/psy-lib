
cc = meson.get_compiler('c')
cunit_dep = dependency('cunit')


window_prog = executable (
    '3dwindow_prog',
    files('window-prog.c'),
    dependencies : [psy_dep]
)

test_gtk_window = executable(
    'test_gtk_window',
    files('test-gtk-window.c'),
    dependencies : [psy_dep]
)

test_image_canvas = executable(
    'test_image_canvas',
    files('test-image-canvas.c'),
    dependencies : [psy_dep]
)

parallel_write = executable(
    'parallel_write',
    files('parallel_write.c'),
    dependencies : [psy_dep]
)

parallel_trigger = executable(
    'parallel_trigger',
    files('parallel_trigger.c'),
    dependencies : [psy_dep]
)

picture_test = executable(
    'picture_test',
    files('picture-test.c'),
    dependencies : [psy_dep]
)

unit_test = executable (
    'unit-test',
    files (
        'main.c',
        'test-canvas.c',
        'test-color.c',
        'test-image.c',
        'test-gl-canvas.c',
        'test-parallel.c',
        'test-picture.c',
        'test-ref-count.c',
        'test-stepping.c',
        'test-text.c',
        'test-time-utilities.c',
        'test-visual-stimulus.c',
        'unit-test-utilities.c',
        'vector-test.c',
        'vector3-test.c',
        'vector4-test.c',
    ),
    dependencies : [psy_dep, cunit_dep]
)

test('unit-test', unit_test)
