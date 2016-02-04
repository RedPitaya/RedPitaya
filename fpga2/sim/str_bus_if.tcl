proc str_bus_if {name path} {
# add wave -noupdate -group ${name}           ${path}/DAT_T
  add wave -noupdate -group ${name} -unsigned ${path}/DN
  add wave -noupdate -group ${name}           ${path}/clk
  add wave -noupdate -group ${name}           ${path}/rstn
  add wave -noupdate -group ${name} -hex      ${path}/dat
  add wave -noupdate -group ${name}           ${path}/kep
  add wave -noupdate -group ${name}           ${path}/lst
  add wave -noupdate -group ${name}           ${path}/vld
  add wave -noupdate -group ${name}           ${path}/rdy
  add wave -noupdate -group ${name}           ${path}/trn
}
