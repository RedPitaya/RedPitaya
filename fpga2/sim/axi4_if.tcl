proc axi4_if {name path} {
  # add interface group 
  add wave -noupdate -expand -group ${name} ${path}/*
  add wave -noupdate -expand -group ${name} -group param ${path}/AW
  add wave -noupdate -expand -group ${name} -group param ${path}/DW
  add wave -noupdate -expand -group ${name} -group param ${path}/SW
  add wave -noupdate -expand -group ${name} -group param ${path}/IW
  add wave -noupdate -expand -group ${name} -group param ${path}/LW
  add wave -noupdate -expand -group ${name} -group AW    ${path}/AW*
  add wave -noupdate -expand -group ${name} -group  W    ${path}/W*
  add wave -noupdate -expand -group ${name} -group  B    ${path}/B*
  add wave -noupdate -expand -group ${name} -group AR    ${path}/AR*
  add wave -noupdate -expand -group ${name} -group  R    ${path}/R*
  # difine Radix
  radix signal ${path}/AWADDR -hexadecimal
  radix signal ${path}/ARADDR -hexadecimal
  radix signal ${path}/WDATA -hexadecimal
  radix signal ${path}/RDATA -hexadecimal
  radix signal ${path}/DW -decimal -unsigned
  radix signal ${path}/AW -decimal -unsigned
  radix signal ${path}/SW -decimal -unsigned
  radix signal ${path}/IW -decimal -unsigned
  radix signal ${path}/LW -decimal -unsigned
}
