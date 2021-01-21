////////////////////////////////////////////////////////////////////////////////
// Red Pitaya equalization filter
// Authors: Matej Oblak, Iztok Jeras
// (c) Red Pitaya  http://www.redpitaya.com
//
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// GENERAL DESCRIPTION:
// Filter to equalize input analog chain. 
//
//  s14  -----  s23  -------  s15  -------  s15  -------  s40  -----  s14
// ---->| FIR |---->| IIR 1 |---->| IIR 2 |---->| SCALE |---->| SAT |---->
//       -----       -------       -------       -------       -----
//         ^            ^             ^             ^
//         | s25        | s18         | s25         | s25
//         |            |             |             |
//         BB           AA            PP            KK
//
////////////////////////////////////////////////////////////////////////////////

module scope_filter #(
  int unsigned DN = 1, // TODO: functionality not yet enabled
  type DTI = logic signed [16-1:0],
  type DTO = logic signed [16-1:0]
)(
  input wire clk,
  input wire rst_n,
  // configuration
  input  logic signed [ 18-1:0] cfg_coeff_aa,   // config AA coefficient
  input  logic signed [ 25-1:0] cfg_coeff_bb,   // config BB coefficient
  input  logic signed [ 25-1:0] cfg_coeff_kk,   // config KK coefficient
  input  logic signed [ 25-1:0] cfg_coeff_pp,   // config PP coefficient
  // control
  //input  logic                  ctl_rst,  // synchronous reset
  // Slave AXI-S
  input  wire [13:0]  s_axis_tdata,
  input  wire         s_axis_tvalid,
  output wire         s_axis_tready,
  
  output reg  [13:0]  m_axis_tdata,
  output wire         m_axis_tvalid,
  input  wire         m_axis_tready
  
  // streams
  //axi4_stream_if.d              sti,      // input
  //axi4_stream_if.s              sto       // output
);

////////////////////////////////////////////////////////////////////////////////
// local signals
////////////////////////////////////////////////////////////////////////////////

//axi4_stream_if #(.DN (DN), .DT (logic signed [23-1:0])) fir0 (.ACLK (clk), .ARESETn (rst_n));
//axi4_stream_if #(.DN (DN), .DT (logic signed [23-1:0])) iir1 (.ACLK (clk), .ARESETn (rst_n));
//axi4_stream_if #(.DN (DN), .DT (logic signed [15-1:0])) iir2 (.ACLK (clk), .ARESETn (rst_n));

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

logic signed [25+$bits(DTI)-1:0] fir_mul [0:1];
logic signed [22:0] fir0;
logic signed [22:0] iir1;
logic signed [13:0] iir2;

//axi4_stream_if #(.DN (DN), .DT (fir_mul_t [2-1:0])) firm (.ACLK (clk), .ARESETn (rst_n));

// FIR filter
fir_buf_t [2-1:0] fir_buf;  // data buffer
fir_cfg_t [2-1:0] fir_cfg;  // coeficients

assign s_axis_tready = 1;
// FIR data buffer
assign fir_buf [0] = s_axis_tdata;
//assign fir_buf [0] = sti.TDATA;

always @(posedge clk)
begin
  if (~rst_n) begin
    fir_buf[1] <= '0;
  end else begin
    if ((s_axis_tvalid == 1) && (s_axis_tready == 1)) begin  
      fir_buf[1] <= fir_buf[0];
    end
  end
end

// FIR coeficients
assign fir_cfg[0] =  (1 <<< (18+10));
assign fir_cfg[1] = -(1 <<< (18+10)) + cfg_coeff_bb;

// multiplications
generate
  for (genvar i=0; i<2; i++) begin: for_fir_mul
    always @(posedge clk)
    begin
      //if ((s_axis_tvalid == 1) && (s_axis_tready == 1)) begin  
        fir_mul[i] <= (fir_buf[i] * fir_cfg[i]) >>> 10;
        //firm.TDATA[0][i] <= (fir_buf[i] * fir_cfg[i]) >>> 10;
      //end
    end
  end: for_fir_mul
endgenerate

// final summation
always @(posedge clk) 
begin
//if (firm.transf)  fir0.TDATA <= (firm.TDATA[0][1] + firm.TDATA[0][0]) >>> 10;
  fir0 <= (fir_mul[1] + fir_mul[0]) >>> 10;
end

// control signals
//always @(posedge clk)
//if (~rst_n) begin
//  firm.TVALID <= 1'b0;
//  fir0.TVALID <= 1'b0;
//end else begin
//  if (sti.TREADY)  firm.TVALID <= sti.TVALID;
//  if (firm.TREADY) fir0.TVALID <= firm.TVALID;
//end

//assign sti.TREADY  = firm.TREADY | ~firm.TVALID;
//assign firm.TREADY = fir0.TREADY | ~fir0.TVALID;

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

//// data processing
always @(posedge clk)
begin
  if (~rst_n) begin
    iir1 <= '0;
  end else begin
  //if (fir0.transf)  iir1.TDATA <= ((fir0.TDATA <<< 25) + (iir1.TDATA * ((1 <<< 25) - cfg_aa))) >>> 25;
    iir1 <= ((fir0 <<< 25) + (iir1 * ((1 <<< 25) - cfg_coeff_aa))) >>> 25;
  end
end

//// control signals
//always @(posedge clk)
//if (~rst_n)      iir1.TVALID <= 1'b0;
//else if (fir0.TREADY)  iir1.TVALID <= fir0.TVALID;

//assign fir0.TREADY = iir1.TREADY | ~iir1.TVALID;

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

//always @(posedge clk)
//if (~rst_n)      iir2.TDATA <= '0;
//else if (iir1.transf)  iir2.TDATA <= (iir1.TDATA >>> 8) + ((iir2.TDATA * cfg_pp) >>> 16);

always @(posedge clk)
begin
  if (~rst_n) begin
    iir2 <= '0;
  end else begin
  //if (fir0.transf)  iir1.TDATA <= ((fir0.TDATA <<< 25) + (iir1.TDATA * ((1 <<< 25) - cfg_aa))) >>> 25;
    iir2 <= (iir1 >>> 8) + ((iir2 * cfg_coeff_pp) >>> 16);
  end
end

//// control signals
//always @(posedge clk)
//if (~rst_n)      iir2.TVALID <= 1'b0;
//else if (iir1.TREADY)  iir2.TVALID <= iir1.TVALID;

//assign iir1.TREADY = iir2.TREADY | ~iir2.TVALID;

////////////////////////////////////////////////////////////////////////////////
// Scaling and saturation
////////////////////////////////////////////////////////////////////////////////

logic signed [40-1:0] kk_mult;

assign kk_mult = ($signed(iir2) * $signed(cfg_coeff_kk)) >>> 24;

// saturation
//always @(posedge clk)
//if (~rst_n)      sto.TDATA <= '0;
//else if (iir2.transf)  sto.TDATA <= kk_mult[40-1:40-2] ? {kk_mult[40-1], {$bits(DTO)-1{~kk_mult[40-1]}}} : kk_mult[$bits(DTO)-1:0];

always @(posedge clk)
begin
  if (~rst_n) begin
    m_axis_tdata <= '0;
  end else begin
  //if (fir0.transf)  iir1.TDATA <= ((fir0.TDATA <<< 25) + (iir1.TDATA * ((1 <<< 25) - cfg_aa))) >>> 25;
    m_axis_tdata <= kk_mult[40-1:40-2] ? {kk_mult[40-1], {$bits(DTO)-1{~kk_mult[40-1]}}} : kk_mult[$bits(DTO)-1:0];
  end
end

//assign sto.TLAST = 1'b0;
//assign sto.TKEEP = '1;

//// control signals
//always @(posedge clk)
//if (~rst_n)      sto.TVALID <= 1'b0;
//else if (iir2.TREADY)  sto.TVALID <= iir2.TVALID;

//assign iir2.TREADY = sto.TREADY | ~sto.TVALID;

endmodule: scope_filter
