proc axi4_stream_if {name path {params false}} {
  # parameters
  if {${params}} {
  # add wave -noupdate -group ${name}           ${path}/DT
    add wave -noupdate -group ${name} -unsigned ${path}/DN
  }
  # system signals
  add wave -noupdate -group ${name}           ${path}/ACLK
  add wave -noupdate -group ${name}           ${path}/ARESETn
  # stream
  add wave -noupdate -group ${name} -hex      ${path}/TDATA
  add wave -noupdate -group ${name}           ${path}/TKEEP
  add wave -noupdate -group ${name}           ${path}/TLAST
  add wave -noupdate -group ${name}           ${path}/TVALID
  add wave -noupdate -group ${name}           ${path}/TREADY
  add wave -noupdate -group ${name}           ${path}/transf
}
