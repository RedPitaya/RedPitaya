proc axi4_lite_if {name path {params false}} {
  # parameters
  if {${params}} {
    add wave -noupdate -group ${name} -group param -unsigned ${path}/AW
    add wave -noupdate -group ${name} -group param -unsigned ${path}/DW
    add wave -noupdate -group ${name} -group param -unsigned ${path}/SW
  }
  # system signals
  add wave -noupdate -group ${name}                        ${path}/ACLK
  add wave -noupdate -group ${name}                        ${path}/ARESETn
  # write address channel
  add wave -noupdate -group ${name} -group AW    -hex      ${path}/AWADDR
  add wave -noupdate -group ${name} -group AW              ${path}/AWPROT 
  add wave -noupdate -group ${name} -group AW              ${path}/AWVALID
  add wave -noupdate -group ${name} -group AW              ${path}/AWREADY
  # write data channel
  add wave -noupdate -group ${name} -group  W    -hex      ${path}/WDATA
  add wave -noupdate -group ${name} -group  W              ${path}/WSTRB
  add wave -noupdate -group ${name} -group  W              ${path}/WVALID
  add wave -noupdate -group ${name} -group  W              ${path}/WREADY
  # write response channel
  add wave -noupdate -group ${name} -group  B              ${path}/BRESP
  add wave -noupdate -group ${name} -group  B              ${path}/BVALID
  add wave -noupdate -group ${name} -group  B              ${path}/BREADY
  # read address channel
  add wave -noupdate -group ${name} -group AR    -hex      ${path}/AWADDR
  add wave -noupdate -group ${name} -group AR              ${path}/AWPROT 
  add wave -noupdate -group ${name} -group AR              ${path}/AWVALID
  add wave -noupdate -group ${name} -group AR              ${path}/AWREADY
  # read data channel
  add wave -noupdate -group ${name} -group  R    -hex      ${path}/RDATA
  add wave -noupdate -group ${name} -group  R              ${path}/RRESP
  add wave -noupdate -group ${name} -group  R              ${path}/RVALID
  add wave -noupdate -group ${name} -group  R              ${path}/RREADY
}
