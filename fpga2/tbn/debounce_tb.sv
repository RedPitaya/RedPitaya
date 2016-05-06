`timescale 1ns / 1ps

module debounce_tb;

// number of debounced signals
localparam DW = 2;

// counter length (max debounce time / clock period)
localparam realtime CP = 1us / 2;//125; // Mhz
localparam int      CN = 10ms / CP; 
localparam int      CW = $clog2(int'(CN)); 

// list of local signals
logic          clk ;  // clock
logic          rstn;  // reset (active low)
logic [DW-1:0] d_i;   // debounce inptu
logic [DW-1:0] d_o;   // debounce output
logic [DW-1:0] d_p;   // debounce output posedge
logic [DW-1:0] d_n;   // debounce output negedge

int seed;  // random seed

// request for a dumpfile
initial begin
  $dumpfile("debounce_tb.vcd");
  $dumpvars(0, debounce_tb);
end

// clock generation
initial        clk = 1'b1;
always #(CP/2) clk = ~clk;

// reset generation
initial begin
  rstn = 1'b0;
  repeat (4) @ (posedge clk);
  rstn = 1'b1;
end

// initialize random seed
initial seed = 0;

generate
for (genvar d=0; d<DW; d=d+1) begin

  // test signal generation
  // keypress timing is asynchronous
  initial begin
    // stable OFF before start for 50ms
    d_i[d] = 1'b0;
    # 20ms;
    // switch ON random pulses (max 10ms)
    bounce (d, 1'b1, 30, 5000, 10_000, 100_000, 80);
    // stable ON state for 50ms
    d_i[d] = 1'b1;
    # 20ms;
    // switch OFF random pulses (max 10ms)
    bounce (d, 1'b0, 30, 5000, 10_000, 100_000, 80);
    // stable OFF state at the end for 50ms
    d_i[d] = 1'b0;
    # 20ms;
    // end simulation
    $finish();
  end

end
endgenerate

task automatic bounce (
  input int unsigned d,
  input logic        val,
  input realtime     t_pulse_min, t_pulse_max,
  input realtime     t_pause_min, t_pause_max,
  input int unsigned n
);
begin
  int unsigned t;
  for (int unsigned cnt=0; cnt<n; cnt=cnt+1) begin
    // short pulses
    d_i[d] =  val;  t = $dist_uniform(seed, t_pulse_min, t_pulse_max);  #t;
    // folowed by longer pauses
    d_i[d] = ~val;  t = $dist_uniform(seed, t_pause_min, t_pause_max);  #t;
  end
end
endtask

// instantiate RTL DUT
debounce #(
  .CW   (CW)
) debounce [DW-1:0] (
  // system signals
  .clk  (clk ),
  .rstn (rstn),
  // configuration and control
  .ena  (1'b1),
  .len  (CN[CW-1:0]),
  .d_i  (d_i),
  .d_o  (d_o),
  .d_p  (d_p),
  .d_n  (d_n)
);

endmodule: debounce_tb
