
#pragma once

/**
 * PsyDrawingContextError:
 * @PSY_DRAWING_CONTEXT_ERROR_NAME_EXISTS: A shader program with that name has already
 *                                 been registered.
 * @PSY_DRAWING_CONTEXT_ERROR_NAME_FAILED: Some less specified error regarding the
 *                                 context occured.
 *
 * These errors may be the result of invalid operations on an instance
 * of `PsyDrawingContext`
 */
typedef enum {
    PSY_DRAWING_CONTEXT_ERROR_NAME_EXISTS,
    PSY_DRAWING_CONTEXT_ERROR_FAILED
} PsyDrawingContextError;

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
typedef enum  {
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
 * @PSY_TEXTURE_ERROR_DECODE:Error occured because we were not able to decode the image.
 * @PSY_TEXTURE_ERROR_FAILED:Another error regarding the texture occurred.
 */
typedef enum  {
    PSY_TEXTURE_ERROR_DECODE, // failed to decode the texture.
    PSY_TEXTURE_ERROR_FAILED
} PsyTextureError;


/**
 * PsyWindowProjectionStyle:
 * @PSY_WINDOW_PROJECTION_STYLE_C: The origin is in the upper left corner of the
 *                                 window. With positive y coordinates going down.
 * @PSY_WINDOW_PROJECTION_STYLE_CENTER: The origin is in the center of the window
 *                            with positive y coordinates is going up.
 * @PSY_WINDOW_PROJECTION_STYLE_PIXELS: Create a projection matrix that makes the
 *                            screen as wide,tall as the dimensions of the
 *                            number of pixels. Stimuli may than also be
 *                            specified in pixels.
 * @PSY_WINDOW_PROJECTION_STYLE_METER:  Create a projection matrix based on the number
 *                            meters the window is, the sizes of stimuli can be
 *                            specified in meters.
 * @PSY_WINDOW_PROJECTION_STYLE_MILLIMETER: Create a projection matrix based on the
 *                            number meters the window is tall. Stimuli
 *                            may also be presented in millimeters.
 * @PSY_WINDOW_PROJECTION_STYLE_VISUAL_DEGREES: Create a projection matrix based on 
 *                            the number of visual degrees the window is tall.
 *                            Stimuli should be specified in visual degrees.
 *                            NOT IMPLEMENTED YET.
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
 *  - PSY_WINDOW_PROJECTION_STYLE_C
 *  - PSY_WINDOW_PROJECTION_STYLE_CENTER
 *
 * and exactly one of 
 *
 *  - PSY_WINDOW_PROJECTION_STYLE_PIXELS
 *  - PSY_WINDOW_PROJECTION_STYLE_METER
 *  - PSY_WINDOW_PROJECTION_STYLE_MILLIMETER
 *  - PSY_WINDOW_PROJECTION_STYLE_VISUAL_DEGREES
 */
typedef enum {
    PSY_WINDOW_PROJECTION_STYLE_C               = 1 << 0,
    PSY_WINDOW_PROJECTION_STYLE_CENTER          = 1 << 1,
    PSY_WINDOW_PROJECTION_STYLE_PIXELS          = 1 << 2,
    PSY_WINDOW_PROJECTION_STYLE_METER           = 1 << 3,
    PSY_WINDOW_PROJECTION_STYLE_MILLIMETER      = 1 << 4,
    PSY_WINDOW_PROJECTION_STYLE_VISUAL_DEGREES  = 1 << 5
} PsyWindowProjectionStyle;

