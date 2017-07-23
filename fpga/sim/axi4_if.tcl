proc axi4_if {name path {params false}} {
  # parameters
  if {${params}} {
    add wave -noupdate -group ${name} -group param -unsigned ${path}/AW
    add wave -noupdate -group ${name} -group param -unsigned ${path}/DW
    add wave -noupdate -group ${name} -group param -unsigned ${path}/SW
    add wave -noupdate -group ${name} -group param -unsigned ${path}/IW
    add wave -noupdate -group ${name} -group param -unsigned ${path}/LW
  }
  # system signals
  add wave -noupdate -group ${name}                        ${path}/ACLK
  add wave -noupdate -group ${name}                        ${path}/ARESETn
  # write address channel
  add wave -noupdate -group ${name} -group AW              ${path}/AWID
  add wave -noupdate -group ${name} -group AW    -hex      ${path}/AWADDR
  add wave -noupdate -group ${name} -group AW              ${path}/AWREGION
  add wave -noupdate -group ${name} -group AW              ${path}/AWLEN
  add wave -noupdate -group ${name} -group AW              ${path}/AWSIZE
  add wave -noupdate -group ${name} -group AW              ${path}/AWBURST
  add wave -noupdate -group ${name} -group AW              ${path}/AWLOCK
  add wave -noupdate -group ${name} -group AW              ${path}/AWCACHE
  add wave -noupdate -group ${name} -group AW              ${path}/AWPROT
  add wave -noupdate -group ${name} -group AW              ${path}/AWQOS
  add wave -noupdate -group ${name} -group AW              ${path}/AWVALID
  add wave -noupdate -group ${name} -group AW              ${path}/AWREADY
  # write data channel
  add wave -noupdate -group ${name} -group  W              ${path}/WID
  add wave -noupdate -group ${name} -group  W    -hex      ${path}/WDATA
  add wave -noupdate -group ${name} -group  W              ${path}/WSTRB
  add wave -noupdate -group ${name} -group  W              ${path}/WLAST
  add wave -noupdate -group ${name} -group  W              ${path}/WVALID
  add wave -noupdate -group ${name} -group  W              ${path}/WREADY
  # write response channel
  add wave -noupdate -group ${name} -group  B              ${path}/BID
  add wave -noupdate -group ${name} -group  B              ${path}/BRESP
  add wave -noupdate -group ${name} -group  B              ${path}/BVALID
  add wave -noupdate -group ${name} -group  B              ${path}/BREADY
  # read address channel
  add wave -noupdate -group ${name} -group AR              ${path}/ARID
  add wave -noupdate -group ${name} -group AR    -hex      ${path}/ARADDR
  add wave -noupdate -group ${name} -group AR              ${path}/ARREGION
  add wave -noupdate -group ${name} -group AR              ${path}/ARLEN
  add wave -noupdate -group ${name} -group AR              ${path}/ARSIZE
  add wave -noupdate -group ${name} -group AR              ${path}/ARBURST
  add wave -noupdate -group ${name} -group AR              ${path}/ARLOCK
  add wave -noupdate -group ${name} -group AR              ${path}/ARCACHE
  add wave -noupdate -group ${name} -group AR              ${path}/ARPROT
  add wave -noupdate -group ${name} -group AR              ${path}/ARQOS
  add wave -noupdate -group ${name} -group AR              ${path}/ARVALID
  add wave -noupdate -group ${name} -group AR              ${path}/ARREADY
  # read data channel
  add wave -noupdate -group ${name} -group  R              ${path}/RID
  add wave -noupdate -group ${name} -group  R    -hex      ${path}/RDATA
  add wave -noupdate -group ${name} -group  R              ${path}/RRESP
  add wave -noupdate -group ${name} -group  R              ${path}/RLAST
  add wave -noupdate -group ${name} -group  R              ${path}/RVALID
  add wave -noupdate -group ${name} -group  R              ${path}/RREADY
}
