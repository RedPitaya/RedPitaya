proc axi4_stream_if {name path} {
  # add interface group 
  add wave -noupdate -expand -group ${name} ${path}/*
  # difine Radix
  radix signal ${path}/TDATA -hexadecimal
}
