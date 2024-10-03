Title:Building psylib on Windows
SPDX-License-Identifier: MIT
SPDX-FileCopyrightText: 2024 Maarten Duijndam

#Building psylib on Windows

There are probably many ways to get psylib to compile on windows. We are going 
to explore, howto compile using [msys2][1] and [MinGW-w64][2] and try to 
install compile it using the VisualStudio backend of the Meson build system.

Microsoft has developer [virtual machine][3] images available. These vm's are
for developers are valid for a limited period. This guide is written with
a Windows 11 vm running using virtual box or QEMU/KVM using virtmanager.

## Building using msys2 and MinGW-w64

The following steps are used to build psylib using msys2 and the toolchains
listed in the [msys2-environments][4]

### install [msys2][1]

When msys2 is installed, you can find different environments in the msys2 folder
in the start bar. This example uses the msys2 UCRT64 environment.  To allow building a
library for an `x86_64` architecture using the ucrt C library and libstdc++ as
standard C++ library.

Open a mintty terminal: When you click the start menu/bar, (all apps) go to
MSYS2->MSYS2_UCRT64.


```bash
# install git
pacman -S git

# cd to the directory where to git clone psylib
mkdir git
cd git

# install pactoys for installing packages with short name
pacman -S pactoys


# clone psylib
 git clone git@github.com:UiL-OTS-labs/psy-lib.git
cd psy-lib

#install python within msys environment
pacboy -S python:p python-pip:p

#install gcc/g++ or clang etc. depending on the chose environment.
pacboy -S toolchain:p

#install required packages
pacboy -S glib2:p\
          gstreamer:p\
          gst-plugins-base:p\
          gst-plugins-good:p\
          gtk4:p\
          libepoxy:p\
          portaudio:p\
          boost:p\
          gobject-introspection:p

# optional for unit tests
pacboy -S cunit:p

# optional for documentation
packboy -S gi-docgen:p

#finally install CMake (for ninja), ninja and meson
pacboy -S cmake:p\
          meson:p\
          ninja:p

# configure the build directory you
meson setup build -Dalsa=false

# cd to the build directory
cd build
ninja
```

[1]: https://msys2.org
[2]: https://MinGW-w64.org
[3]: https://developer.microsoft.com/en-us/windows/downloads/virtual-machines/
[4]: https://www.msys2.org/docs/environments/#__tabbed_1_1
