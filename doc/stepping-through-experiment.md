Title:Stepping through your experiment using psylib

# Introduction into the control flow in a psylib experiment

When you run a psylib experiment, you'll typically want to structure your task
in a fashion that suits your experiment. A possible experiment might consists
of the following parts:

1. Practice Instruction
1. Run a number of practice trials in a loop. A trial might look like:
     1. Present a stimulus
     1. When incorrect show feedback
1. When the performance of the practice wasn't optimal, go back to instruction.
   Otherwise, move on to the test instruction.
1. Run a loop with test trials.
     And here a stimulus is presented in a loop
1. Thank the participant for taking part in the experiment stop the program

This document tries to make sense of how to do this with using psylib.

## Every point in the experiment above is a Step in psylib

The parent of all steps is the [class@Step]. You may enter a step and leave
a step. To enter a step means that you want to activate it. On activation
of a simple trial, you want to present some stimuli, and once that is done
you typically want to leave that trial. Leaving a step means to enter
a next step. The main trials of an experiment are [class@SteppingStones],
[class@Loop] and [class@Trial]. A trial is more or less a concrete kind of step.

### Stepping Stones

The entire list of steps in the experiment above may be considered as a
[class@SteppingStones] object. The SteppingStones are steps you can go through
in the order other steps are added to the experiment. You can also add steps
with a key - a string that is meaningful to you - such as "prac instruction"
to identify the instruction before the practice loop. So when you leave the
practice block, you get the chance to see whether the participant did it
good enough to proceed to the test phase, or whether you want to jump back to
the practice fase.
The stepping stones are pretty flexible, you could run an entire experiment
with just adding a lot of steps to the stepping stones.

### Loops


### Trials


## Meta Steps

This is a todo we could add steps just for the sake of jumping to it. A
**PsyJumpStep**. The idea is that when you enter the step, it will just activate the
next step. So when you have it in your stepping stones object, it won't do
anything, but to step to the next step.
**ApplicationStep** A step that will nicely quit the experiment when leaving
it. It should be similar to [class@SteppingStones], but it will end the
program when it is done.

