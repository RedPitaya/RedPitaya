cc = meson.get_compiler('c')
depm = cc.find_library('m', required : false)

dep = [dependency('libudev'), depm]
src = ['src/util.c',
       'src/uio.c',
       'src/evn.c',
       'src/hwid.c',
       'src/gen.c',
       'src/asg_per.c',
       'src/asg_bst.c',
       'src/gen_out.c',
       'src/wave.c',
       'src/osc.c',
       'src/acq.c',
       'src/osc_trg.c',
       'src/osc_fil.c',
       'src/la.c',
       'src/la_trg.c']
inc = include_directories(['include', 'src'])

librp1_shared = shared_library('rp1', src, dependencies : dep, include_directories : inc,
                               version : '0.1.0', soversion : '0',
                               install : true)
librp1_static = static_library('rp1', src, dependencies : dep, include_directories : inc,
                               install : true)

librp1_header = ['include/redpitaya/rp1.h',
                 'include/redpitaya/util.h',
                 'include/redpitaya/uio.h',
                 'include/redpitaya/evn.h',
                 'include/redpitaya/hwid.h',
                 'include/redpitaya/gen.h',
                 'include/redpitaya/asg_per.h',
                 'include/redpitaya/asg_bst.h',
                 'include/redpitaya/gen_out.h',
                 'include/redpitaya/wave.h',
                 'include/redpitaya/osc.h',
                 'include/redpitaya/acq.h',
                 'include/redpitaya/osc_trg.h',
                 'include/redpitaya/osc_fil.h',
                 'include/redpitaya/la.h',
                 'include/redpitaya/la_trg.h']
install_headers(librp1_header, subdir : 'redpitaya')

# this is only usefull for installing on a FAT partition
# TODO: it should be removed, if/when possible
meson.add_install_script('install_vfat.sh')

executable('demo', 'src/main.c', dependencies : depm, include_directories : inc, link_with : librp1_shared)

incrp1 = include_directories(['include'])
deprp1_shared = declare_dependency(link_with : librp1_shared, include_directories : incrp1, dependencies : dep)
deprp1_static = declare_dependency(link_with : librp1_static, include_directories : incrp1, dependencies : dep)
