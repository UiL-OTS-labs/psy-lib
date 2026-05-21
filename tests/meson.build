
cc = meson.get_compiler('c')

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
    dependencies: [psy_dep],
    c_args : UNIT_PP_ARGS,
    cpp_args : UNIT_PP_ARGS,
)

# One can only open an audio device once
test('test-audio',
    test_audio,
    is_parallel : false,
    args: ['-j1'], # So also the criterion test should not run in parallel
    protocol: 'tap'
)

test_audio_channel_mapping = executable(
    'test-audio-channel-mapping',
    files('test-audio-channel-mapping.c', 'unit-test-utilities.c'),
    dependencies: [psy_dep],
    c_args : UNIT_PP_ARGS,
    cpp_args : UNIT_PP_ARGS,
)

test('test-audio-channel-mapping', test_audio_channel_mapping, protocol: 'tap')

test_audio_utils = executable(
    'test-audio-utils',
    files('test-audio-utils.c', 'unit-test-utilities.c'),
    dependencies: [psy_dep],
    c_args : UNIT_PP_ARGS,
    cpp_args : UNIT_PP_ARGS,
)
test('test-audio-utils', test_audio_utils, protocol: 'tap')

# perhaps skip this on windows for now
test_canvas = executable(
    'test-canvas',
    files('test-canvas.c'),
    dependencies: [psy_dep],
    c_args : UNIT_PP_ARGS,
    cpp_args : UNIT_PP_ARGS,
)
test('test-canvas', test_canvas, protocol: 'tap')

test_color = executable(
    'test-color',
    files('test-color.c'),
    dependencies: [psy_dep],
    c_args : UNIT_PP_ARGS,
    cpp_args : UNIT_PP_ARGS,
)
test('test-color', test_color, protocol: 'tap')


#may skip on win32
test_cross = executable(
    'test-cross',
    files('test-cross.c', 'unit-test-utilities.c'),
    dependencies: [psy_dep],
    c_args : UNIT_PP_ARGS,
    cpp_args : UNIT_PP_ARGS,
)
test('test-cross', test_cross, protocol: 'tap')

test_image = executable(
    'test-image',
    files('test-image.c', 'unit-test-utilities.c'),
    dependencies: [psy_dep],
    c_args : UNIT_PP_ARGS,
    cpp_args : UNIT_PP_ARGS,
)
test('test-image', test_image, protocol:'tap')

test_init = executable(
    'test-init',
    files('test-init.c', 'unit-test-utilities.c'),
    dependencies: [psy_dep],
    c_args : UNIT_PP_ARGS,
    cpp_args : UNIT_PP_ARGS,
)
test('test-init', test_init, protocol:'tap')

test_gl_canvas = executable(
    'test-gl-canvas',
    files('test-gl-canvas.c'),
    dependencies: [psy_dep],
    c_args : UNIT_PP_ARGS,
    cpp_args : UNIT_PP_ARGS,
)
test('test-gl-canvas', test_gl_canvas, protocol:'tap')
         
test_gl_utils = executable(
    'test-gl-utils',
    files('test-gl-utils.c'),
    dependencies: [psy_dep],
    c_args : UNIT_PP_ARGS,
    cpp_args : UNIT_PP_ARGS,
)
test('test-gl-utils', test_gl_utils, protocol: 'tap')
 
test_matrix4 = executable (
    'test-matrix4',
    files('test-matrix4.c',),
    dependencies: [psy_dep],
    c_args : UNIT_PP_ARGS,
    cpp_args : UNIT_PP_ARGS,
)
test('test-matrix4', test_matrix4, protocol: 'tap')

test_parallel = executable(
    'test-parallel',
    files('test-parallel.c',),
    dependencies: [psy_dep],
    c_args : UNIT_PP_ARGS,
    cpp_args : UNIT_PP_ARGS,
)
test('test-parallel', test_parallel, protocol: 'tap')
 
test_picture = executable (
    'test-picture',
    files('test-picture.c', 'unit-test-utilities.c'),
    dependencies: [psy_dep],
    c_args : UNIT_PP_ARGS,
    cpp_args : UNIT_PP_ARGS,
)
test('test-picture', test_picture, protocol: 'tap',is_parallel: false,)
 
test_queue = executable(
    'test-queue',
    files('test-queue.c',),
    dependencies: [psy_dep],
    c_args : UNIT_PP_ARGS,
    cpp_args : UNIT_PP_ARGS,
)
test('test-queue', test_queue,)

test_ref_count = executable('test-refcount',
    files('test-ref-count.c',),
    dependencies: [psy_dep],
    c_args : UNIT_PP_ARGS,
    cpp_args : UNIT_PP_ARGS,
)
test('test-ref-count', test_ref_count, protocol: 'tap')

test_stepping = executable(
    'test-stepping',
    files('test-stepping.c',),
    dependencies: [psy_dep],
    c_args : UNIT_PP_ARGS,
    cpp_args : UNIT_PP_ARGS,
)
test('test-stepping', test_stepping, protocol: 'tap')

test_text = executable(
    'test-text',
    files('test-text.c', 'unit-test-utilities.c',),
    dependencies: [psy_dep],
    c_args : UNIT_PP_ARGS,
    cpp_args : UNIT_PP_ARGS,
)
test('test-text', test_text, protocol: 'tap')

test_time_utilities = executable(
    'test-time-utilities',
    files('test-time-utilities.c',),
    dependencies: [psy_dep],
    c_args : UNIT_PP_ARGS,
    cpp_args : UNIT_PP_ARGS,
)
test('test-time-utilities', test_time_utilities, protocol: 'tap',)

test_timer = executable(
    'test-timer',
    files('test-timer.c', 'unit-test-utilities.c', ),
    dependencies: [psy_dep],
    c_args : UNIT_PP_ARGS,
    cpp_args : UNIT_PP_ARGS,
)
test('test-timer.c', test_timer, is_parallel: false, protocol: 'tap')

test_utilities = executable(
    'test-utilities',
    files('test-utility.c',),
    dependencies: [psy_dep],
    c_args : UNIT_PP_ARGS,
    cpp_args : UNIT_PP_ARGS,
)
test('test-utilities', test_utilities, protocol: 'tap',)

test_visual_stimulus = executable(
    'test-visual-stimulus',
    files('test-visual-stimulus.c', 'unit-test-utilities.c'),
    dependencies: [psy_dep],
    c_args : UNIT_PP_ARGS,
    cpp_args : UNIT_PP_ARGS,
)

test('test-visual-stimulus', test_visual_stimulus, protocol: 'tap')
 
test_wave = executable(
    'test-wave',
    files('test-wave.c', 'unit-test-utilities.c'),
    dependencies: [psy_dep],
    c_args : UNIT_PP_ARGS,
    cpp_args : UNIT_PP_ARGS,
)
test('test-wave', test_wave, is_parallel: false, protocol: 'tap')

# test_vector = executable(
#     'test-vector',
#     files('test-vector.c'),
#     dependencies: [psy_dep, criterion_dep],
#     c_args : UNIT_PP_ARGS,
#     cpp_args : UNIT_PP_ARGS,
# )
# test('test-vector', test_vector)
# 
# test_vector3 = executable(
#     'test-vector3',
#     files('test-vector3.c'),
#     dependencies: [psy_dep, criterion_dep],
#     c_args : UNIT_PP_ARGS,
#     cpp_args : UNIT_PP_ARGS,
# )
# test('test-vector3', test_vector3)
# 
# test_vector4 = executable(
#     'test-vector4',
#     files('test-vector4.c'),
#     dependencies: [psy_dep, criterion_dep],
#     c_args : UNIT_PP_ARGS,
#     cpp_args : UNIT_PP_ARGS,
# )
# test('test-vector4', test_vector4)

