
libpsyfiles += files(
    'psy-parallel-port.c',
)

libpsy_headers += files(
    'psy-parallel-port.h',
)

if cdata.has('HAVE_LINUX_PARPORT_H') and cdata.has('HAVE_LINUX_PPDEV_H')
    libpsyfiles += files('psy-parport.c')
    libpsy_headers += files('psy-parport.h')
endif
