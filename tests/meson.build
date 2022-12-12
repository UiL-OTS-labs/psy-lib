
cc = meson.get_compiler('c')
cunit_dep = dependency('cunit')


window_prog = executable (
    '3dwindow_prog',
    files('window-prog.c'),
    dependencies : [psy_dep]
)

vector4_test=executable(
    'vector4-test',
    'vector4-test.c',
    dependencies : [psy_dep]
)

test_gtk_window = executable(
    'test-gtk-window',
    'test-gtk-window.c',
    dependencies : [psy_dep]
)


unit_test = executable (
    'unit-test',
    files (
        'main.c',
        'test-color.c',
        'test-ref-count.c',
        'test-stepping.c',
        'test-time-utilities.c',
        'vector-test.c'
    ),
    dependencies : [psy_dep, cunit_dep]
)

test('vector4_test', vector4_test)
test('unit-test', unit_test)
