#!/usr/bin/env python3
import gi

gi.require_version("Psy", "0.1")
gi.require_version("GLib", "2.0")

from gi.repository import Psy
from gi.repository import GLib

WHITE = Psy.Color(r=1, g=1, b=1)
BLACK = Psy.Color(r=0, g=0, b=0)

canvas = Psy.GtkWindow.new()
canvas.props.background_color = BLACK

loop = GLib.MainLoop()

loop.run()
