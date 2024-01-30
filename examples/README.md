# Examples

One of the nice things about psylib is it's ability to write relatively low
level code in C(, C++ and rust), and export it to relatively high level
languages such as Python. Than the magic of GObject introspection maps the C
types to the target language and you can use psylib with your preferred language

## Python 3.x

The most important languages is Python for example, you can write python code
and use very low level code in order to do drawing, present audio etc in a
pretty efficient way. Efficiency isn't the most important detail about psylib,
but using lowlevel features is.

So in the python sub folder there are a number of python examples so that
you may get an idea of how to run your code with python.

## C/C++

Although, we wouldn't recommend C nor C++ as a language to program your
experiment, you can most certainly do it. Some GObject libraries have special
bindings for C++ such as Gtk's Gtkmm, however we do not yet plan such an
endeavor. You can use the plain C library from C++ though.

The c folder shows some examples of how to run you experiment using C/C++

## Add your language where psylib works here
