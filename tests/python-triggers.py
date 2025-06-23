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


def stop(loop: GLib.MainLoop):
    loop.quit()


def on_timer_fire(
    timer: Psy.Timer,
    tp: Psy.TimePoint,
    teensy: Psy.TeensyTrigger,
    parallel: Psy.ParallelTrigger,
):
    teensy.write(0xFF, tp.add(isi))
    parallel.write(0xFF, tp.add(isi), trigger_dur)

    timer.set_fire_time(tp.add(timer_dur))


def main() -> None:
    
    parser = ap.ArgumentParser(sys.argv[0], description="trigger a teensy device")
    parser.add_argument("-s", "--serial_name", type=str, default=SERIAL_DEV)
    args = parser.parse_args()

    loop = GLib.MainLoop()
    teensy = Psy.TeensyTrigger.new(args.serial_name)
    parallel = Psy.ParallelTrigger()

    now = clk.now()

    timer = Psy.Timer()
    timer.set_fire_time(now.add(timer_dur))
    timer.connect("fired", on_timer_fire, teensy, parallel)

    teensy.open()
    teensy.set_trig_dur(trigger_dur)
    parallel.open(0)

    GLib.timeout_add(10000, stop, loop)

    loop.run()


if __name__ == "__main__":
    init = Psy.Initializer(portaudio=False, gstreamer=False)
    main()
