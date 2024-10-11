import gi
import os
import time as t

gi.require_version("Psy", "0.1")
gi.require_version("GLib", "2.0")

# The next two line may generate warnings, ignore them ;-)
from gi.repository import Psy, GLib
import psy_operators

wait_dur = Psy.Duration.new_ms(100)
clock = Psy.Clock()


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
        assert self is _self
        now = t.time()
        print(f"{self.n}: dur = {now - self.old_now} seconds")
        self.old_now = now
        self.props.fire_time = tp + wait_dur
        self.n += 1
        if self.n >= self.max:
            self.loop.quit()


event_loop = GLib.MainLoop()

now = clock.now()
mt = MyTimer(event_loop, 10)
mt.props.fire_time = now + wait_dur

event_loop.run()
