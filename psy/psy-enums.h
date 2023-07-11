
#pragma once

/**
 * PsyAudioSampleRate:
 * @PSY_AUDIO_SAMPLE_RATE_22050: A low quality sample rate (old MP3's)
 * @PSY_AUDIO_SAMPLE_RATE_32000: A quality sufficient for audio tapes FM radio
 * @PSY_AUDIO_SAMPLE_RATE_44100: The quality of CD audio
 * @PSY_AUDIO_SAMPLE_RATE_48000: The quality of DVD audio the default for psylib
 * @PSY_AUDIO_SAMPLE_RATE_96000: A high quality likely better than you need
 * @PSY_AUDIO_SAMPLE_RATE_192000: A very high quality likely much higher than
 * you need.
 *
 * The sample rates higher then 32000 can represent all audio waves that most
 * humans can hear, except for the young. The most used sample rates will
 * probably be PSY_AUDIO_SAMPLE_RATE_44100 and _48000, as these can represent
 * everything we can hear. You can go higher, but is it necessary???
 *
 * Info is taken at july 11th 2023 from the [audacity-wiki]
 * (https://manual.audacityteam.org/man/sample_rates.html)
 */
typedef enum {
    PSY_AUDIO_SAMPLE_RATE_22050  = 22050,
    PSY_AUDIO_SAMPLE_RATE_32000  = 32000,
    PSY_AUDIO_SAMPLE_RATE_44100  = 44100,
    PSY_AUDIO_SAMPLE_RATE_48000  = 48000,
    PSY_AUDIO_SAMPLE_RATE_96000  = 96000,
    PSY_AUDIO_SAMPLE_RATE_192000 = 192000
} PsyAudioSampleRate;

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
