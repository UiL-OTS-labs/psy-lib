
libpsyfiles += files(
    'parallel-port.c',
)

libpsy_headers += files(
    'parallel-port.h',
)

if cdata.has('HAVE_LINUX_PARPORT_H') and cdata.has('HAVE_LINUX_PPDEV_H')
    libpsyfiles += files('parport.c')
    libpsy_headers += files('parport.h')
endif
