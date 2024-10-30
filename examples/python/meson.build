

python3 = find_program('python3')

if build_machine.system() != 'windows' and build_machine.system() != 'cygwin'

    test(
       'python-step-example',
       python3, 
       timeout : 5,
       suite : ['examples', 'python-examples'],
       args : [meson.current_source_dir() + '/stepping.py'],
    )

    test(
       'python-step-inheritance-example',
       python3, 
       timeout : 5,
       suite : ['examples', 'python-examples'],
       args : [meson.current_source_dir() + '/stepping-inheritance.py'],
)

endif

test(
   'python-loop-example',
   python3, 
   timeout : 5,
   suite : ['examples', 'python-examples'],
   args : [meson.current_source_dir() + '/loop.py'],
)

test(
   'python-loop-child',
   python3, 
   timeout : 5,
   suite : ['examples', 'python-examples'],
   args : [meson.current_source_dir() + '/loop-child.py'],
)

test(
   'python-loop-indices',
   python3, 
   timeout : 5,
   suite : ['examples', 'python-examples'],
   args : [meson.current_source_dir() + '/loop-indices.py'],
)
