`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: Anton Potocnik
// 
// Create Date: 05.10.2016 00:09:16
// Design Name: 
// Module Name: knight_rider
// Project Name: 
// Target Devices: xc7z010clg400-1
// Tool Versions: 
// Description: 
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////


module knight_rider(
    input clk,
    output [7:0] led_out
    );
    
    parameter LEDS_INIT = 10'b1100000000;
    parameter DIR_INIT = 1;
    
    reg [9:0] leds = LEDS_INIT; // register for led output
    reg [3:0] position = DIR_INIT*8; // state counter 0->15 
    reg direction = DIR_INIT;   // direction indicator

    
    always @ (posedge clk) begin
        if (direction == 0) begin 
            leds <= leds << 1;  // bit-shift leds register
        end else begin
            leds <= leds >> 1;  // bit-shift leds register
        end
        position <= position + 1;
    end

    always @ (position) begin       // change direction 
        if (position < 8) begin     // in the second half 
            direction = 0;
        end else begin
            direction = 1;
        end
    end

    assign led_out = leds[8:1]; // wire output and leds register
    
endmodule
