project('redpitaya shared library', 'c')

dep = dependency('libudev')
src = ['src/uio.c', 'src/hwid.c', 'src/gen.c']
inc = include_directories(['include', 'src'])

lib = shared_library('rp', src, dependencies : dep, include_directories : inc, version : '1.2.3', soversion : '0')
executable('demo', 'src/main.c', link_with : lib)
