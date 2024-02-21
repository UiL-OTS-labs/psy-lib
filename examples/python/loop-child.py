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


def on_leave_loop(loop, tp, mainloop):
    print("closing mainloop")
    mainloop.quit()


class Trial(Psy.Trial):
    def do_enter(self, tp):
        print(Trial.do_enter)

    def do_activate(self, tp):
        loop = self.props.parent
        print(loop.props.index)
        self.leave(tp)


def main():
    main_loop = GLib.MainLoop()
    clk = Psy.Clock()
    starttime = clk.now()

    loop = Psy.Loop(index=0, stop=10)
    trial = Trial()
    loop.set_child(trial)
    loop.connect("leave", on_leave_loop, main_loop)

    # loop from 1 to 10
    loop.enter(starttime)

    main_loop.run()
    print("mainloop stopped")


if __name__ == "__main__":
    main()
