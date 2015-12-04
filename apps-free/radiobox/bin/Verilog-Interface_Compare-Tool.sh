#!/bin/sh

# This tool is written by Ulrich Habel (DF4IAH) for comparing two verilog interface cut-out files with each other:
# an interface definition on one side and the block instantiation on the other side. See the short example:
#
# file_1.v:
#  aclk,
#  aresetn,
#  s_axi_awid,
#  s_axi_awaddr,
#    ...
#  m_axi_rready 
#
#
# file_2.v:
#  .aclk(processing_system7_0_fclk_clk0),
#  .aresetn(proc_sys_reset_0_interconnect_aresetn),
#  .m_axi_araddr(axi_protocol_converter_0_M_AXI_ARADDR),
#  .m_axi_arready(axi_protocol_converter_0_M_AXI_ARREADY),
#    ...
#  .s_axi_wvalid(processing_system7_0_M_AXI_GP1_WVALID));
#
#


if [[ $# != 2 ]]; then
  echo
  echo "USAGE: $0  verilog_interface.txt  verilog_instantiation.txt"
  echo
else
  INFILE_1="$1"
  INFILE_2="$2"

  cat $INFILE_1 | sort | sed -E -e 's/[ \t\.]*([^,\(]+).*$/\1/g' > tmp_$INFILE_1
  cat $INFILE_2 | sort | sed -E -e 's/[ \t\.]*([^,\(]+).*$/\1/g' > tmp_$INFILE_2

  diff -y tmp_$INFILE_1 tmp_$INFILE_2 

  rm tmp_$INFILE_1 tmp_$INFILE_2
fi

