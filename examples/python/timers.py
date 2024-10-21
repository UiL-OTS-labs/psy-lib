import gi
import os
import argparse as ap
import time as t

gi.require_version("Psy", "0.1")
gi.require_version("GLib", "2.0")

# The next two line may generate warnings, ignore them ;-)
from gi.repository import Psy, GLib
import psy_operators

psy_init = Psy.Initializer()

cmd_parser = ap.ArgumentParser(
    "timers", description="An example using Psy.Timers with python"
)
cmd_parser.add_argument("-n", "--num-repetitions", type=int, default=10)
cmd_parser.add_argument("-m", "--ms", type=int, default=100)

args = cmd_parser.parse_args()


class MyTimer(Psy.Timer):
    def __init__(self, loop: GLib.MainLoop, Max: int, **kwargs):
        super().__init__(**kwargs)
        self.connect("fired", self.on_fired)
        self.loop = loop
        self.max = Max
        self.n = 0
        self.old_now = t.time()

    def on_fired(self, _self, tp):
        """Callback registered in init"""
        now = t.time()
        assert self is _self
        print(f"{self.n}: dur = {now - self.old_now} seconds")
        self.old_now = now
        self.props.fire_time = tp + wait_dur
        self.n += 1
        if self.n >= self.max:
            self.loop.quit()


wait_dur = Psy.Duration.new_ms(args.ms)
clock = Psy.Clock()

event_loop = GLib.MainLoop()

now = clock.now()
mt = MyTimer(event_loop, args.num_repetitions)
mt.props.fire_time = now + wait_dur

event_loop.run()
