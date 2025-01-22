# Testing the timing of visual stimuli

## Introduction to display synchronisation

This folder is called test_vsync. The vsync is short for the vertical synchronisation
of the monitor. This stems back from the time that monitors were Catode Ray Tube's
(CRTs), where a beam of electrons were shot at a phosphorous layer in the near end of
your monitor. When the layer is hit it briefly lights up and this is what we see. 
Screen savers where invented in order to prevent burning in of an image that didn't
change over time. During the vsync the ray goes back from the bottom of the screen to
the top (its a vertical movement hence vsync.

The beam doesn't only go from the top to the bottom, it also moves from the left to
the right. This is the hsync for horizontal synchronization. When a hsync happens
the line below the previous is being written. So for each line in your display
its written from left to right and all lines are written from top to bottom.

There is an analogy with a [typewriter][1]. Each time when an hsync happens, is the same
as pushing the lever on a typewriter to make the typing advance to the start of the
next line. In this sense, a vsync is much like replacing the paper and start at the
top of a new paper.

### Double buffering

Double buffering is a technical feature to make appealing graphics.
To continue with our typewriter analogy. It's isn't easy to read along with the 
text on a typewriter when you are not the one typing it. You could consider a psylib
program to be the person operating a typewriter, you participant is your editor. As
a writer you want your editor to be pleased with the results. So changing the text
while your editor is reading it doesn't give her (or him) a pleasant result.

So what happens in practice is, you have two pieces of paper. One that your editor is
reading and one you are currently typing to. Or in psylib's view, one memory buffer we
are drawing to and one that is currently on screen where your participant
is watching to. Now each time the when the vsync is happening, you hand out your
paper - on which you have been typing last the last frame to - to your editor and you
get her previous copy back.

While the editor is reading the new page/ your participant is looking
at the previously drawn frame, you start to write a new frame. So you first erase
the paper (ok this doesn't happen in real pre eighties tech, you'd just pick a new
paper) and start to write the next page/frame.

The piece of paper you are writing to is called the back buffer, the page/buffer being
displayed to your editor/participant is called the front buffer. The front buffer
isn't updated anymore, giving one consistent drawing.
So what is important is that each drawing is finished before the vsync happens
in order to successfully exchange the papers/buffers.

### Compositors a new technology to make life easier on the editor/participant

Using modern desktop computer environments, give modern problems. These days, you
cannot hand your page to your editor directly, she has a secretary. He makes
sure the page is read at an appropriate time to your editor, when she has time.
The secretary sometimes need to take his kids to the daycare, is sick or finds the
pages of other writers more important. He make sure your editor reads nice pages, 
but as a computer program you are much although you have finished your page in time
it is hard to see whether your editor reads the page on time. So although you nicely
wait for the vsync etc, its harder to tell whether your editor reads the page in time
or it is still being processed by her secretary.

The secretary is the compositor, he makes the life of the editor a bit easier, some
jobs are handled a bit more efficiently. He makes sure all pages (including those of 
other programs/writers are complete so the eyes have a good looking experience.) But
it makes our job very complicated as every OS or Desktop environment has a secretary
of there own. So all journals (OS's/Desktop environments) to which you would like to
send your articles handle a bit differently and have a different latency. So we have
to determine for each whether the pages is rendered and presented on time.

## Measuring the display latency

In the early days, we would like to wait to the vsync and swap the buffers and then we
were sure the stimuli are presented on time. This was typically done by alternating
a black and a white stimulus. If we were able to see that every 1/60th of a second 
(60 Hz is a typical refreshrate), the stimulus alternates between white and black we
used to be happy. It's still a good sign. But it might be the case that every
stimulus is 1, 2 or x frames late. Now it could be that all frames when
presented by us are all a bit late, they are rendered to the compositor and not
directly to the display.

### Measurement strategy
The windows try to present a window on a given moment. On the same moment, we
try to present a pulse on a trigger interface. We can compare with a photo sensor
the time when the square is visible to the time when pulses is emitted.

## Other things to consider

### power settings

1. When using Ubuntu-22.04 with Wayland running a fullscreen SDL window occasionally
   seems to drop frames. When running a windowed window (not fullscreen) this doesn't
   seem to happen. It also doesn't seem to happen when using Ubuntu on XOrg.
   **the fix** was to put the performance setting to **performance** instead of
   *balanced* or *powersaver*.

[1]: https://www.youtube.com/watch?v=FkUXn5bOwzk
