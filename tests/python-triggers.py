#!/usr/bin/env/python

import argparse as ap
import os
import gi
import sys

gi.require_versions({"Psy": "0.1", "GLib": "2.0"})

from gi.repository import Psy, GLib

SERIAL_DEV = "/dev/ttyACM0" if os.name == "posix" else "COM1"

clk = Psy.Clock()
timer_dur = Psy.Duration.new_ms(100)
isi = Psy.Duration.new_ms(50)  # onset of trigger after timer has fired
trigger_dur = Psy.Duration.new_ms(1)


class RangedInt:
    def __init__(self, min, max):
        if min >= max:
            raise ValueError("min should be smaller than max")
        self.min = min
        self.max = max

    def __call__(self, value):
        ival = int(value)

        if self.min <= ival < self.max:
            return ival
        else:
            raise ap.ArgumentTypeError(
                f"{value} is not an int in the range [{self.min}, {self.max})"
            )

    def __repr__(self):
        return f"RangedInt({self.min}, {self.max})"


def stop(loop: GLib.MainLoop):
    loop.quit()


def on_timer_fire(
    timer: Psy.Timer,
    tp: Psy.TimePoint,
    teensy: Psy.TeensyTrigger,
    parallel: Psy.ParallelTrigger,
):
    teensy.write(0xFF, tp.add(isi))
    if parallel.props.is_open:
        parallel.write(0xFF, tp.add(isi), trigger_dur)

    timer.set_fire_time(tp.add(timer_dur))


def main() -> None:
    global trigger_dur

    parser = ap.ArgumentParser(sys.argv[0], description="trigger a teensy device")
    parser.add_argument("-s", "--serial_name", type=str, default=SERIAL_DEV)
    parser.add_argument("-t", "--trigdur", type=RangedInt(1, 50), default=1)
    parser.add_argument("--no-parallel", action="store_true")

    args = parser.parse_args()
    if args.trigdur:
        trigger_dur = Psy.Duration.new_ms(args.trigdur)

    loop = GLib.MainLoop()
    teensy = Psy.TeensyTrigger.new(args.serial_name)
    parallel = Psy.ParallelTrigger()

    now = clk.now()

    teensy.open()
    teensy.set_trig_dur(trigger_dur)

    if not args.no_parallel:
        parallel.open(0)

    timer = Psy.Timer()
    print(timer)
    timer.set_fire_time(now.add(timer_dur))
    timer.connect("fired", on_timer_fire, teensy, parallel)

    GLib.timeout_add(10000, stop, loop)

    loop.run()


if __name__ == "__main__":
    init = Psy.Initializer(portaudio=False, gstreamer=False)
    main()
