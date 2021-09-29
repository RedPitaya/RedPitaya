set_property SOURCE_SET sources_1 [get_filesets sim_1]
add_files -fileset sim_1 -norecurse {/home/juretrnovec/RPdev/RP30/redpitaya-public/fpga/prj/v0.94/tbn/red_pitaya_top_sim.sv /home/juretrnovec/RPdev/RP30/redpitaya-public/fpga/prj/v0.94/tbn/top_tb.sv /home/juretrnovec/RPdev/RP30/redpitaya-public/fpga/prj/v0.94/tbn/top_tc.sv /home/juretrnovec/RPdev/RP30/redpitaya-public/fpga/prj/v0.94/tbn/red_pitaya_ps_sim.sv /home/juretrnovec/RPdev/RP30/redpitaya-public/fpga/tbn/axi4_sync.sv}
update_compile_order -fileset sim_1
set_property verilog_define {SIMULATION} [get_filesets sim_1]
set_property USED_IN_SIMULATION 0 [get_files /home/juretrnovec/RPdev/RP30/redpitaya-public/fpga/prj/v0.94/project/redpitaya.srcs/sources_1/imports/fpga/prj/v0.94/rtl/red_pitaya_top.sv]
set_property USED_IN_SIMULATION 0 [get_files /home/juretrnovec/RPdev/RP30/redpitaya-public/fpga/prj/v0.94/project/redpitaya.srcs/sources_1/imports/fpga/prj/v0.94/rtl/red_pitaya_ps.sv]
set_property top top_tb [get_filesets sim_1]
set_property top_lib xil_defaultlib [get_filesets sim_1]
set_property SOURCE_SET sources_1 [get_filesets sim_1]
add_files -fileset sim_1 -norecurse /home/juretrnovec/RPdev/RP30/redpitaya-public/fpga/tbn/axi_bus_model.sv

create_bd_port -dir I rstn_in
delete_bd_objs [get_bd_nets processing_system7_0_fclk_reset3_n]
connect_bd_net [get_bd_ports rstn_in] [get_bd_pins proc_sys_reset/ext_reset_in]
connect_bd_net [get_bd_ports FCLK_RESET3_N] [get_bd_pins processing_system7/FCLK_RESET3_N]
delete_bd_objs [get_bd_intf_nets Vp_Vn_1] [get_bd_intf_nets Vaux1_1] [get_bd_intf_nets Vaux8_1] [get_bd_intf_nets Vaux9_1] [get_bd_nets proc_sys_reset_0_peripheral_aresetn] [get_bd_nets xadc_ip2intc_irpt] [get_bd_intf_nets axi_protocol_converter_0_M_AXI] [get_bd_intf_nets Vaux0_1] [get_bd_cells xadc]
delete_bd_objs [get_bd_intf_nets processing_system7_0_M_AXI_GP1] [get_bd_cells axi_protocol_converter_0]
update_compile_order -fileset sim_1
create_bd_port -dir O -type rst rstn_out
connect_bd_net [get_bd_ports rstn_out] [get_bd_pins proc_sys_reset/interconnect_aresetn]

create_bd_cell -type ip -vlnv xilinx.com:ip:clk_wiz:6.0 clk_wiz_0
set_property -dict [list CONFIG.CLKOUT2_USED {true} CONFIG.CLKOUT1_REQUESTED_OUT_FREQ {62.500} CONFIG.CLKOUT2_REQUESTED_OUT_FREQ {125.000} CONFIG.USE_LOCKED {false} CONFIG.USE_RESET {false} CONFIG.MMCM_CLKOUT1_DIVIDE {8} CONFIG.NUM_OUT_CLKS {2} CONFIG.CLKOUT2_JITTER {119.348} CONFIG.CLKOUT2_PHASE_ERROR {96.948}] [get_bd_cells clk_wiz_0]

delete_bd_objs [get_bd_nets processing_system7_0_fclk_clk3]
connect_bd_net [get_bd_ports FCLK_CLK3] [get_bd_pins processing_system7/FCLK_CLK3]
create_bd_port -dir O -type clk clkout_125
connect_bd_net [get_bd_ports clkout_125] [get_bd_pins clk_wiz_0/clk_out2]
delete_bd_objs [get_bd_nets m_axi_gp0_aclk_1]
delete_bd_objs [get_bd_nets processing_system7_0_fclk_clk0]
connect_bd_net [get_bd_ports FCLK_CLK0] [get_bd_pins processing_system7/FCLK_CLK0]
create_bd_port -dir I -type clk -freq_hz 125000000 clk_in
connect_bd_net [get_bd_ports clk_in] [get_bd_pins clk_wiz_0/clk_in1]
connect_bd_net [get_bd_pins proc_sys_reset/slowest_sync_clk] [get_bd_pins clk_wiz_0/clk_out2]




delete_bd_objs [get_bd_intf_nets processing_system7_M_AXI_GP0]
delete_bd_objs [get_bd_intf_ports M_AXI_GP0]
set_property -dict [list CONFIG.PCW_USE_M_AXI_GP0 {0} CONFIG.PCW_USE_M_AXI_GP1 {0}] [get_bd_cells processing_system7]

launch_simulation