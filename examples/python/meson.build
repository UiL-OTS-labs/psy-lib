

gi_typelib_path = library_build_dir
ld_library_path = library_build_dir

python3 = find_program('python3')

test(
   'python-step-example',
   python3, 
   timeout : 5,
   suite : ['examples', 'python-examples'],
   args : [meson.current_source_dir() + '/stepping.py'],
   env : {
       'GI_TYPELIB_PATH' : gi_typelib_path,
       'LD_LIBRARY_PATH' : ld_library_path
   }
)

test(
   'python-step-inheritance-example',
   python3, 
   timeout : 5,
   suite : ['examples', 'python-examples'],
   args : [meson.current_source_dir() + '/stepping-inheritance.py'],
   env : {
       'GI_TYPELIB_PATH' : gi_typelib_path,
       'LD_LIBRARY_PATH' : ld_library_path
   }
)

test(
   'python-loop-example',
   python3, 
   timeout : 5,
   suite : ['examples', 'python-examples'],
   args : [meson.current_source_dir() + '/loop.py'],
   env : {
       'GI_TYPELIB_PATH' : gi_typelib_path,
       'LD_LIBRARY_PATH' : ld_library_path
   }
)

test(
   'python-loop-child',
   python3, 
   timeout : 5,
   suite : ['examples', 'python-examples'],
   args : [meson.current_source_dir() + '/loop-child.py'],
   env : {
       'GI_TYPELIB_PATH' : gi_typelib_path,
       'LD_LIBRARY_PATH' : ld_library_path
   }
)

test(
   'python-loop-indices',
   python3, 
   timeout : 5,
   suite : ['examples', 'python-examples'],
   args : [meson.current_source_dir() + '/loop-indices.py'],
   env : {
       'GI_TYPELIB_PATH' : gi_typelib_path,
       'LD_LIBRARY_PATH' : ld_library_path
   }
)
