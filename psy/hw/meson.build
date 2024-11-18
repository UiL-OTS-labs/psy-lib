
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

if host_machine.system() in ['windows', 'cygwin']
    cpu_family = host_machine.cpu_family()
    if cpu_family == 'x86'
        fs.copyfile('inpout32.dll', install: true, install_dir: 'psy')
    elif cpu_family == 'x86_64'
        fs.copyfile('inpout32.dll', install: true, install_dir : 'psy')
    endif
    libpsyfiles += files('psy-inpout-port.c')
    libpsy_headers += files('psy-inpout-port.h')
endif
