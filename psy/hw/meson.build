
libpsyfiles += files(
    'psy-parallel-port.c',
    'psy-parallel-trigger.c',
    'psy-serial-port.c',
    'psy-teensy-trigger.c',
)

libpsy_headers += files(
    'psy-parallel-port.h',
    'psy-parallel-trigger.h',
    'psy-serial-port.h',
    'psy-teensy-trigger.h',
)

if cdata.has('HAVE_LINUX_PARPORT_H') and cdata.has('HAVE_LINUX_PPDEV_H')
    libpsyfiles += files('psy-parport.c')
    libpsy_headers += files('psy-parport.h')
endif

if cdata.has('HAVE_TERMIOS_H')
    libpsyfiles += files('psy-termios.c')
    libpsy_headers += files('psy-termios.h')
endif

if host_machine.system() in ['windows', 'cygwin']
    libpsyfiles += files('psy-inpout-port.c')
    libpsy_headers += files('psy-inpout-port.h')
endif
