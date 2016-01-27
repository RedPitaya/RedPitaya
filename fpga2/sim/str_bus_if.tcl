proc str_bus_if {name path} {
  # add interface group 
  add wave -noupdate -expand -group ${name} ${path}/*
  # difine Radix
  radix signal ${path}/dat -hexadecimal
}
