cc = meson.get_compiler('c')
depm = cc.find_library('m', required : false)

scpi_parser = subproject('scpi-parser')

depscpi_shared = scpi_parser.get_variable('depscpi_shared')
#depscpi_static = scpi_parser.get_variable('depscpi_static')

dep = [deprp1_shared, depscpi_shared, depm]

src = ['scpi-commands.c',
       'scpi-server.c',
       'scpi_gen.c']

inc = include_directories('') 

executable('scpi', src, dependencies : dep, include_directories : inc, install : true)
