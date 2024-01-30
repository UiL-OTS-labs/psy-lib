
#pragma once

/**
 * PsyAudioDeviceError:
 * @PSY_AUDIO_DEVICE_ERROR_NO_SERVER_CONNECTION: This error can occur when a
 *      jack client is unable to connect to a jack server, perhaps it should
 *      be started first.
 * @PSY_AUDIO_DEVICE_ERROR_BUSY: This error can occur when trying to connect
 *      to an audio device that is already in operation.
 * @PSY_AUDIO_DEVICE_ERROR_OPEN: This operation cannot be performed when the
 *      device is open.
 * @PSY_AUDIO_DEVICE_ERROR_OPEN_NAME: Unable to open a device with this name.
 * @PSY_AUDIO_DEVICE_ERROR_OPEN_NO_MATCH: Unable to open, no matching devices
 *      found.
 * @PSY_AUDIO_DEVICE_ERROR_FAILED: unspecific error read the error message
 *      for more info.
 *
 * An enumeration for errors that may result in operating an audio device
 */
typedef enum {
    PSY_AUDIO_DEVICE_ERROR_NO_SERVER_CONNECTION,
    PSY_AUDIO_DEVICE_ERROR_BUSY,
    PSY_AUDIO_DEVICE_ERROR_OPEN,
    PSY_AUDIO_DEVICE_ERROR_OPEN_NAME,
    PSY_AUDIO_DEVICE_ERROR_OPEN_NO_MATCH,
    PSY_AUDIO_DEVICE_ERROR_FAILED,
} PsyAudioDeviceError;

/**
 * PsyAudioSampleRate:
 * @PSY_AUDIO_SAMPLE_RATE_22050: A low quality sample rate (old MP3's)
 * @PSY_AUDIO_SAMPLE_RATE_24000:
 * @PSY_AUDIO_SAMPLE_RATE_32000: A quality sufficient for audio tapes FM radio
 * @PSY_AUDIO_SAMPLE_RATE_44100: The quality of CD audio
 * @PSY_AUDIO_SAMPLE_RATE_48000: The quality of DVD audio the default for psylib
 * @PSY_AUDIO_SAMPLE_RATE_88200:
 * @PSY_AUDIO_SAMPLE_RATE_96000: A high quality likely better than you need
 * @PSY_AUDIO_SAMPLE_RATE_192000: A very high quality likely much higher than
 * @PSY_AUDIO_SAMPLE_RATE_UNKNOWN: A unknown/handled samplerate by psylib
 * you need.
 *
 * The sample rates higher then 32000 can represent all audio waves that most
 * humans can hear, except for the young. The most used sample rates will
 * probably be PSY_AUDIO_SAMPLE_RATE_44100 and _48000, as these can represent
 * everything we can hear. You can go higher, but is it necessary???
 *
 * Info is taken at july 11th 2023 from the [audacity-wiki]
 * (https://manual.audacityteam.org/man/sample_rates.html)
 *
 * TODO // In python this might map to Psy.AudioSampleRate.48000 and python
 * doesn't like the .48000, hence we might need to prefix a R to get
 * Psy.AudioSampleRate.48000
 *
 * TODO convert the name of the enum to PsyAudioFrameRate as it is more accurate
 * according to the definition of Frame and Sample [audio-with-psylib]
 *
 * 22000, 88200 have been added as they are in PortAudio
 */
typedef enum {
    PSY_AUDIO_SAMPLE_RATE_22050   = 22050,
    PSY_AUDIO_SAMPLE_RATE_24000   = 24000,
    PSY_AUDIO_SAMPLE_RATE_32000   = 32000,
    PSY_AUDIO_SAMPLE_RATE_44100   = 44100,
    PSY_AUDIO_SAMPLE_RATE_48000   = 48000,
    PSY_AUDIO_SAMPLE_RATE_88200   = 88200,
    PSY_AUDIO_SAMPLE_RATE_96000   = 96000,
    PSY_AUDIO_SAMPLE_RATE_192000  = 192000,
    PSY_AUDIO_SAMPLE_RATE_UNKNOWN = -1
} PsyAudioSampleRate;

/**
 * PsyAudioChannelStrategy:
 * @PSY_AUDIO_CHANNEL_STRATEGY_NONE: When there are an equal number of inputs
 *     and outputs the intput-n is matched to output-n. However, when
 *     there is a mismatch between the number only channels
 *     0 to min(num_inputs, num_outputs) are mapped, hence some channels will
 *     not be mapped at all, and they are stripped from the final result.
 * @PSY_AUDIO_CHANNEL_STRATEGY_DUPLICATE_INPUTS: When a PsyAudioOutputDevice has
 *     more channels available that there are in the source audio, channels are
 *     duplicated in order to write to all outputs, mono audio signals will
 *     be present on both audio channels. For three output channels the mapping
 *     will be [left, right, left] for stereo output. So inputs will be
 *     duplicated when there are more output channels
 * @PSY_AUDIO_CHANNEL_STRATEGY_MIX_TRAILING_INPUTS: When the sound source has
 *     more output channels than input channels the trailing outputs of the
 *     source are summed with the channels already written. So writing a stereo
 *     signal to a mono PsyAudioOutputDevice will lead from mixing two channels
 *     [left, right] to one channel [left + right]
 * @PSY_AUDIO_CHANNEL_STRATEGY_CUSTOM:
 *     Choose your own mapping using e.g.
 *     [method@AuditoryStimulus.set_channel_map]
 * @PSY_AUDIO_CHANNEL_STRATEGY_DEFAULT: The default channel strategy is a
 *     the same as [enum@AudioChannelStrategy.DUPLICATE_INPUTS]
 *
 * With these flags you can specify the default behavior when there is a mis-
 * match in the number of AudioSource outputs and AudioSink inputs.
 * By default psylib will generate a mapping between each input channel
 * is mapped to the channel with the same number on the output. When there
 * are more output channels, you can choose to use inputs multiple times as
 * 1 output, this results in that you can mix your mono audio on both (or more)
 * speakers of an stereo audio output.
 */
typedef enum {
    PSY_AUDIO_CHANNEL_STRATEGY_NONE                = 0,
    PSY_AUDIO_CHANNEL_STRATEGY_DUPLICATE_INPUTS    = 1 << 0,
    PSY_AUDIO_CHANNEL_STRATEGY_MIX_TRAILING_INPUTS = 1 << 1,
    PSY_AUDIO_CHANNEL_STRATEGY_CUSTOM              = 1 << 2,
    PSY_AUDIO_CHANNEL_STRATEGY_DEFAULT
    = PSY_AUDIO_CHANNEL_STRATEGY_DUPLICATE_INPUTS
} PsyAudioChannelStrategy;

/**
 * PsyDrawingContextError:
 * @PSY_DRAWING_CONTEXT_ERROR_NAME_EXISTS: A resouce with that name has
 *      already been registered.
 * @PSY_DRAWING_CONTEXT_ERROR_BUSY: Can't do this operation as it is already
 *      in progress
 * @PSY_DRAWING_CONTEXT_ERROR_NAME_FAILED: Some less specified error regarding
 *      the context occured.
 *
 * These errors may be the result of invalid operations on an instance
 * of `PsyDrawingContext`
 */
typedef enum {
    PSY_DRAWING_CONTEXT_ERROR_NAME_EXISTS,
    PSY_DRAWING_CONTEXT_ERROR_BUSY,
    PSY_DRAWING_CONTEXT_ERROR_FAILED
} PsyDrawingContextError;

/**
 * PsyGlError:
 * @PSY_GL_ERROR_SHADER_COMPILE: Unable to compile shader
 * @PSY_GL_ERROR_PROGRAM_LINK: Unable to link program,
 * @PSY_GL_ERROR_INVALID_ENUM: glError returned GL_INVALID_ENUM
 * @PSY_GL_ERROR_INVALID_VALUE: glError returned GL_INVALID_VALUE
 * @PSY_GL_ERROR_INVALID_OPERATION: glError returned GL_INVALID_OPERATION
 * @PSY_GL_ERROR_INVALID_FRAMEBUFFER_OPERATION: glError returned
 * GL_INVALID_FRAMEBUFFER_OPERATION
 * @PSY_GL_ERROR_OUT_OF_MEMORY: glError returned GL_OUT_OF_MEMORY
 * @PSY_GL_ERROR_STACK_UNDERFLOW: glError returned GL_STACK_UNDERFLOW
 * @PSY_GL_ERROR_STACK_OVERFLOW: glError returned GL_STACK_OVERFLOW
 *
 * An operation related to OpenGL failed.
 */
typedef enum {
    PSY_GL_ERROR_SHADER_COMPILE,
    PSY_GL_ERROR_PROGRAM_LINK,

    PSY_GL_ERROR_INVALID_ENUM,
    PSY_GL_ERROR_INVALID_VALUE,
    PSY_GL_ERROR_INVALID_OPERATION,
    PSY_GL_ERROR_INVALID_FRAMEBUFFER_OPERATION,
    PSY_GL_ERROR_OUT_OF_MEMORY,
    PSY_GL_ERROR_STACK_UNDERFLOW,
    PSY_GL_ERROR_STACK_OVERFLOW
} PsyGlError;

/**
 * PsyImageFormat:
 * @PSY_IMAGE_FORMAT_LUM:         a 1 byte per pixel corresponding with
 *                                luminance each pixels is in the range 0-255.
 * @PSY_IMAGE_FORMAT_RGB:         a 3 byte per pixel corresponding with RGB
 *                                luminance
 * @PSY_IMAGE_FORMAT_RGBA:        A 4 byte per pixel corresponding with the
 *                                three channels for RGB and an alpha channel.
 * @PSY_IMAGE_FORMAT_INVALID:     Don't use, it is returned on a error.
 *
 * This enumeration may be used to determine the image format of the
 * If a channel for, red, green, blue or alpha exists it will be a 8 bit
 * quantity ranging [0-255]. Generally, there are 8 bits per channel, however,
 * in some cases https://www.cairograhpics.org is used to draw on a stimulus.
 * Cairo has it's own image formats, notably in the RGB24 case a pixel still
 * occupies 32 bits, but ignore the first 8 bits or or first byte.
 */
typedef enum {
    PSY_IMAGE_FORMAT_LUM,
    PSY_IMAGE_FORMAT_RGB,
    PSY_IMAGE_FORMAT_RGBA,
    PSY_IMAGE_FORMAT_INVALID,
} PsyImageFormat;

/**
 * PsyIoDirection:
 * @PSY_IO_DIRECTION_INPUT: The device is/should be configured as input
 * @PSY_IO_DIRECTION_OUTPUT: The device is/should be configured as output
 *
 * These values may be used to configure a device as in- or output.
 */
typedef enum PsyIoDirection {
    PSY_IO_DIRECTION_IN,
    PSY_IO_DIRECTION_OUT,
} PsyIoDirection;

/**
 * PsyIoLevel:
 * @PSY_IO_LEVEL_HIGH: The logical high level of an signal
 * @PSY_IO_LEVEL_LOW: The logical low level of an signal
 *
 * These values may be used to configure a line etc to have a
 * high or low level voltage, it depends on the device what is the precise
 * level.
 */
typedef enum PsyIoLevel {
    PSY_IO_LEVEL_LOW,
    PSY_IO_LEVEL_HIGH,
} PsyIoLevel;

/**
 * PsyLoopCondition:
 * @PSY_LOOP_CONDITION_LESS:  The loop continues while :
 *                            `PsyLoop:index` < `PsyLoop:stop`
 * @PSY_LOOP_CONDITION_LESS_EQUAL: The loop continues while
 *                                 `PsyLoop:index` <= `PsyLoop:stop`
 * @PSY_LOOP_CONDITION_EQUAL: The loop conintues while
 *                            `PsyLoop:index` == `PsyLoop:stop`
 * @PSY_LOOP_CONDITION_GREATER_EQUAL: The loop continues while:
 *                                    `PsyLoop:index` >= `PsyLoop:stop`
 * @PSY_LOOP_CONDITION_GREATER: The loop continues while:
 *                              `PsyLoop:index` > `PsyLoop:stop`
 *
 * An enumeration to specify when to terminate an `PsyLoop`
 * PsyLoops have an internal PsyLoop:index, PsyLoop:stop and PsyLoop:increment.
 * The PsyLoop:index is incremented with PsyLoop:increment until a comparison
 * of index and stop isn't met anymore. This enumeration determines the
 * comparison.
 */
typedef enum {
    PSY_LOOP_CONDITION_LESS,
    PSY_LOOP_CONDITION_LESS_EQUAL,
    PSY_LOOP_CONDITION_EQUAL,
    PSY_LOOP_CONDITION_GREATER_EQUAL,
    PSY_LOOP_CONDITION_GREATER
} PsyLoopCondition;

/**
 * PsyParallelPortError:
 * @PSY_PARALLEL_PORT_ERROR_OPEN: Unable to open the device
 * @PSY_PARALLEL_PORT_ERROR_DEV_CLOSED: Unable to perform action on a device
 *     that isn't open yet.
 * @PSY_PARALLEL_PORT_ERROR_DIRECTION: Unable to perform action because the
 *     devices is not configured in the desired direction for the action
 *     e.g. writing to a deviced configured as #PSY_IO_DIRECTION_INPUT
 * @PSY_PARALLEL_PORT_ERROR_FAILED: Operation failed (check error message?).
 */
typedef enum {
    PSY_PARALLEL_PORT_ERROR_OPEN,
    PSY_PARALLEL_PORT_ERROR_DEV_CLOSED,
    PSY_PARALLEL_PORT_ERROR_DIRECTION,
    PSY_PARALLEL_PORT_ERROR_FAILED,
} PsyParallelPortError;

/**
 * PsyParallelTriggerError:
 * @PSY_PARALLEL_TRIGGER_ERROR_BUSY: This error is thrown when you are trying
 *                                   to schedule a new trigger when the old
 *                                   one hasn't finished yet.
 * @PSY_PARALLEL_TRIGGER_ERROR_INVALID_PARAMETER:
 *                                   This error isn't currently used. But
 *                                   could in the future be necessary.
 * @PSY_PARALLEL_TRIGGER_ERROR_FAILED:
 *                                   A unspecific error occurred when using the
 *                                   parallel port, you should check the message
 *                                   of the error to get an idea of what is
 *                                   failing.
 *
 * This enum is used for errors while operating the parallelport as a trigger
 * interface.
 */
typedef enum {
    PSY_PARALLEL_TRIGGER_ERROR_BUSY,
    PSY_PARALLEL_TRIGGER_ERROR_INVALID_PARAMETER,
    PSY_PARALLEL_TRIGGER_ERROR_FAILED,
} PsyParallelTriggerError;

/**
 * PsySteppingStoneError:
 * @PSY_STEPPING_STONES_ERROR_KEY_EXISTS:Unable to add step, because the name
 * already exits.
 * @PSY_STEPPING_STONES_ERROR_INVALID_INDEX: Trying to activate a step with an
 * invalid index.
 * @PSY_STEPPING_STONES_ERROR_NO_SUCK_KEY:Cannot activate a key with the
 * given name as it has not been added to this `SteppingStones` instance
 *
 * Errors that may occur on some operations on `PsySteppingStones` instances.
 */
typedef enum {
    PSY_STEPPING_STONES_ERROR_KEY_EXISTS,
    PSY_STEPPING_STONES_ERROR_INVALID_INDEX,
    PSY_STEPPING_STONES_ERROR_NO_SUCH_KEY
} PsySteppingStoneError;

/**
 * PsyTextureError:
 * @PSY_TEXTURE_ERROR_DECODE:Error occured because we were not able to decode
 * the image.
 * @PSY_TEXTURE_ERROR_FAILED:Another error regarding the texture occurred.
 */
typedef enum {
    PSY_TEXTURE_ERROR_DECODE, // failed to decode the texture.
    PSY_TEXTURE_ERROR_FAILED
} PsyTextureError;

/**
 * PsyWindowProjectionStyle:
 * @PSY_CANVAS_PROJECTION_STYLE_C: The origin is in the upper left corner of the
 *                                 window. With positive y coordinates going
 * down.
 * @PSY_CANVAS_PROJECTION_STYLE_CENTER: The origin is in the center of the
 * window with positive y coordinates is going up.
 * @PSY_CANVAS_PROJECTION_STYLE_PIXELS: Create a projection matrix that makes
 * the screen as wide,tall as the dimensions of the number of pixels. Stimuli
 * may than also be specified in pixels.
 * @PSY_CANVAS_PROJECTION_STYLE_METER:  Create a projection matrix based on the
 * number meters the window is, the sizes of stimuli can be specified in meters.
 * @PSY_CANVAS_PROJECTION_STYLE_MILLIMETER: Create a projection matrix based on
 * the number meters the window is tall. Stimuli may also be presented in
 * millimeters.
 * @PSY_CANVAS_PROJECTION_STYLE_VISUAL_DEGREES: Create a projection matrix based
 * on the number of visual degrees the window is tall. Stimuli should be
 * specified in visual degrees. NOT IMPLEMENTED YET.
 *
 * Instances of `PsyWindow` use an orthographic projection by default. We
 * focus on 2 Dimensional stimuli. This enum can be used to set the
 * projection-style property of a PsyWindow. By default the projection is
 * set up with the point[0.0, 0.0] in the center of the screen, regardless
 * of the units used.
 *
 * The PsyWindow is able to tell the size in meters of the window when it's
 * fullscreen and also the "size" in pixels. It doesn't know how far the
 * subject is sitting from the screen, so this need to be specified when using
 * #PsyProjectionStyleVisualDegrees.
 * When using these flags one should always specify exactly one of:
 *
 *  - PSY_CANVAS_PROJECTION_STYLE_C
 *  - PSY_CANVAS_PROJECTION_STYLE_CENTER
 *
 * and exactly one of
 *
 *  - PSY_CANVAS_PROJECTION_STYLE_PIXELS
 *  - PSY_CANVAS_PROJECTION_STYLE_METER
 *  - PSY_CANVAS_PROJECTION_STYLE_MILLIMETER
 *  - PSY_CANVAS_PROJECTION_STYLE_VISUAL_DEGREES
 */
typedef enum {
    PSY_CANVAS_PROJECTION_STYLE_C              = 1 << 0,
    PSY_CANVAS_PROJECTION_STYLE_CENTER         = 1 << 1,
    PSY_CANVAS_PROJECTION_STYLE_PIXELS         = 1 << 2,
    PSY_CANVAS_PROJECTION_STYLE_METER          = 1 << 3,
    PSY_CANVAS_PROJECTION_STYLE_MILLIMETER     = 1 << 4,
    PSY_CANVAS_PROJECTION_STYLE_VISUAL_DEGREES = 1 << 5
} PsyWindowProjectionStyle;

/**
 * PsyPictureSizeStrategy:
 * @PSY_PICTURE_STRATEGY_AUTOMATIC: The size of the stimulus will be set
 *                                  automatically.
 * @PSY_PICTURE_STRATEGY_MANUAL: The size of the stimulus is set manually.
 *
 * This enum allows specify the desired sizing strategy of a stimulus. By
 * default, one doesn't have to specify the size of the as the size can be
 * determined by the size of the image. This may not always be desirable as
 * the window-style is chosen for metric units instead of pixels.
 * Once the stimulus is resized, it's strategy will be set to manual, so
 * this resize is only requested once.
 */
typedef enum PsyPictureSizeStrategy {
    PSY_PICTURE_STRATEGY_AUTOMATIC,
    PSY_PICTURE_STRATEGY_MANUAL
} PsyPictureSizeStrategy;

/**
 * PsyWaveForm:
 * @PSY_WAVE_FORM_SINE: This is the default shape a pure tone/sine wave
 * @PSY_WAVE_FORM_SQUARE: A square wave form is generated
 * @PSY_WAVE_FORM_SAW: A saw tooth wave form is generated
 * @PSY_WAVE_FORM_TRIANGLE: A triangular wave for is generated
 * @PSY_WAVE_FORM_SILENCE: A silent wave form is generated
 * @PSY_WAVE_FORM_WHITE_UNIFORM_NOISE: White noise with a uniform distribution
 * @PSY_WAVE_FORM_PINK_NOISE: A waveform with pink noise is generated
 * @PSY_WAVE_FORM_WHITE_GAUSSIAN_NOISE: white gaussian noise is generated
 * @PSY_WAVE_FORM_RED_NOISE: red noise is generated
 * @PSY_WAVE_FORM_BLUE_NOISE: blue noise is generated
 * @PSY_WAVE_FORM_VIOLET_NOISE: violet noise is generated

 * The different types of wave form that PsyWave should be able to generate.
 */
typedef enum {
    PSY_WAVE_FORM_SINE                 = 0,
    PSY_WAVE_FORM_SQUARE               = 1,
    PSY_WAVE_FORM_SAW                  = 2,
    PSY_WAVE_FORM_TRIANGLE             = 3,
    PSY_WAVE_FORM_SILENCE              = 4,
    PSY_WAVE_FORM_WHITE_UNIFORM_NOISE  = 5,
    PSY_WAVE_FORM_PINK_NOISE           = 6,
    PSY_WAVE_FORM_WHITE_GAUSSIAN_NOISE = 9,
    PSY_WAVE_FORM_RED_NOISE            = 10,
    PSY_WAVE_FORM_BLUE_NOISE           = 11,
    PSY_WAVE_FORM_VIOLET_NOISE         = 12
} PsyWaveForm;
