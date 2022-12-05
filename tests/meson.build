
cc = meson.get_compiler('c')
cunit_dep = dependency('cunit')


window_prog = executable (
    '3dwindow_prog',
    files('window-prog.c'),
    dependencies : [psy_dep]
)

vector_test=executable(
    'vector-test',
    'vector-test.c',
    dependencies : [psy_dep]
)

vector4_test=executable(
    'vector4-test',
    'vector4-test.c',
    dependencies : [psy_dep]
)

test_stepping = executable(
    'test-stepping',
    'test-stepping.c',
    dependencies : [psy_dep]
)

test_time_utilities = executable(
    'test-time-utilities',
    'test-time-utilities.c',
    dependencies : [psy_dep]
)

test_gtk_window = executable(
    'test-gtk-window',
    'test-gtk-window.c',
    dependencies : [psy_dep]
)

test_color = executable (
    'test-color',
    'test-color.c',
    dependencies : [psy_dep, mutest_dep]
)

test_ref_count = executable (
    'test-ref-count',
    'test-ref-count.c',
    dependencies : [psy_dep, cunit_dep]
)

test('vector test', vector_test)
test('vector4_test', vector4_test)
test('test-stepping', test_stepping)
test('test-time-utilities', test_time_utilities)
test('test-color', test_color)
test('test-ref-count.c', test_color)
