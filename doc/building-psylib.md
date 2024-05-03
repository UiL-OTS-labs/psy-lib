Title:Building psylib
SPDX-License-Identifier: MIT
SPDX-FileCopyrightText: 2024 Maarten Duijndam

# Building psylib from source

Psylib can be build from source. Psylib is mainly programmed in C, but uses a
number of C++ files along the way. It mostly uses C libraries, but a few
C++ libraries are included/used as well. So we've tried to look at a number of
build systems that can build psyib from source.
Currently psylib is developed on Linux and we are trying to build it on Windows.
You might find other platforms on which you are able to build it, but we try
to document on how to build psylib on Windows and Linux.

## The Meson Build system

We've been looking for a build system to use in order to build psylib. Since,
psylib depends very much on [GObject][1] for object orientation, and [GLib][2]
for internal algorithms and data structures, and both those two use meson to
build itself. Additionally, 

## Compiling on Linux
see [Building psylib on Linux](./building-psylib-on-linux.html)

## Compiling on Windows
see [Building psylib on Windows](./building-psylib-on-windows.html)

[1]: https://docs.gtk.org/gobject/
[2]: https://docs.gtk.org/glib/
[3]: https://mesonbuild.com/
