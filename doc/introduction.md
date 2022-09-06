Title:Introduction

# PsyLib introduction
Psylib is essentially a C library that aims to be a good candidate for
psychological research, as it can be quick, precise and may access low
level system libraries. However, psychological research is often conducted
by people who might not feel at ease when using C. Psylib is using the GObject
library in order to use GObject Introspection. GObject introspection creates
language bindings, to programming languages familiar with researchers in the
field, such as python.

# Object orientation
Although, C is not an object oriented programming language by design,
the GObject type system, does allow to create types or instances of types.
So a PsyClock is implemented in C using a "class" structure and an
instance structure. The GObject introspection allows to create a repository
for scripting languages. So one can import the gi in a language
such as python. And then from the repository, one can import PsyLib as
a module and from the module types Such as Clock as a class.

```python
import gi
gi.require_version("Psy", "0.1") # specify a specific version
from gi.repository import Psy    # import the LibPsy library

clock = Psy.Clock()              # create an instance of PsyClock
now = clock.now()                # get the current time.
                                 # Note that now is an instance of the C
                                 # PsyTimepoint or in python speak Psy.Timepoint
```

 
