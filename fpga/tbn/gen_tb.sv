////////////////////////////////////////////////////////////////////////////////
// Module: Red Pitaya arbitrary signal generator testbench.
// Authors: Matej Oblak, Iztok Jeras
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

`timescale 1ns / 1ps

module gen_tb #(
  // time period
  realtime  TP = 8.0ns,  // 125MHz
  // types
  type U16 = logic [16-1:0],
  type S16 = logic signed [16-1:0],
  type S14 = logic signed [14-1:0],
  // data parameters
  type DT = S14,
  type DTM = S16,
  type DTS = S14,
  // buffer parameters
  int unsigned CWM = 14,  // counter width magnitude (fixed point integer)
  int unsigned CWF = 16   // counter width fraction  (fixed point fraction)
);

////////////////////////////////////////////////////////////////////////////////
// signal generation
////////////////////////////////////////////////////////////////////////////////

// system signals
logic clk ;
logic rstn;

// interrupt
logic irq;

// stream
axi4_stream_if #(.DN (1), .DT (DT)) str (.ACLK (clk), .ARESETn (rstn));

// events input/output
evn_pkg::evn_t evn;
// triggers input/output
logic trg;

////////////////////////////////////////////////////////////////////////////////
// clock
////////////////////////////////////////////////////////////////////////////////

initial        clk = 1'b0;
always #(TP/2) clk = ~clk;

// clocking 
default clocking cb @ (posedge clk);
  input  rstn;
endclocking: cb

// DAC reset
initial begin
  rstn <= 1'b0;
  ##4;
  rstn <= 1'b1;
end

// ADC cycle counter
int unsigned dac_cyc=0;
always_ff @ (posedge clk)
dac_cyc <= dac_cyc+1;

////////////////////////////////////////////////////////////////////////////////
// test sequence
////////////////////////////////////////////////////////////////////////////////

logic        [ 32-1: 0] rdata;
logic signed [ 32-1: 0] rdata_blk [];

////////////////////////////////////////////////////////////////////////////////
// signal generation
////////////////////////////////////////////////////////////////////////////////

localparam int CTL_RST = 4'b0001;
localparam int CTL_STR = 4'b0010;
localparam int CTL_STP = 4'b0100;
localparam int CTL_TRG = 4'b1000;

//int buf_len = 2**CWM;
int buf_len = 8;
real freq  = 10_000; // 10kHz
real phase = 0; // DEG

initial begin
  ##10;
  // write table
  for (int i=0; i<buf_len; i++) begin
    busm_tbl.write((i*4), i);  // write table
  end
  // read table
  rdata_blk = new [80];
  for (int i=0; i<buf_len; i++) begin
    busm_tbl.read((i*4), rdata_blk [i]);  // read table
  end
  // configure amplitude and DC offset
  busm.write('h40, 1 << ($bits(DTM)-2));  // amplitude
  busm.write('h44, 0);                    // DC offset
  busm.write('h48, 1);                    // enable output

  // events
  busm.write('h04, 0);      // software event select
  busm.write('h08, 2'b00);  // hardware trigger mask

  // enable continuous/periodic mode
  busm.write('h10, 2'b00);  // burst disable
  // configure frequency and phase
  busm.write('h14,  buf_len                    * 2**CWF - 1);  // table size
  busm.write('h18, (buf_len * (phase/360.0)  ) * 2**CWF    );  // offset
//busm.write('h1c, (buf_len * (freq*TP/10**6)) * 2**CWF - 1);  // step
  busm.write('h1c, 1                           * 2**CWF - 1);  // step
  // start/trigger
  busm.write('h00, CTL_STR);
  busm.write('h00, CTL_TRG);
  ##22;
  // stop
  busm.write('h00, CTL_STP);
  ##20;
  // reset
  busm.write('h00, CTL_RST);
  ##20;

  // enable burst mode
  busm.write('h10, 2'b01);  // burst enable
  // burst configuration
  busm.write('h20, 2-1);  // burst data   repetitions
  busm.write('h24, 3-1);  // burst data   length
  busm.write('h28, 8-1);  // burst period length
  busm.write('h2c, 2-1);  // burst period number
  // start/trigger
  busm.write('h00, CTL_STR);
  busm.write('h00, CTL_TRG);
  // wait for burst end, so there is no need to stop
  ##120;
  // reset
  busm.write('h00, CTL_RST);
  ##20;

  // end simulation
  ##20;
  $stop();
  //$finish();
end

////////////////////////////////////////////////////////////////////////////////
// module instances
////////////////////////////////////////////////////////////////////////////////

sys_bus_if bus     (.clk (clk), .rstn (rstn));
sys_bus_if bus_tbl (.clk (clk), .rstn (rstn));

sys_bus_model busm     (.bus (bus    ));
sys_bus_model busm_tbl (.bus (bus_tbl));

gen #(
  .DT  (DT),
  .DTM (DTM),
  .DTS (DTS),
  .EN  (1),
  .TN  ($bits(trg))
) gen (
  // stream output
  .sto      (str),
  // events input/output
  .evi      (evn),
  .evo      (evn),
  // triggers input/output
  .trg      (trg),
  .tro      (trg),
  // interrupt
  .irq      (irq),
  // system bus
  .bus      (bus),
  .bus_tbl  (bus_tbl)
);

// stream drain
assign str.TREADY = 1'b1;

////////////////////////////////////////////////////////////////////////////////
// waveforms
////////////////////////////////////////////////////////////////////////////////

initial begin
  $dumpfile("gen_tb.vcd");
  $dumpvars(0, gen_tb);
end

endmodule: gen_tb
