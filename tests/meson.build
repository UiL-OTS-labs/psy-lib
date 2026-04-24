
cc = meson.get_compiler('c')
cunit_dep = dependency('cunit')

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

timer_test = executable(
    'timer-test',
    files('timer-test.c'),
    dependencies:[psy_dep]
)
test('timer-test', timer_test)



timer_cancellation_no_loop = executable(
    'timer-cancellation-no-loop',
    'timer-cancellation-no-loop.c',
    dependencies: [psy_dep],
    c_args: UNIT_PP_ARGS,
    cpp_args: UNIT_PP_ARGS,
)
env = environment({'G_DEBUG':'fatal-warnings'})
test(
    'timer-cancellation-no-loop',
    timer_cancellation_no_loop,
    env:env
)

test_audio = executable(
    'test-audio',
    files ('unit-test-utilities.c', 'test-audio.c'),
    dependencies: [psy_dep, criterion_dep],
    c_args : UNIT_PP_ARGS,
    cpp_args : UNIT_PP_ARGS,
)

# One can only open an audio device once
test('test-audio',
    test_audio,
    is_parallel : false,
    args: ['-j1'], # So also the criterion test should not run in parallel
)

test_audio_channel_mapping = executable(
    'test-audio-channel-mapping',
    files('test-audio-channel-mapping.c'),
    dependencies: [psy_dep, criterion_dep],
    c_args : UNIT_PP_ARGS,
    cpp_args : UNIT_PP_ARGS,
)

test('test-audio-channel-mapping', test_audio_channel_mapping,)

test_audio_utils = executable(
    'test-audio-utils',
    files('test-audio-utils.c', 'unit-test-utilities.c'),
    dependencies: [psy_dep, criterion_dep],
    c_args : UNIT_PP_ARGS,
    cpp_args : UNIT_PP_ARGS,
)
test('test-audio-utils', test_audio_utils,)

# perhaps skip this on windows for now
test_canvas = executable(
    'test-canvas',
    files('test-canvas.c', 'unit-test-utilities.c'),
    dependencies: [psy_dep, criterion_dep],
    c_args : UNIT_PP_ARGS,
    cpp_args : UNIT_PP_ARGS,
)
test('test-canvas', test_canvas,)

test_color = executable(
    'test-color',
    files('test-color.c'),
    dependencies: [psy_dep, criterion_dep],
    c_args : UNIT_PP_ARGS,
    cpp_args : UNIT_PP_ARGS,
)
test('test-color', test_color,)

#may skip on win32
test_cross = executable(
    'test-cross',
    files('test-cross.c', 'unit-test-utilities.c'),
    dependencies: [psy_dep, criterion_dep],
    c_args : UNIT_PP_ARGS,
    cpp_args : UNIT_PP_ARGS,
)
test('test-cross', test_cross,)

test_image = executable(
    'test-image',
    files('test-image.c', 'unit-test-utilities.c'),
    dependencies: [psy_dep, criterion_dep],
    c_args : UNIT_PP_ARGS,
    cpp_args : UNIT_PP_ARGS,
)
test('test-image', test_image,)

criterion_tests = executable(
    'criterion_tests',
    files(
        'unit-test-utilities.c',
        'test-init.c',
        'test-timer.c',
        'test-audio.c',
    ),
    dependencies: [psy_dep, criterion_dep],
    c_args : UNIT_PP_ARGS,
    cpp_args : UNIT_PP_ARGS,
)

test('criterion-tests', criterion_tests, args : ['-j1'])

unit_test = executable (
    'unit-test',
    files (
        'main.c',
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
        'test-utility.c',
        'test-wave.c',
        'test-visual-stimulus.c',
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
