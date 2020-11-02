`timescale 1ns / 1ps

module adc_bfm
  #(parameter DATA_BITS = 14)(
  input  wire                 clk,
  output reg  [DATA_BITS-1:0] data
);

reg [15:0] rom [0:1022];
integer addr;

initial 
begin
  addr = 0;
  $readmemh("sine.data", rom);
end


always @(posedge clk)
begin
  if (addr == 1022) begin
    addr <= 0;
  end else begin
    addr <= addr + 1;
  end
  
  data <= rom[addr];
end

endmodule
