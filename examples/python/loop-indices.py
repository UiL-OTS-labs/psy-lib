#!/usr/bin/env python3


import gi
import os
import sys

gi.require_version("Psy", "0.1")
gi.require_version("GLib", "2.0")

# The next two line may generate warnings, ignore them ;-)
from gi.repository import Psy, GLib
import psy_operators

expected = [(0, 0), (0, 1), (1, 0), (1, 1)]

clk = Psy.Clock()


def on_outer_leave(loop, tp, mainloop: GLib.MainLoop):
    """Make the mainloop quit"""
    mainloop.quit()


def on_inner_enter(loop, tp):
    """Reset the index to 0, because, otherwise there is nothing resetting
    the index property in the second iteration of the loop"""
    loop.props.index = 0


class PrintIndicesTrial(Psy.Trial):
    error_caught = False

    def do_activate(self, tp: Psy.TimePoint):
        Psy.Trial.do_activate(self, tp)  # chain up to parent

        print(self.props.active)

        inner_index = self.get_loop_index(0)
        outer_index = self.get_loop_index(1)
        indices = self.get_loop_indices()

        if not PrintIndicesTrial.error_caught:
            try:
                self.get_loop_index(2)
            except GLib.Error as e:
                if e.matches(Psy.step_error_quark(), Psy.StepError.NO_SUCH_LOOP):
                    PrintIndicesTrial.error_caught = True
                else:
                    raise e

        print(f"inner = {inner_index}\touter = {outer_index}\tindices = {indices}")

        assert indices == [outer_index, inner_index]
        assert inner_index == self.props.parent.props.index
        assert outer_index == self.props.parent.props.parent.props.index

        self.leave(tp)


outer_loop = Psy.Loop(index=0, increment=1, stop=2)
inner_loop = Psy.Loop(index=0, increment=1, stop=2)
trial = PrintIndicesTrial()

outer_loop.set_child(inner_loop)
inner_loop.set_child(trial)
inner_loop.connect("enter", on_inner_enter)

outer_loop.enter(clk.now())

mainloop = GLib.MainLoop()

outer_loop.connect("leave", on_outer_leave, mainloop)
mainloop.run()

assert PrintIndicesTrial.error_caught is True
