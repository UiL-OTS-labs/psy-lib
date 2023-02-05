# psy-lib
A GObject based library for psychological experiments

## Building psy-lib
Currently psy-lib is setup to run under Linux, support for windows is currently
planned. So the steps described here to get psy-lib compiled are the based
on the steps that psy-lib does in the continues integration.

0. You'll need a C and C++ compiler clang and gcc toolchains work, we plan
   support for msvc.
1. Install dependencies, you'll be needing the development libararies:
   - glib-2.0
   - gtk-4
   - libcunit1 (for the unittests)
2. You'll need python-3.10
3. You need the python packages from the "requirements.txt" file.
4. Run the meson build system
   - For a debug build:
```console
meson setup build
```
   - For a release build:
```console
meson setup build --buildtype=release
```
   - For other buildtypes please checkout the Meson buildsystem
5. Compile psy-lib
   - meson compile -C build


## This library is a work in progress and not yet nearly ready for any serious work
The main idea of the library is to have a C library for tight control over
all the relatively low level system libraries such as opengl and windowing libraries
to a generally friendly language to program in such as python. Although it's perfectly
possible to run an experiment using C. We would opt to use an interpreted language
such as python to run your experiments.
## Exposing the library can to other languages may be done using GObject introspection.
The psy-lib library should be exposed to a non-programmer friendly programming language
such as Python. In Python it should be possible to import the psylib module using the
GObject Introspection Repository (GIR)

```python
import gi
gi.require_version("Psy", "0.0")
from gi.repository import Psy
```
Other languages might use different ways to load libraries from a GIR. But libraries
based on GObjects are designed to be exported to other languages. For many languages
it is possible to run GObject based libraries.

# ToDo Figure out how we would like to run PsyLib programs.

- Do we really like to run an entire experiment from a main loop or do we like
  to run fragments inside a loop.
- It seems more appealing to say: Hey, stimulus, run for 50ms, than to wait for
  a vblank 3 times (in case of 60Hz) and draw such as in perhaps more typical OpenGL
  programs.
  
# figure out what requirements we like to meet
- Psylib sould aim to support multiple connected monitors. This is good for babylab where
  multiple monitors are used in looking time experiments. Also it is nice to support
  to choose on which monitor one would like to run the experiment.
- Psylib shoudd aim to be able to present a stimulus with millisecond precision, In a case such as
  a 60Hz monitor, this means that a stimulus is to be fitted onto one of the 60 frames
  presented in one second, however, we like to know on which millisecond the frame is presented.

# Figure out which libraries to use.
- How do we like to present sound. GStreamer fits nice in this niche as it is GObject
  based and fits nicely in this ecosystem
- We currently support OpenGL in combination with Gtk+-4.0 for visual stimuli.
  But perhaps we'd like to use Direct3D on windows.
  
  
