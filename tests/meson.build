
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

test('vector test', vector_test)
test('vector4_test', vector4_test)
test('test-stepping', test_stepping)
test('test-time-utilities', test_time_utilities)
