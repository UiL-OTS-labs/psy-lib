#!/usr/bin/env python3
"""Short demonstration of looping inside an event loop, The mainloop can
handle a lot of events, whereas a Psy.Loop can structure you experiment
in time. All Psy.TimePoints in this example calculated based on input. They
do not directly have a reference to a clock, stimuli in psylib, in contrast do
here you can see from the output that the time passes very quickly, whereas
in the count down example, on each iteration the calculated time is one second.
So in essence the tp arguments of the iteration are a reference to some event
that has passed.
"""

import gi
import os

gi.require_version("Psy", "0.1")
gi.require_version("GLib", "2.0")

# The next two line may generate warnings, ignore them ;-)
from gi.repository import Psy, GLib
import psy_operators


def on_iteration(loop: Psy.Loop, index: int, tp: Psy.TimePoint):
    """Something to run on each iteration, start your trial here"""
    print(f"index = {index}")
    # queue next iteration typically you would do this from a response,
    # stimulus, timeout etc. stopping.
    loop.activate(tp)


def on_iteration2(loop: Psy.Loop, index: int, tp: Psy.TimePoint):
    """Do something with time"""
    one_second = Psy.Duration.new(1.0)
    new_tp = tp + one_second
    if index == 0:
        print("Liftoff")
    else:
        print(f"index = {index}")
    loop.activate(new_tp)


def leave_loop(loop: Psy.Loop, tp: Psy.TimePoint, mainloop: GLib.MainLoop, starttime):
    """Quits the mainloop"""
    if tp.equal(starttime):
        print("tp == starttime")
    else:
        dur = tp.subtract(starttime)
        print("virtual time passed = {}".format(dur.get_seconds()))
    mainloop.quit()


def main():
    main_loop = GLib.MainLoop()
    clock = Psy.Clock()
    starttime = clock.now()

    print("Counting up")

    # ```python
    # for i in range(10):
    #    pass
    # ```
    loop = Psy.Loop(index=0, stop=10, increment=1, condition=Psy.LoopCondition.LESS)

    loop.connect("iteration", on_iteration)
    loop.connect("leave", leave_loop, main_loop, starttime)  # quit event loop

    loop.enter(starttime)

    main_loop.run()

    print(os.linesep, "Counting down")

    main_loop = GLib.MainLoop()

    # loop from 10 to 0
    loop = Psy.Loop(
        index=10, stop=0, increment=-1, condition=Psy.LoopCondition.GREATER_EQUAL
    )
    loop.connect("iteration", on_iteration2)
    loop.connect("leave", leave_loop, main_loop, starttime)
    loop.enter(starttime)

    main_loop.run()

    endtime = clock.now()
    print(
        os.linesep,
        "The two loops took {} seconds".format(
            endtime.subtract(starttime).get_seconds()
        ),
    )


if __name__ == "__main__":
    main()
