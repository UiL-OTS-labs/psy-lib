Title:Testing psylib
SPDX-License-Identifier: MIT
SPDX-FileCopyrightText: 2024 Maarten Duijndam

# Testing psylib

Before running the tests make sure you are able to configure and compile a build first.
In order to run all unit tests and examples - the examples are also tested in CI -  you
will have to setup an environment where you can run the tests. If you don't e.g.
libpsy.dll isn't found isn't found on windows. Additionally in order to get the python
(and in the future maybe other) examples, the GI_TYPELIB_PATH must be setup.
meson helps out in this respect. Consider that you have builddir is called "debug" and
your in the root of psylib. You can activate the environment by running:

```bash
meson devenv -C debug
```

Now you can run the tests with: `meson test`, or you can do use one command to run
everything and step out of the debug environment by running:

```bash
meson devenv -C debug meson test
```
In order to run a release instead of debug, replace debug with release in the commands
above.

## Issues on Ubuntu-22.04 with clang address sanitizers

The test can be run using the ASAN and LSAN tools in order to inspect
issues related to memory issues. On Ubuntu 22.04 this sometimes crashes with a great
number of errors being printed.

```
AddressSanitizer:DEADLYSIGNAL
```

There is a [work around](https://github.com/actions/runner-images/issues/9524#issuecomment-2002475952)
posted for this, because it came out in a lot of CI of other projects.
The workaround with the following command:

```bash
sudo sysctl vm.mmap_rnd_bits=28
```
