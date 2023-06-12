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
import argparse as ap

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


class MyRect(Psy.Rectangle):
    def __init__(self, nth_frame, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.is_white = True
        self.white = Psy.Color.new_rgb(1, 1, 1)
        self.black = Psy.Color()
        self.nth_frame = nth_frame
        self.counter = 0

    def do_update(self, timepoint, frame_num):
        if self.counter % self.nth_frame == 0:
            if self.is_white:
                self.set_color(self.white)
            else:
                self.set_color(self.black)
            self.is_white = not self.is_white
        self.counter += 1


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


def main(args: ap.Namespace):
    """run the program using the parameters in args"""
    loop = GLib.MainLoop()
    clock = Psy.Clock()
    start = clock.now()
    dur = Psy.Duration.new_ms(args.start_dur)
    stim_dur = Psy.Duration.new_ms(args.duration)
    window = Psy.GtkWindow(n_monitor=args.monitor)
    circle = Psy.Circle.new(window)
    rect = Psy.Rectangle.new(window)
    flikker = MyRect(
        args.swap,
        canvas=window,
        width=100,
        height=100,
        x=-1920 / 2 + 50,
        y=1080 / 2 - 50,
    )
    flikker.set_color(Psy.Color.new_rgb(1, 0, 0))
    rect.props.x, rect.props.y = -200, -200
    rect.props.width, rect.props.height = 100, 100
    rect.set_color(Psy.Color.new_rgb(1.0, 0.0, 0.0))
    cross = MyCross(canvas=window, x=200, y=200, line_length=100, line_width=30)
    circle.play_for(start.add(dur), stim_dur)
    cross.play_for(start.add(dur), stim_dur)
    rect.play_for(start.add(dur), stim_dur)
    flikker.play_for(start.add(dur), stim_dur)

    circle.connect("stopped", stop_loop, (loop, start))
    circle.connect("update", circle_update)

    loop.run()


if __name__ == "__main__":
    cmd_parser = ap.ArgumentParser(
        "python-test.py",
        description="show some test stimuli",
        epilog="Happy Experimenting",
    )

    cmd_parser.add_argument(
        "-m", "--monitor", help="Choose a monitor by number", type=int, default=0
    )
    cmd_parser.add_argument(
        "-s",
        "--start-dur",
        help="The start duration before onset of the stimuli in milliseconds",
        type=int,
        default=500,
    )
    cmd_parser.add_argument(
        "-d",
        "--duration",
        help="Duration of the stimuli in milliseconds",
        type=int,
        default=4000,
    )
    cmd_parser.add_argument(
        "--swap",
        help="num frames between successive swaps of the rectangle",
        type=int,
        default=1,
    )

    args = cmd_parser.parse_args()
    main(args)
