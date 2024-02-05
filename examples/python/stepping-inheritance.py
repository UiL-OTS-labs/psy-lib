#!/usr/bin/env python3

import argparse as ap
import gi
from math import sin, cos, pi

gi.require_version("Psy", "0.1")
gi.require_version("GLib", "2.0")

from gi.repository import Psy, GLib

window: Psy.Canvas
_NUM_CIRCLES = 12
_NUM_ITERATIONS = 50
DUR = Psy.Duration.new(0.100)
BLACK = Psy.Color.new()
RED = Psy.Color.new_rgb(1, 0, 0)
BLUE = Psy.Color.new_rgb(0, 0, 1)
WHITE = Psy.Color.new_rgb(1, 1, 1)


def iterate_canvas(window: Psy.GlCanvas):
    """Iterates the window, as a image canvas doesn't iterate it self"""
    window.iterate()
    # ToDo, iterating the canvas Doesn't seem to adjust time, hence the stimuli
    # don't seem to be started/finished
    return GLib.SOURCE_CONTINUE


def on_cross_started(cross: Psy.Cross, tp: Psy.TimePoint, step: Psy.Loop):
    """marks the current iteration of the loop as done and schedules the next"""
    step.activate(tp)


def draw(n, tp: Psy.TimePoint, color: Psy.Color, step: Psy.Loop):
    radius = 400
    cradius = 0.1 * radius
    for i in range(_NUM_CIRCLES):
        angle = 2 * pi / _NUM_CIRCLES * i
        circle = Psy.Circle.new_full(
            window, sin(angle) * radius, cos(angle) * radius, cradius, 60
        )
        if i != n:
            circle.props.color = color
        else:
            circle.props.color = window.props.background_color
        circle.props.duration = DUR
        circle.play(tp.add(DUR))
    cross = Psy.Cross.new(window)
    cross.props.color = BLACK
    cross.props.line_length = cradius
    cross.props.line_width = 10
    cross.props.duration = DUR
    cross.play(tp.add(DUR))
    cross.connect("started", on_cross_started, step)


class RedLoop(Psy.Loop):
    """Is responsible for drawing the red dot's"""

    def __init__(self, num_circles, steps: Psy.SteppingStones, blue_first):
        super().__init__(
            index=0, stop=num_circles, increment=1, condition=Psy.LoopCondition.LESS
        )
        self.blue_first = blue_first
        self.stepping_stones = steps

    def do_iteration(self, index: int, tp: Psy.TimePoint) -> None:
        """Overwrite parents iteration, is called on each iteration of the loop"""
        draw(index % _NUM_CIRCLES, tp, RED, self)
        Psy.Loop.do_iteration(self, index, tp)  # chainup to parent

    def do_on_leave(self, tp: Psy.TimePoint) -> None:
        """Do something when leaving the loop"""
        if self.blue_first:
            self.stepping_stones.activate_next_by_name("exit")
        Psy.Loop.do_on_leave(self, tp)  # chainup to parent


class BlueLoop(Psy.Loop):
    """Is responsible for drawing the blue dot's"""

    def __init__(self, num_circles, steps: Psy.SteppingStones, blue_first):
        super().__init__(
            index=0, stop=num_circles, increment=1, condition=Psy.LoopCondition.LESS
        )
        self.blue_first = blue_first
        self.stepping_stones = steps

    def do_iteration(self, index: int, tp: Psy.TimePoint) -> None:
        """Overwrite parents iteration, is called on each iteration of the loop"""
        draw(index % _NUM_CIRCLES, tp, BLUE, self)
        Psy.Loop.do_iteration(self, index, tp)  # chainup to parent

    def do_on_leave(self, tp: Psy.TimePoint) -> None:
        """Do something when leaving the loop"""
        if self.blue_first:
            self.stepping_stones.activate_next_by_name("red")
        Psy.Loop.do_on_leave(self, tp)  # chainup to parent


class ExperimentSteps(Psy.SteppingStones):
    """The main steps of the experiment, controls the order of the blocks of
    the experiment.
    """

    def __init__(self, blue_first, main_loop):
        super().__init__()
        self.blue_first = blue_first
        self.main_loop = main_loop

    def do_on_enter(self, tp: Psy.TimePoint) -> None:
        """Choose whether to enter the red or the blue loop"""
        if self.blue_first:
            self.activate_next_by_name("blue")
        Psy.SteppingStones.do_on_enter(self, tp)  # chain up

    def do_on_leave(self, tp: Psy.TimePoint) -> None:
        """Stop the mainloop"""
        self.main_loop.quit()
        Psy.SteppingStones.do_on_leave(self, tp)


def on_exit_trial_enter(trial: Psy.Trial, tp: Psy.TimePoint) -> None:
    """Just leave this trial without doing anything"""
    trial.leave(tp)  # Just hand over the timepoint to the next step


def main():
    global window
    global _NUM_CIRCLES
    clock = Psy.Clock()
    cmd_parser = ap.ArgumentParser(
        "stepping.py", description="Run in CI or in windowed mode"
    )
    cmd_parser.add_argument(
        "-w",
        "--windowed",
        default=False,
        action="store_true",
        help="View the steps in windowed mode",
    )
    cmd_parser.add_argument(
        "-b", "--blue-first", help="Present blue first, than red", action="store_true"
    )
    cmd_parser.add_argument(
        "-n",
        "--num-iterations",
        type=int,
        default=_NUM_ITERATIONS,
        help="Specify the number of iterations of the loop",
    )
    cmd_parser.add_argument(
        "-c",
        "--num-circles",
        type=int,
        default=_NUM_CIRCLES,
        help="Specify the number to present",
    )

    main_loop = GLib.MainLoop()

    args = cmd_parser.parse_args()
    _NUM_CIRCLES = args.num_circles

    if args.windowed:
        window = Psy.GtkWindow.new()
    else:
        window = Psy.GlCanvas.new(640, 480)
        window.props.frame_dur = Psy.Duration.new(1.0 / 60)
        # The gl canvas doesn't iterate out of it's own, so we do it manually
        GLib.idle_add(iterate_canvas, window)

    # Contains the main parts of this "experiment"
    steps = ExperimentSteps(args.blue_first, main_loop)

    # loop starting from 0 to 10 with steps of one, while the loops index is
    # smaller than 10
    red_loop = RedLoop(args.num_iterations, steps, args.blue_first)
    blue_loop = BlueLoop(args.num_iterations, steps, args.blue_first)
    exit_trial = Psy.Trial()  # We can jump to this step to exit the steps

    exit_trial.connect("enter", on_exit_trial_enter)

    steps.add_step_by_name("red", red_loop)
    steps.add_step_by_name("blue", blue_loop)
    steps.add_step_by_name("exit", exit_trial)

    # enter the step with a time in the future
    steps.enter(clock.now().add(Psy.Duration.new(1.0)))

    exit(main_loop.run())


if __name__ == "__main__":
    main()
