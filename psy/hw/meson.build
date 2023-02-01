
libpsyfiles += files(
    'psy-parallel-port.c',
    'psy-parallel-trigger.c'
)

libpsy_headers += files(
    'psy-parallel-port.h',
    'psy-parallel-trigger.h'
)

if cdata.has('HAVE_LINUX_PARPORT_H') and cdata.has('HAVE_LINUX_PPDEV_H')
    libpsyfiles += files('psy-parport.c')
    libpsy_headers += files('psy-parport.h')
endif
