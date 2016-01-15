////////////////////////////////////////////////////////////////////////////////
// Module: housekeeping
// Authors: Matej Oblak, Iztok Jeras <iztok.jeras@redpitaya.com>
// (c) Red Pitaya  (redpitaya.com)
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// GENERAL DESCRIPTION:
//
// House keeping module takes care of system identification.
//
// This module takes care of system identification via DNA readout at startup and
// ID register which user can define at compile time.
//
// Beside that it is currently also used to test expansion connector and for
// driving LEDs.
////////////////////////////////////////////////////////////////////////////////

module red_pitaya_hk #(
  // GPIO
  int unsigned DWL = 8, // data width for LED
  int unsigned DWE = 8, // data width for extension
  // ID
  bit [57-1:0] DNA = 57'h0823456789ABCDE
)(
  // LED
  output logic [DWL-1:0] led_o,  // LED output
  // global configuration
  output logic           digital_loop,
  // expansion connector
  input  logic [DWE-1:0] exp_p_i ,  // input
  output logic [DWE-1:0] exp_p_o ,  // output
  output logic [DWE-1:0] exp_p_oe,  // output enable
  input  logic [DWE-1:0] exp_n_i ,  //
  output logic [DWE-1:0] exp_n_o ,  //
  output logic [DWE-1:0] exp_n_oe,  //
  // system bus
  sys_bus_if.s           bus
);

////////////////////////////////////////////////////////////////////////////////
//  Read device DNA
////////////////////////////////////////////////////////////////////////////////

logic          dna_dout ;
logic          dna_clk  ;
logic          dna_read ;
logic          dna_shift;
logic [ 9-1:0] dna_cnt  ;
logic [57-1:0] dna_value;
logic          dna_done ;

always_ff @(posedge bus.clk)
if (!bus.rstn) begin
  dna_clk   <= '0;
  dna_read  <= '0;
  dna_shift <= '0;
  dna_cnt   <= '0;
  dna_value <= '0;
  dna_done  <= '0;
end else begin
  if (!dna_done)
    dna_cnt <= dna_cnt + 1'd1;

  dna_clk <= dna_cnt[2] ;
  dna_read  <= (dna_cnt < 9'd10);
  dna_shift <= (dna_cnt > 9'd18);

  if ((dna_cnt[2:0]==3'h0) && !dna_done)
    dna_value <= {dna_value[57-2:0], dna_dout};

  if (dna_cnt > 9'd465)
    dna_done <= 1'b1;
end

// parameter specifies a sample 57-bit DNA value for simulation
DNA_PORT #(.SIM_DNA_VALUE (DNA)) i_DNA (
  .DOUT  (dna_dout ), // 1-bit output: DNA output data.
  .CLK   (dna_clk  ), // 1-bit input: Clock input.
  .DIN   (1'b0     ), // 1-bit input: User data input pin.
  .READ  (dna_read ), // 1-bit input: Active high load DNA, active low read input.
  .SHIFT (dna_shift)  // 1-bit input: Active high shift enable input.
);

////////////////////////////////////////////////////////////////////////////////
// Desing identification
////////////////////////////////////////////////////////////////////////////////

logic [32-1:0] id_value;

assign id_value[31:4] = 28'h0; // reserved
assign id_value[ 3:0] =  4'h1; // board type   1 - release 1

////////////////////////////////////////////////////////////////////////////////
//  System bus connection
////////////////////////////////////////////////////////////////////////////////

localparam int unsigned BDW = 6;

always_ff @(posedge bus.clk)
if (!bus.rstn) begin
  digital_loop <= '0;
  // LED
  led_o    <= '0;
  // GPIO
  exp_p_o  <= '0;
  exp_p_oe <= '0;
  exp_n_o  <= '0;
  exp_n_oe <= '0;
end else if (bus.wen) begin
  if (bus.addr[BDW-1:0]=='h0c)   digital_loop <= bus.wdata[0];
  // GPIO
  if (bus.addr[BDW-1:0]=='h10)   exp_p_oe <= bus.wdata[DWE-1:0];
  if (bus.addr[BDW-1:0]=='h14)   exp_n_oe <= bus.wdata[DWE-1:0];
  if (bus.addr[BDW-1:0]=='h18)   exp_p_o  <= bus.wdata[DWE-1:0];
  if (bus.addr[BDW-1:0]=='h1C)   exp_n_o  <= bus.wdata[DWE-1:0];
  // LED
  if (bus.addr[BDW-1:0]=='h30)   led_o    <= bus.wdata[DWL-1:0];
end

always_ff @(posedge bus.clk)
if (!bus.rstn)  bus.err <= 1'b1;
else            bus.err <= 1'b0;

logic sys_en;
assign sys_en = bus.wen | bus.ren;

always_ff @(posedge bus.clk)
if (!bus.rstn) begin
  bus.ack <= 1'b0;
end else begin
  bus.ack <= sys_en;
  casez (bus.addr[BDW-1:0])
    // ID
    'h00:  bus.rdata <= {                id_value          };
    'h04:  bus.rdata <= {                dna_value[32-1: 0]};
    'h08:  bus.rdata <= {{64- 57{1'b0}}, dna_value[57-1:32]};
    'h0c:  bus.rdata <= {{32-  1{1'b0}}, digital_loop      };
    // GPIO
    'h10:  bus.rdata <= {{32-DWE{1'b0}}, exp_p_oe};
    'h14:  bus.rdata <= {{32-DWE{1'b0}}, exp_n_oe};
    'h18:  bus.rdata <= {{32-DWE{1'b0}}, exp_p_o} ;
    'h1C:  bus.rdata <= {{32-DWE{1'b0}}, exp_n_o} ;
    'h20:  bus.rdata <= {{32-DWE{1'b0}}, exp_p_i} ;
    'h24:  bus.rdata <= {{32-DWE{1'b0}}, exp_n_i} ;
    // LED
    'h30:  bus.rdata <= {{32-DWL{1'b0}}, led_o}   ;

    default: bus.rdata <= '0;
  endcase
end

endmodule: red_pitaya_hk
