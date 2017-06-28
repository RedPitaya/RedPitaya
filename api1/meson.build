project('Red Pitaya shared library', 'c', default_options : ['c_std=c99'])

cc = meson.get_compiler('c')
depm = cc.find_library('m', required : false)

dep = [dependency('libudev'), depm]
src = ['src/uio.c', 'src/evn.c', 'src/hwid.c', 'src/gen.c', 'src/asg_per.c', 'src/asg_bst.c', 'src/gen_out.c']
inc = include_directories(['include', 'src'])

lib = shared_library('rp', src, dependencies : dep, include_directories : inc, version : '1.2.3', soversion : '0')
executable('demo', 'src/main.c', dependencies : depm, link_with : lib)
