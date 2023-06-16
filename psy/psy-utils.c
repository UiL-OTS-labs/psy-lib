
#include "psy-utils.h"

/**
 * psy_coordinate_center_to_c:
 * @width: the width of the surface
 * @height: the height of the surface
 * @x_in: The x coordinate that has is origin in the left upper corner in the
 *        surface
 * @y_in: The y coordinate that has its origin in the left upper corner in the
 *        surface
 * @x_out:(out): The resulting x-coordinate that that its origin in the center
 *               of the image and the top at above
 * @y_out:(out): The resulting y-coordinate that that its origin in the center
 *               of the image and the top at above
 *
 * translates coordinates on a surface with a given width and height to from
 * c-style coordinates (the typical the origin of the image is in the left upper
 * corner) and positive y-coordinates move down ward:
 * ```
 *  0,0 ************* w,0
 *   *                 *
 *   *                 *
 *   *     w/2,h/2     *
 *   *                 *
 *   *                 *
 *  0,h ************* w,h
 * ```
 *
 * to a space where the center is in the origin.
 *
 * ```
 *  -w/2,h/2 ************* w/2,h/2
 *      *                     *
 *      *                     *
 *      *        0,0          *
 *      *                     *
 *      *                     *
 *  -w/2,-h/2 ************ w/2,-h/2
 * ```
 *
 * So the input coordinates should belong to the upper ascii art
 * and the output coordinates should belong to the lower ascii art
 */
void
psy_coordinate_center_to_c(gdouble  width,
                           gdouble  height,
                           gdouble  x_in,
                           gdouble  y_in,
                           gdouble *x_out,
                           gdouble *y_out)
{
    *x_out = x_in + width / 2;
    *y_out = -y_in + height / 2;
}

/**
 * psy_coordinate_c_to_center:
 * @width: the width of the surface
 * @height: the height of the surface
 * @x_in: The x coordinate that has is origin in the left upper corner in the
 *        surface
 * @y_in: The y coordinate that has its origin in the left upper corner in the
 *        surface
 * @x_out:(out): The resulting x-coordinate that that its origin in the center
 *               of the image and the top at above
 * @y_out:(out): The resulting y-coordinate that that its origin in the center
 *               of the image and the top at above
 *
 * translates coordinates on a surface with a given width and height to from
 * center-style coordinates (the typical the origin of the image is in the
 * center of the image to a c-style coordinate, where the center is at the top:
 *
 *
 * ```
 *  0,0 ************* w,0
 *   *                 *
 *   *                 *
 *   *     w/2,h/2     *
 *   *                 *
 *   *                 *
 *  0,h ************* w,h
 * ```
 *
 * to a space where the center is in the origin.
 *
 * ```
 *  -w/2,h/2 ************** w/2,h/2
 *      *                      *
 *      *                      *
 *      *         0,0          *
 *      *                      *
 *      *                      *
 *  -w/2,-h/2 ************* w/2,-h/2
 * ```
 *
 * So the input coordinates should belong to the upper ascii art
 * and the output coordinates should belong to the lower ascii art
 */
void
psy_coordinate_c_to_center(gdouble  width,
                           gdouble  height,
                           gdouble  x_in,
                           gdouble  y_in,
                           gdouble *x_out,
                           gdouble *y_out)
{
    *x_out = x_in - width / 2;
    *y_out = -y_in + height / 2;
}

/**
 * psy_coordinate_center_to_c_i:
 * @width: the width of the surface
 * @height: the height of the surface
 * @x_in: The x coordinate that has is origin in the left upper corner in the
 *        surface
 * @y_in: The y coordinate that has its origin in the left upper corner in the
 *        surface
 * @x_out:(out): The resulting x-coordinate that that its origin in the center
 *               of the image and the top at above
 * @y_out:(out): The resulting y-coordinate that that its origin in the center
 *               of the image and the top at above
 *
 * translates coordinates on a surface with a given width and height to from
 * c-style coordinates (the typical the origin of the image is in the left upper
 * corner) and positive y-coordinates move down ward:
 * ```
 *  0,0 ************* w,0
 *   *                 *
 *   *                 *
 *   *     w/2,h/2     *
 *   *                 *
 *   *                 *
 *  0,h ************* w,h
 * ```
 *
 * to a space where the center is in the origin.
 *
 * ```
 *  -w/2,h/2 ************* w/2,h/2
 *      *                     *
 *      *                     *
 *      *        0,0          *
 *      *                     *
 *      *                     *
 *  -w/2,-h/2 ************ w/2,-h/2
 * ```
 *
 * So the input coordinates should belong to the upper ascii art
 * and the output coordinates should belong to the lower ascii art
 */

void
psy_coordinate_center_to_c_i(
    gint width, gint height, gint x_in, gint y_in, gint *x_out, gint *y_out)
{
    *x_out = x_in + width / 2;
    *y_out = -y_in + height / 2;
}

/**
 * psy_coordinate_c_to_center_i:
 * @width: the width of the surface
 * @height: the height of the surface
 * @x_in: The x coordinate that has is origin in the left upper corner in the
 *        surface
 * @y_in: The y coordinate that has its origin in the left upper corner in the
 *        surface
 * @x_out:(out): The resulting x-coordinate that that its origin in the center
 *               of the image and the top at above
 * @y_out:(out): The resulting y-coordinate that that its origin in the center
 *               of the image and the top at above
 *
 * translates coordinates on a surface with a given width and height to from
 * center-style coordinates (the typical the origin of the image is in the
 * center of the image to a c-style coordinate, where the center is at the top:
 *
 *
 *
 * ```
 *  0,0 ************* w,0
 *   *                 *
 *   *                 *
 *   *     w/2,h/2     *
 *   *                 *
 *   *                 *
 *  0,h ************* w,h
 * ```
 *
 * to a space where the center is in the origin.
 *
 * ```
 *  -w/2,h/2 ************** w/2,h/2
 *      *                      *
 *      *                      *
 *      *         0,0          *
 *      *                      *
 *      *                      *
 *  -w/2,-h/2 ************* w/2,-h/2
 * ```
 *
 * So the input coordinates should belong to the upper ascii art
 * and the output coordinates should belong to the lower ascii art
 */
void
psy_coordinate_c_to_center_i(
    gint width, gint height, gint x_in, gint y_in, gint *x_out, gint *y_out)
{
    *x_out = x_in - width / 2;
    *y_out = -y_in + height / 2;
}
