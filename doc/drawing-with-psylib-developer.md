Title:Drawing with psylib for developers

# Drawing with psylib for developers

## Introduction

This document describes how drawing works for developers. It describes how
Different classes work together in order to draw on canvasses like windows
(or pictures to be developed). In short there is a unison between a canvas
such as [class@Window], artist's whose base class is [class@Artist] and
visual stimuli whose base class is [class@VisualStimulus].

## Canvasses such as windows

Drawing needs some surface to apply the paint to. The actors that apply the paint
to the canvas are called [class@Artist]. Typically, `Window`'s will have a
callback installed. The callback is fired just in the frame before it is
actually put on the monitor. The window will make sure that every
[class@VisualStimulus] registered with the window will receive an update so they
can adapt themselves to the coming frame. Additionally after the stimuli are
updated, the canvas will use the [class@Artist] that is registered for that
specific [class@VisualStimulus]. So that the Artist can draw.

### Drawing with specific backends (OpenGL, Direct3D etc)

Every canvas has a [class@DrawingContext], that belongs to one specic drawing
backend of psylib. Currently, we have a [class@GtkWindow], that uses OpenGL to
draw it's stimuli. Artist can use the vertext buffer[class@VBuffer] provided by
the context so that the artist doesn't have to worry about whether the buffer
is compatible with OpenGL, it only has to specify and upload the vertices and use
the vertex buffer when ready.

### The drawing space of the stimuli

A canvas sets up the coordinate frame in which the drawing occurs. We have
chosen to take a orthogonal projection, that is we have opted for a choice in
which depth exist, but object further away do not look smaller than objects
nearby. Depth can be used to display the nearest picture. See the section on
[drawing overlapping stimuli](#drawing-overlapping-stimuli).

## Artists, the instances that do the drawing

The window has a number of registered classes, who are able to draw the stimuli.
Hence, for every stimulus, the window will create an Artist that is going to
draw the given stimulus. Currently, the windows have a number of classes that
are registered with the window when the window is created. So for every type
of derived from [class@VisualStimulus], there will be a suitable class derived
from [class@Artist] that knows how to draw the corresponding stimulus.

## VisualStimuli the set of parameters that tell the Artists how to draw

Artists are very capable to draw stimuli on canvasses, however, they need to
know what to draw, and this is what the [class@VisualStimulus] tells them.
Where the Artist use the drawing context in order to draw, the stimuli know
where, in what color, to draw the stimuli. So you can set Stimuli as a list
of parameters that the artist use in order to draw the stimuli.

## Drawing overlapping stimuli

One task drawing frameworks have to deal with how do overlapping stimuli behave.
Psylib has currently two parallel strategy's to deal with this situation.

1. For stimuli with the same z-value (0.0 by default), the order in which stimuli
   are drawn/registered to the window VisualStimuli are instances of [class@Stimulus],
   they can be "played" if the are played, they are scheduled with the canvas.
   So if we were to display two playing card of a deck we want to playing the
   stimuli is like throwing them on the table, the one that is played secondly,
   is displayed on top. The OpenGL windows use a depth buffer, that means the
   stimulus that is drawn first, will be displayed and the second stimulus
   that has the same depth value will no be closer in the depth buffer and
   hence it won't be drawn.
2. The z-value is always leading. In psylib, greater z-values are closer. Hence,
   using the playing card example, if the card that is played first has a greater
   z-value than the second it will still be shown on top.

## The mapping of VisualStimulus and Artist

### VisualStimuli

The base classes are [class@VisualStimulus] and [class@PsyArtist]. All visual
stimuli and artist derive from these classes respectively. The stimuli have
x, y and z values. Those coordinates are used to translate (shift) the stimuli
in to place. The stimuli have a scale x and a scale y attribute that make the
stimuli scale along the x- and y-axis and a scale attribute that scales both
along the x and y axis. Finally, all stimuli have a rotation around the z-axis,
so you be able to rotate an image around.

### Artists

The base Artist class uses a matrix to represent the transformations above,
so it will first translate a stimulus into position. Then scale it to the
desired size and finally rotate it around the z-axis in order to make the image
look good.

This also demonstrates the separation between Stimuli and Artists, Artists
are a much more lower level feature, whereas Stimuli contain relatively simple
parameters.

### Deriving classes

If you derive from [class@VisualStimulus], you'll also need to have an derived
[class@Artist], For example [class@Circle] derives from the VisualStimulus class
and the window has an accompanying [class@CircleArtist] that knows how to draw
a circle. A Circle has the same attributes as a VisualStimulus, so it can be
moved, scaled and rotated in position. But it adds a radius and a number of
vertices, so a final circle may be formed. So the circle will just be drawn
around the origin (0, 0). And scaling, translating and rotating, the stimulus
can be done without the need of sending new vertices to the GPU.

