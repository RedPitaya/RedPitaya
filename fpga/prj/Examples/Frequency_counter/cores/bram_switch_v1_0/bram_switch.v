`timescale 1 ns / 1 ps


// BRAM PORTA input
// BRAM PORTB input
// BRAM PORTC output


module bram_switch #
(
  parameter integer BRAM_DATA_WIDTH = 32,
  parameter integer BRAM_ADDR_WIDTH = 15  // 2^11 = 2048 positions
)
(
  // System signals
  input  wire                        switch,
  

  // BRAM PORT A
  input  wire                        bram_porta_clk,
  input  wire                        bram_porta_rst,
  input  wire [BRAM_ADDR_WIDTH-1:0]  bram_porta_addr,
  input  wire [BRAM_DATA_WIDTH-1:0]  bram_porta_wrdata,
  output wire [BRAM_DATA_WIDTH-1:0]  bram_porta_rddata,
  input  wire                        bram_porta_we,
  
  // BRAM PORT B
  input  wire                        bram_portb_clk,
  input  wire                        bram_portb_rst,
  input  wire [BRAM_ADDR_WIDTH-1:0]  bram_portb_addr,
  input  wire [BRAM_DATA_WIDTH-1:0]  bram_portb_wrdata,
  output wire [BRAM_DATA_WIDTH-1:0]  bram_portb_rddata,
  input  wire                        bram_portb_we,
  
  
  // BRAM PORT C
  output wire                        bram_portc_clk,
  output wire                        bram_portc_rst,
  output wire [BRAM_ADDR_WIDTH-1:0]  bram_portc_addr,
  output wire [BRAM_DATA_WIDTH-1:0]  bram_portc_wrdata,
  input  wire [BRAM_DATA_WIDTH-1:0]  bram_portc_rddata,
  output wire                        bram_portc_we
  
);

 // reg int_clk;

//assign bram_portc_clk = int_clk;
//assign bram_portc_clk = switch == 1 ? bram_porta_clk : bram_portb_clk;
//assign int_clk = switch == 1 ? bram_porta_clk : bram_portb_clk;
assign bram_portc_rst = switch == 1 ? bram_porta_rst : bram_portb_rst;
assign bram_portc_addr = switch == 1 ? bram_porta_addr : bram_portb_addr;
assign bram_portc_wrdata = switch == 1 ? bram_porta_wrdata : bram_portb_wrdata;
assign bram_portc_we = switch == 1 ? bram_porta_we: bram_portb_we;

assign bram_porta_rddata = switch == 1 ? bram_portc_rddata : {(BRAM_DATA_WIDTH){1'b0}};
assign bram_portb_rddata = switch == 0 ? bram_portc_rddata : {(BRAM_DATA_WIDTH){1'b0}};


// BUFGCTRL  bugmux  (.O(bram_portc_clk),  
                  // .CE0(~switch),  
                  // .CE1(switch),  
                  // .I0(bram_portb_clk),  
                  // .I1(bram_porta_clk),  
                  // .IGNORE0(1'b0),  
                  // .IGNORE1(1'b0),  
                  // .S0(1'b1), // Clock select0 input  
                  // .S1(1'b1)  // Clock select1 input  
// );


BUFGMUX #(
   )
   BUFGMUX_inst (
      .O(bram_portc_clk),   // 1-bit output: Clock output
      .I0(bram_portb_clk), // 1-bit input: Clock input (S=0)
      .I1(bram_porta_clk), // 1-bit input: Clock input (S=1)
      .S(switch)    // 1-bit input: Clock select
   );

//(.INIT_OUT(0),  
//                  .PRESELECT_I0(FALSE),  
//                  .PRESELECT_I1(FALSE)
//               )

//BUFG adc_clk_inst (.I(int_clk), .O(bram_portc_clk));

endmodule
