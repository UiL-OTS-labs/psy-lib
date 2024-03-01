
EXAMPLE_PP_ARGS = [
    '-DG_LOG_DOMAIN="Psy-Example"',
    '-DG_LOG_USE_STRUCTURED'
]

loop = executable (
    'loop',
    files (
        'loop.c',
    ),
    dependencies : [psy_dep],
    c_args : EXAMPLE_PP_ARGS,
)

nested_loops = executable (
    'nested_loops',
    files (
        'nested-loops.c',
    ),
    dependencies : [psy_dep],
    c_args : EXAMPLE_PP_ARGS,
)

test(
   'c-loop-example',
   loop, 
   timeout : 5,
   suite : ['examples', 'c-examples'],
)

test(
   'c-nested-loops-example',
   nested_loops, 
   timeout : 5,
   suite : ['examples', 'c-examples'],
)


