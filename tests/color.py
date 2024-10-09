#!/usr/bin/env python

import gi
import tempfile

# All object in Psy.* derive from GObject and not GObject.InitiallyUnowned.
# Hence, they cannot have a floating reference. All objects/functions (should) use
# transfer annotations, to determine object lifetime, as is currently advised.

gi.require_version("Psy", "0.1")
from gi.repository import Psy

bg_color = Psy.Color(r=0.5, g=0.5, b=0.5)
fg_color = Psy.Color(r=1.0, g=0.0, b=0.0)

x, y = 0.0, 0.0
radius = 150.0
num_vertices = 100

canvas = Psy.GlCanvas.new(640, 480)
circle = Psy.Circle.new_full(canvas, x, y, num_vertices, radius)
canvas.set_background_color(bg_color)  # this seems fine
circle.props.color = fg_color  # this is fine as we have an ref in python
# The next line returns an error.
circle.props.color = Psy.Color(r=0.5, g=0.5, b=0.5)
circle.props.color = fg_color
circle.play_for(
    canvas.get_time().add(Psy.Duration.new_ms(16)), Psy.Duration.new_ms(50)
)  # Draw circle for 50 ms.
canvas.iterate()
image = canvas.get_image()
image.save_path(tempfile.gettempdir() + "/red-circle.png", "png")
