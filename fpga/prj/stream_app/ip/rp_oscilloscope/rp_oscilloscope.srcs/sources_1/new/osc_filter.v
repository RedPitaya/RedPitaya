`timescale 1ns / 1ps

module osc_filter(
  input  wire         clk,  
  input  wire         rst_n,    
  // Slave AXI-S
  input  wire [13:0]  s_axis_tdata,
  input  wire         s_axis_tvalid,
  output wire         s_axis_tready,
  // Master AXI-
  output wire [13:0]  m_axis_tdata,
  output wire         m_axis_tvalid,
  input  wire         m_axis_tready,
  // Confi
  input  wire         cfg_bypass,
  input  wire [17:0]  cfg_coeff_aa, 
  input  wire [24:0]  cfg_coeff_bb, 
  input  wire [24:0]  cfg_coeff_kk, 
  input  wire [24:0]  cfg_coeff_pp  
);

////////////////////////////////////////////////////////////////////////////////
// local signals
////////////////////////////////////////////////////////////////////////////////

//axi4_stream_if #(.DN (DN), .DT (logic signed [23-1:0])) fir0 (.ACLK (sti.ACLK), .ARESETn (sti.ARESETn));
//axi4_stream_if #(.DN (DN), .DT (logic signed [23-1:0])) iir1 (.ACLK (sti.ACLK), .ARESETn (sti.ARESETn));
//axi4_stream_if #(.DN (DN), .DT (logic signed [15-1:0])) iir2 (.ACLK (sti.ACLK), .ARESETn (sti.ARESETn));

////////////////////////////////////////////////////////////////////////////////
//
// FIR
//
// Time domain:
//
// y[n] = ( (             2**8) * x[n-0] ) +
//          (bb / 2**20 - 2**8) * x[n-1] )
//
// Frequency domain:
//
// y[n] = ( (           - 2**8) *        ) +
//          (bb / 2**20 - 2**8) * z^(-1) )
//
////////////////////////////////////////////////////////////////////////////////

typedef logic signed [   $bits(DTI)-1:0] fir_buf_t;  // data buffer
typedef logic signed [30           -1:0] fir_cfg_t;  // coeficients
typedef logic signed [25+$bits(DTI)-1:0] fir_mul_t;  // multiplications

axi4_stream_if #(.DN (DN), .DT (fir_mul_t [2-1:0])) firm (.ACLK (sti.ACLK), .ARESETn (sti.ARESETn));

// FIR filter
fir_buf_t [2-1:0] fir_buf;  // data buffer
fir_cfg_t [2-1:0] fir_cfg;  // coeficients

// FIR data buffer
assign fir_buf [0] = sti.TDATA;

always @(posedge clk)
if (rst_n == 0)     fir_buf [1] <= '0;
else if (sti.transf)  fir_buf [1] <= fir_buf [0];

// FIR coeficients
assign fir_cfg [0] =  (1 <<< (18+10))         ;
assign fir_cfg [1] = -(1 <<< (18+10)) + cfg_bb;

// multiplications
generate
for (genvar i=0; i<2; i++) begin: for_fir_mul
  always @(posedge clk)
  if (sti.transf) firm.TDATA[0][i] <= (fir_buf[i] * fir_cfg[i]) >>> 10;
end: for_fir_mul
endgenerate

// final summation
always @(posedge clk)
if (firm.transf)  fir0.TDATA <= (firm.TDATA[0][1] + firm.TDATA[0][0]) >>> 10;

// control signals
always @(posedge clk)
if (rst_n == 0) begin
  firm.TVALID <= 1'b0;
  fir0.TVALID <= 1'b0;
end else begin
  if (sti.TREADY)  firm.TVALID <= sti.TVALID;
  if (firm.TREADY) fir0.TVALID <= firm.TVALID;
end

assign sti.TREADY  = firm.TREADY | ~firm.TVALID;
assign firm.TREADY = fir0.TREADY | ~fir0.TVALID;

////////////////////////////////////////////////////////////////////////////////
//
// IIR 1
//
// Time domain:
//
// y[n] = ( (2**25     ) * x[n-0] +
//          (2**25 - aa) * y[n-1] ) / 2**25
//
// Frequency domain:
//
// TODO
//
////////////////////////////////////////////////////////////////////////////////

// data processing
always @(posedge clk)
if (rst_n == 0)      iir1.TDATA <= '0;
else if (fir0.transf)  iir1.TDATA <= ((fir0.TDATA <<< 25) + (iir1.TDATA * ((1 <<< 25) - cfg_aa))) >>> 25;

// control signals
always @(posedge clk)
if (rst_n == 0)      iir1.TVALID <= 1'b0;
else if (fir0.TREADY)  iir1.TVALID <= fir0.TVALID;

assign fir0.TREADY = iir1.TREADY | ~iir1.TVALID;

////////////////////////////////////////////////////////////////////////////////
//
// IIR 2
//
// Time domain:
//
// y[n] = (      x[n-0] / (2**8 ) +
//          pp * y[n-1] / (2**16) )
//
// Frequency domain:
//
// TODO
//
////////////////////////////////////////////////////////////////////////////////

always @(posedge clk)
if (rst_n == 0)      iir2.TDATA <= '0;
else if (iir1.transf)  iir2.TDATA <= (iir1.TDATA >>> 8) + ((iir2.TDATA * cfg_pp) >>> 16);

// control signals
always @(posedge clk)
if (rst_n == 0)      iir2.TVALID <= 1'b0;
else if (iir1.TREADY)  iir2.TVALID <= iir1.TVALID;

assign iir1.TREADY = iir2.TREADY | ~iir2.TVALID;

////////////////////////////////////////////////////////////////////////////////
// Scaling and saturation
////////////////////////////////////////////////////////////////////////////////

logic signed [40-1:0] kk_mult;

assign kk_mult = ($signed(iir2.TDATA) * $signed(cfg_kk)) >>> 24;

// saturation
always @(posedge clk)
if (rst_n == 0)      sto.TDATA <= '0;
else if (iir2.transf)  sto.TDATA <= kk_mult[40-1:40-2] ? {kk_mult[40-1], {$bits(DTO)-1{~kk_mult[40-1]}}} : kk_mult[$bits(DTO)-1:0];

assign sto.TLAST = 1'b0;
assign sto.TKEEP = '1;

// control signals
always @(posedge clk)
if (rst_n == 0)      sto.TVALID <= 1'b0;
else if (iir2.TREADY)  sto.TVALID <= iir2.TVALID;

assign iir2.TREADY = sto.TREADY | ~sto.TVALID;





//wire signed [13:0]  din; 
//reg         [13:0]  tdata_pipe [0:3];
//reg         [3:0]   tvalid_pipe;
//wire signed [17:0]  coeff_aa;
//wire signed [24:0]  coeff_bb;
//wire signed [24:0]  coeff_kk;
//wire signed [24:0]  coeff_pp;

//wire signed [38:0]  bb_mult;
//wire signed [32:0]  r2_sum;
//reg  signed [32:0]  r1_reg;
//reg  signed [22:0]  r2_reg;
//reg  signed [31:0]  r01_reg;
//reg  signed [27:0]  r02_reg;

//wire signed [40:0]  aa_mult;
//wire signed [48:0]  r3_sum; 
//reg  signed [22:0]  r3_reg_dsp1;
//reg  signed [22:0]  r3_reg_dsp2;
//reg  signed [22:0]  r3_reg_dsp3;

//wire signed [39:0]  pp_mult;
//wire signed [15:0]  r4_sum;
//reg  signed [14:0]  r4_reg;
//reg  signed [14:0]  r3_shr;

//wire signed [39:0]  kk_mult;
//reg  signed [13:0]  r5_reg;

//assign din            = s_axis_tdata;
//assign s_axis_tready  = 1;
//assign coeff_aa       = cfg_coeff_aa;
//assign coeff_bb       = cfg_coeff_bb;
//assign coeff_kk       = cfg_coeff_kk;
//assign coeff_pp       = cfg_coeff_pp;
//assign m_axis_tdata   = (cfg_bypass == 0) ? r5_reg : tdata_pipe[3];
//assign m_axis_tvalid  = tvalid_pipe[3];

//assign bb_mult = din * coeff_bb;
//assign r2_sum  = r01_reg + r1_reg;

//always @(posedge clk)
//begin
//  r1_reg  <= r02_reg - r01_reg;
//  r2_reg  <= r2_sum >>> 10;
//  r01_reg <= din <<< 18;
//  r02_reg <= bb_mult >>> 10;
//end

////---------------------------------------------------------------------------------
////  IIR 1

//assign aa_mult = r3_reg_dsp1 * coeff_aa;
//assign r3_sum  = (r2_reg <<< 25) + (r3_reg_dsp2 <<< 25) - aa_mult;

//always @(posedge clk)
//begin
//  if (rst_n == 0) begin
//    r3_reg_dsp1 <= 0;
//    r3_reg_dsp2 <= 0;
//    r3_reg_dsp3 <= 0;          
//  end else begin
//    r3_reg_dsp1 <= r3_sum >>> 25;
//    r3_reg_dsp2 <= r3_sum >>> 25;
//    r3_reg_dsp3 <= r3_sum >>> 33;
//  end
//end

////---------------------------------------------------------------------------------
////  IIR 2

//assign pp_mult = r4_reg * coeff_pp;
//assign r4_sum  = r3_shr + (pp_mult >>> 16);

//always @(posedge clk)
//begin
//  if (rst_n == 0) begin
//    r3_shr <= 0;   
//    r4_reg <= 0;     
//  end else begin
//    r3_shr <= r3_reg_dsp3;
//    r4_reg <= r4_sum;
//  end    
//end

////---------------------------------------------------------------------------------
////  Scaling

//assign kk_mult = r4_reg * coeff_kk;

//always @(posedge clk)
//begin
//  if ((kk_mult >>> 24) > $signed(14'h1FFF)) begin
//    r5_reg <= 14'h1FFF;
//  end else begin
//    if ((kk_mult >>> 24) < $signed(14'h2000)) begin  
//      r5_reg <= 14'h2000;
//    end else  begin
//      r5_reg <= kk_mult >>> 24;
//    end
//  end
//end

//always @(posedge clk)
//begin
//  tdata_pipe[0] <= din;
//  tdata_pipe[1] <= tdata_pipe[0];
//  tdata_pipe[2] <= tdata_pipe[1];  
//  tdata_pipe[3] <= tdata_pipe[2];   
//end

//always @(posedge clk)
//begin
//  tvalid_pipe[0] <= s_axis_tvalid;
//  tvalid_pipe[1] <= tvalid_pipe[0];
//  tvalid_pipe[2] <= tvalid_pipe[1];
//  tvalid_pipe[3] <= tvalid_pipe[2];    
//end

endmodule