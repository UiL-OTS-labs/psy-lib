#!/usr/bin/env python3

"""
Run with Psy.0.1.typelib on a default path or specify the following
environmental variables before starting this script:
   GI_TYPELIB_PATH = "path/to/Psy-0.1.typelib"
When the suitable libpsy-1.0.so .dll is available on the path you
should be able to run it otherwise use:
    LD_LIBRARY_PATH = "path/to/libpsy.so/folder"
"""

import gi
import typing
import math as m
import time as t

gi.require_versions({"Psy": "0.1", "GLib": "2.0"})

from gi.repository import Psy
from gi.repository import GLib


class MyCross(Psy.Cross):
    """
    In order to override a virtual method you have to prepend your
    method with do_, so PsyCrossClass->update is called in python
    by MyCross.do_update
    """

    def do_update(self, timepoint, frame_num):
        self.props.x += 1
        self.props.y += 1
        self.set_color(
            Psy.Color(r=(m.sin(t.time()) / 2 + 0.5), g=m.cos(t.time()) / 2 + 0.5, b=0.5)
        )


def stop_loop(
    circle: Psy.Circle,
    time_point: Psy.TimePoint,
    tup: typing.Tuple[GLib.MainLoop, Psy.TimePoint],
):
    """
    Exit from the mainloop and exit the program
    """
    loop = tup[0]
    time_start = tup[1]

    try:
        print(
            "Circle.start = {}".format(
                circle.props.start_time.subtract(time_start).props.seconds
            )
        )
        print(
            "Circle.stop = {}".format(
                circle.props.stop_time.subtract(time_start).props.seconds
            )
        )
        print(
            "The circle is presented for roughly = {} seconds".format(
                circle.props.stop_time.subtract(circle.props.start_time).props.seconds
            )
        )
    except Exception as e:
        print("We shouldn't get here", e)
        pass

    loop.quit()


def circle_update(circle: Psy.Circle, nth_frame, data):
    """
    Do something nice with the circle
    """
    circle.props.radius = circle.props.radius + 1
    circle.props.x = circle.props.x - 1
    circle.props.num_vertices = circle.props.num_vertices + 1


loop = GLib.MainLoop()
clock = Psy.Clock()
start = clock.now()
dur = Psy.Duration.new(0.5)
window = Psy.GtkWindow()
circle = Psy.Circle.new(window)
rect = Psy.Rectangle.new(window)
rect.props.x, rect.props.y = -200, -200
rect.props.width, rect.props.height = 100, 100
rect.set_color(Psy.Color.new_rgb(1.0, 0.0, 0.0))
cross = MyCross(canvas=window, x=200, y=200, line_length=100, line_width=30)
circle.play_for(start.add(dur), dur.multiply_scalar(5))
cross.play_for(start.add(dur), dur.multiply_scalar(4))
rect.play_for(start.add(dur), dur.multiply_scalar(4))
circle.connect("stopped", stop_loop, (loop, start))
circle.connect("update", circle_update)

loop.run()
