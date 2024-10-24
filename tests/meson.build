
cc = meson.get_compiler('c')
cunit_dep = dependency('cunit')

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

timer_test = executable('timer-test', files('timer-test.c'), dependencies:[psy_dep])
test('timer-test', timer_test)

#preprocessor args for the unit tests
UNIT_PP_ARGS = [
    '-DG_LOG_DOMAIN="Psy-Unit"',
    '-DG_LOG_USE_STRUCTURED'
]

buildtype = get_option('buildtype')
if buildtype != 'debug' and buildtype != 'debugoptimized'
    # flags for release builds (disable asserts)
    UNIT_PP_ARGS += '-DG_DISABLE_ASSERTS=1'
    UNIT_PP_ARGS += '-DNDEBUG=1'
endif

unit_test = executable (
    'unit-test',
    files (
        'main.c',
        'test-audio.c',
        'test-audio-channel-mapping.c',
        'test-audio-utils.c',
        'test-canvas.c',
        'test-color.c',
        'test-image.c',
        'test-gl-canvas.c',
        'test-gl-utils.c',
        'test-matrix4.c',
        'test-parallel.c',
        'test-picture.c',
        'test-queue.c',
        'test-ref-count.c',
        'test-stepping.c',
        'test-text.c',
        'test-time-utilities.c',
        'test-timer.c',
        'test-utility.c',
        'test-wave.c',
        'test-visual-stimulus.c',
        'test-visual-stimuli.c',
        'unit-test-utilities.c',
        'vector-test.c',
        'vector3-test.c',
        'vector4-test.c',
    ),
    dependencies : [psy_dep, cunit_dep],
    c_args : UNIT_PP_ARGS,
    cpp_args : UNIT_PP_ARGS,
)

# This runs the unit test verbosely and saves the pictures
# So we get them as artifact in the CI
test('unit-test', unit_test, args: ['-v', '-S', '-l', 'debug'])
