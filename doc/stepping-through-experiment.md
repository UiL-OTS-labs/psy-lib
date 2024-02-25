Title:Stepping through your experiment

# Introduction into the control flow in a psylib experiment

When you run a psylib experiment, you'll typically want to structure your task
in a fashion that suits your experiment. A possible experiment might consist
of the following parts:

1. Practice Instruction
1. Run a number of practice trials in a loop. A trial might look like:
     1. Present a stimulus
     1. When incorrect show feedback
1. When the performance of the practice wasn't optimal, go back to instruction.
   Otherwise, move on to the test instruction.
1. Run a loop with test trials.
   And here a stimulus is presented in each iteration of a loop.
1. Thank the participant for taking part in the experiment stop the program

This document tries to make sense of how to do this with using psylib.

## Every point in the experiment above is a Step in psylib

The parent of all steps is the [class@Step]. You may enter a step and leave
a step. To enter a step means that you want to activate it. On activation
of a simple trial, you want to present some stimuli, and once that is done
you typically want to leave that trial. Leaving a step means to enter
a next step. The main trials of an experiment are [class@SteppingStones],
[class@Loop] and [class@Trial]. A trial is more or less a concrete kind of step.

## Trials

[class@Trial]s, are designed to run one trial in an experiment. They are,
more or less a plain [class@Step], with the only exception that a Step is an
abstract class whereas a Trial is a concrete class - it may be instantiated.

## Stepping Stones

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

## Loops

[class@Loop]s are about repeating a particular step. The loop is iterating until
the running condition isn't met anymore. You can use a loop a bit in the sense
of a C(++), Java style for loop. You set the (start) **index**, an **increment**
that the index will be incremented with after each iteration of the loop and a
**stop** on a loop. Finally, you can set a predefined manner to test the
condition whether the loop should continue to run or a next step should
be activated.

## Note about Steps that take children

There are a number of steps that take other instances of [class@Step] as
a substep/child. What is important is that every instance of [class@Step]
can only have one parent, so when adding a step to an instance of
[class@Loop] or [class@SteppingStones], it should not have been added to another
step. The reason is, most likely when the substep is done, it will only
reactivate the parent to which it would have been added as last, and not
the parent to which it has been added before. Just create a new instance
of the step when you want one step to be added.

## Meta Steps

This is a todo we could add steps just for the sake of jumping to it. A
**PsySideStep**. The idea is that when you enter/activate the step, it will
automatically activate the next step. So when you have it in your stepping
stones object, it won't do anything, but to step to the next step. You
you might add it by name to an instance of [class@SteppingStones], so
you can easily jump to it. You might just add it to do check something
You may still connect to the [sigal@Step::activate] or override the virtual
activate method  (as long as you chain the activate up to its parent).
So you typically use this step for it side effects, hence the name.
**ApplicationStep** A step that will nicely quit the experiment when leaving
it. It should be similar to [class@SteppingStones], but it will end the
program when it is done.

