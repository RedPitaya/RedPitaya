###############################################################################################################
# Core-Level Timing Constraints for axi_clock_converter Component "system_auto_cc_0"
###############################################################################################################
#
# This component is not configured to perform asynchronous clock-domain-crossing.
# No timing core-level constraints are needed.
# (Synchronous clock-domain-crossings, if any, remain covered by your system-level PERIOD constraints.)
create_waiver -internal -scope -type CDC -id CDC-10 -user axi_clock_converter -tags "1024161" -to [get_pins -quiet *gen_clock_conv.gen_async_conv.asyncfifo_axi/inst_fifo_gen/gaxi_full_lite.g*_ch.g*ch2.axi_*/grf.rf/rstblk/ngwrdrst.grst.g7serrst.gnsckt_*_reg2_inst/arststages_ff_reg[0]/PRE]\
-description {Waiving CDC-10 Although there is combo logic going into FIFO Gen reset, the expectation/rule is that the reset signal will be held for 1 clk cycles on the slowest clock.  Hence there should not be any issues cause by this logic}

create_waiver -internal -scope -type CDC -id CDC-11 -user axi_clock_converter -tags "1024161" -to [get_pins -quiet *gen_clock_conv.gen_async_conv.asyncfifo_axi/inst_fifo_gen/gaxi_full_lite.g*_ch.g*ch2.axi_*/grf.rf/rstblk/ngwrdrst.grst.g7serrst.gnsckt_*_reg2_inst/arststages_ff_reg[0]/PRE]\
-description {Waiving CDC-11 Although there is combo logic going into FIFO Gen reset, the expectation/rule is that the reset signal will be held for 1 clk cycles on the slowest clock.  Hence there should not be any issues cause by this logic}

create_waiver -internal -scope -type CDC -id CDC-15 -user axi_clock_converter -tags "1024442" -from [get_pins -quiet *gen_clock_conv.gen_async_conv.asyncfifo_axi/inst_fifo_gen/gaxi_full_lite.g*_ch.g*ch2.axi_*/grf.rf/gntv_or_sync_fifo.mem/gdm.dm_gen.dm/RAM_reg_0_15_*/RAM*/CLK]\ 
-to [get_pins -quiet *gen_clock_conv.gen_async_conv.asyncfifo_axi/inst_fifo_gen/gaxi_full_lite.g*_ch.g*ch2.axi_*/grf.rf/gntv_or_sync_fifo.mem/gdm.dm_gen.dm/gpr1.dout_i_reg*/D]\
-description {Waiving CDC-15 Timing constraints are processed during implementation, not synthesis. The xdc is marked only to be used during implementation, as advised by the XDC folks at the time.}

create_waiver -internal -scope -type METHODOLOGY -id {LUTAR-1} -user "axi_clock_converter" -desc {the pathway is completely within fifo-gen, and that path is present dual-clock usage}\
-tags "1024444"\
 -objects [get_cells -hierarchical "*gen_clock_conv.gen_async_conv.asyncfifo_axi*"] \
 -objects [get_pins -hierarchical * -filter "(NAME=~*gen_clock_conv.gen_async_conv.asyncfifo_axi/inst_fifo_gen/gaxi_full_lite.g*_ch.g*ch2.axi_*/grf.rf/rstblk/ngwrdrst.grst.g7serrst.gnsckt_wrst.gic_rst.rst_rd_reg2_inst/arststages_ff_reg*/PRE) || (NAME=~*gen_clock_conv.gen_async_conv.asyncfifo_axi/inst_fifo_gen/gaxi_full_lite.g*_ch.g*ch2.axi_*/grf.rf/rstblk/ngwrdrst.grst.g7serrst.gnsckt_wrst.rst_wr_reg2_inst/arststages_ff_reg*/PRE)"]
