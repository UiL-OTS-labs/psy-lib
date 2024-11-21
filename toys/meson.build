
sleep_time = executable('sleep-time', files('sleep-time.c'), dependencies:[psy_dep])

wbemuuid_dep = cc.find_library('wbemuuid', static:true)
wmi_parport= executable(
    'wmi_parallel_port',
    files('wmi-parallel-port.c'),
    dependencies: wbemuuid_dep
)

