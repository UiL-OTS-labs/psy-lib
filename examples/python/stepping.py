#!/usr/bin/env python3

import argparse as ap
import gi
from math import sin, cos, pi

gi.require_version("Psy", "0.1")
gi.require_version("GLib", "2.0")

from gi.repository import Psy, GLib

window: Psy.Canvas
NUM_CIRCLES = 12
DUR = Psy.Duration.new(0.200)
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
    for i in range(NUM_CIRCLES):
        angle = 2 * pi / NUM_CIRCLES * i
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


def on_red_loop_iter(loop: Psy.Loop, index: int, tp: Psy.TimePoint):
    """Run an iteration of the red loop"""
    draw(index % NUM_CIRCLES, tp, RED, loop)


def on_blue_loop_iter(loop: Psy.Loop, index: int, tp: Psy.TimePoint):
    """Run an iteration of the blue loop"""
    draw(index % NUM_CIRCLES, tp, BLUE, loop)


def on_blue_loop_leave(loop: Psy.Loop, tp: Psy.TimePoint, blue_first):
    """Activate the red loop when presenting blue first, or simply continue to
    the exit_trial."""
    if blue_first:
        loop.props.parent.activate_next_by_name("red_loop")


def on_red_loop_leave(loop: Psy.Loop, tp: Psy.TimePoint, blue_first):
    """When blue first is true, the blue loop should already have played,
    hence we skip to the exit"""
    if blue_first:
        loop.props.parent.activate_next_by_name("exit")


def on_main_step_finish(step: Psy.Step, tp: Psy.TimePoint, loop: GLib.MainLoop) -> None:
    """Stop the mainloop"""
    loop.quit()


def on_steps_enter(
    step: Psy.SteppingStones, tp: Psy.TimePoint, blue_first: bool
) -> None:
    """Choose whether to enter the red or the blue loop"""
    if blue_first:
        step.activate_next_by_name("blue_loop")


def on_exit_trial_enter(trial: Psy.Trial, tp: Psy.TimePoint) -> None:
    """Just leave this trial without doing anything"""
    trial.leave(tp)  # Just hand over the timepoint to the next step


def main():
    global window
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

    main_loop = GLib.MainLoop()

    args = cmd_parser.parse_args()
    if args.windowed:
        window = Psy.GtkWindow.new()
    else:
        window = Psy.GlCanvas.new(640, 480)
        window.props.frame_dur = Psy.Duration.new(1.0 / 60)
        # The gl canvas doesn't iterate out of it's own, so we do it manually
        GLib.idle_add(iterate_canvas, window)

    steps = Psy.SteppingStones.new()

    # loop starting from 0 to 10 with steps of one, while the loops index is
    # smaller than 10
    red_loop = Psy.Loop.new_full(0, 5, 1, Psy.LoopCondition.LESS)
    blue_loop = Psy.Loop.new_full(0, 5, 1, Psy.LoopCondition.LESS)
    exit_trial = Psy.Trial()  # We can jump to this step to exit the steps

    red_loop.connect("iteration", on_red_loop_iter)
    blue_loop.connect("iteration", on_blue_loop_iter)
    red_loop.connect("leave", on_red_loop_leave, args.blue_first)
    blue_loop.connect("leave", on_blue_loop_leave, args.blue_first)

    exit_trial.connect("enter", on_exit_trial_enter)

    steps.add_step_by_name("red", red_loop)
    steps.add_step_by_name("blue", blue_loop)
    steps.add_step_by_name("exit", exit_trial)

    steps.connect("enter", on_steps_enter, args.blue_first)
    steps.connect("leave", on_main_step_finish, main_loop)

    steps.enter(clock.now().add(Psy.Duration.new(1.0)))

    exit(main_loop.run())


if __name__ == "__main__":
    main()
