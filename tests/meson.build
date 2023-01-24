
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

unit_test = executable (
    'unit-test',
    files (
        'main.c',
        'test-color.c',
        'test-parallel.c',
        'test-ref-count.c',
        'test-stepping.c',
        'test-time-utilities.c',
        'vector-test.c',
        'vector4-test.c'
    ),
    dependencies : [psy_dep, cunit_dep]
)

test('unit-test', unit_test)
