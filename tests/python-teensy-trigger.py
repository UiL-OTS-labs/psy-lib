#!/usr/bin/env/python

import argparse as ap
import gi
import os
import sys

gi.require_versions({"Psy": "0.1", "GLib": "2.0"})
from gi.repository import Psy, GLib  # noqa: E402

DEV = "/dev/ttyACM0" if os.name == "posix" else "COM1"


def quit_loop(loop: GLib.MainLoop, t1: Psy.TimePoint):
    t2 = Psy.Clock().now()
    print("Quitting loop after: {}".format((t2.subtract(t1)).get_seconds()))
    loop.quit()


def main():
    parser = ap.ArgumentParser(sys.argv[0], description="trigger a teensy device")
    parser.add_argument("-s", "--serial_name", type=str, default=DEV)
    args = parser.parse_args()

    trigger = Psy.TeensyTrigger(name=args.serial_name, trig_dur=Psy.Duration.new_ms(1))

    trigger.open()

    loop: GLib.MainLoop = GLib.MainLoop()
    t1 = Psy.Clock().now()
    GLib.timeout_add(1000, quit_loop, loop, t1)

    trigger.write(0xFF, t1.add(Psy.Duration.new_ms(500)))

    loop.run()


if __name__ == "__main__":
    init = Psy.Initializer(portaudio=False, gstreamer=False)
    main()
