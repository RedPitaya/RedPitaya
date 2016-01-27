proc sys_bus_if {name path} {
  # add interface group 
  add wave -noupdate -expand -group ${name} ${path}/*
  # difine Radix
  radix signal ${path}/addr  -hexadecimal
  radix signal ${path}/wdata -hexadecimal
  radix signal ${path}/rdata -hexadecimal
  radix signal ${path}/DW -decimal -unsigned
  radix signal ${path}/AW -decimal -unsigned
  radix signal ${path}/SW -decimal -unsigned
}
