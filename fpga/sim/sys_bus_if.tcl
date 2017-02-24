proc sys_bus_if {name path {params false}} {
  # parameters
  if {${params}} {
    add wave -noupdate -group ${name} -unsigned ${path}/DW
    add wave -noupdate -group ${name} -unsigned ${path}/AW
    add wave -noupdate -group ${name} -unsigned ${path}/SW
  }
  add wave -noupdate -group ${name}           ${path}/clk
  add wave -noupdate -group ${name}           ${path}/rstn
  add wave -noupdate -group ${name}           ${path}/wen
  add wave -noupdate -group ${name}           ${path}/ren
  add wave -noupdate -group ${name} -hex      ${path}/addr
  add wave -noupdate -group ${name} -hex      ${path}/wdata
  add wave -noupdate -group ${name} -hex      ${path}/rdata
  add wave -noupdate -group ${name}           ${path}/ack
  add wave -noupdate -group ${name}           ${path}/err
}
