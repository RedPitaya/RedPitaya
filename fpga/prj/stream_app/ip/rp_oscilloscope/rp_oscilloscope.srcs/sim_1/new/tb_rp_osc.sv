`timescale 1ns / 1ps

module tb_rp_osc;

localparam S_AXI_ADDR_BITS = 12;
localparam TRIG_SRC_NUM     = 7;
localparam EVENT_SRC_NUM    = 7;
localparam ADC_BITS         = 16;

reg                             clk_125;
reg                             rst_n;
wire                            s_axi_reg_aclk;
reg  [S_AXI_ADDR_BITS-1:0]      s_axi_reg_awaddr;
reg                             s_axi_reg_awvalid;
wire                            s_axi_reg_awready;
reg  [31:0]                     s_axi_reg_wdata;
reg                             s_axi_reg_wvalid;
wire                            s_axi_reg_wready;
reg                             s_axi_reg_bready;
wire                            s_axi_reg_bvalid;
reg  [S_AXI_ADDR_BITS-1:0]      s_axi_reg_araddr;
reg                             s_axi_reg_arvalid;
wire                            s_axi_reg_arready;
wire                            s_axi_reg_rvalid;
reg                             s_axi_reg_rready;

wire [EVENT_SRC_NUM-1:0]        event_trig;                
wire [EVENT_SRC_NUM-1:0]        event_stop;                
wire [EVENT_SRC_NUM-1:0]        event_start;               
wire [EVENT_SRC_NUM-1:0]        event_reset;               
wire [TRIG_SRC_NUM-1:0]         trig;                      
                              
wire [3:0]                      osc1_event;                                                              
//wire                            osc1_event_trig;          
//wire                            osc1_event_stop;           
//wire                            osc1_event_start;          
//wire                            osc1_event_reset;          
wire                            osc1_trig;                 

wire [3:0]                      osc2_event;
//wire                            osc2_event_trig;          
//wire                            osc2_event_stop;           
//wire                            osc2_event_start;          
//wire                            osc2_event_reset;          
wire                            osc2_trig;   
int sine_dat;
reg  [ADC_BITS-1:0] sine_mem [0:628];

reg  [ADC_BITS-1:0] adc_data_ch1;
wire signed [15:0] dat;

int file_sine_wr;
int file_sine_rd;
integer status;
integer j;   
        
assign event_trig   = {3'd0, osc2_event[0], osc1_event[0], 2'd0};
assign event_stop   = {3'd0, osc2_event[1], osc1_event[1], 2'd0};
assign event_start  = {3'd0, osc2_event[2], osc1_event[2], 2'd0};
assign event_reset  = {3'd0, osc2_event[3], osc1_event[3], 2'd0};
assign trig         = {3'd0, osc2_trig, osc1_trig, 2'd0};
                                                                          
initial
begin
  clk_125 = 1;
  forever begin
    #(8.0/2.0) clk_125 = ~clk_125; 
  end
end

assign s_axi_reg_aclk = clk_125;

initial
begin
  rst_n = 0;
  
  #1000;
  @(posedge clk_125)
  rst_n = 1;  
end

rp_oscilloscope #(
  .EVENT_SRC_NUM        (EVENT_SRC_NUM),
  .TRIG_SRC_NUM         (TRIG_SRC_NUM),
  .ADC_DATA_BITS        (ADC_BITS))
  U_rp_osc(
  .clk                  (clk_125),                
  .rst_n                (rst_n),                        
  .intr                 (),          
  .adc_data_ch1         (adc_data_ch1),
  .adc_data_ch2         (),      
  .event_ip_trig        (event_trig),         
  .event_ip_stop        (event_stop),         
  .event_ip_start       (event_start),        
  .event_ip_reset       (event_reset),        
  .trig_ip              (trig),       
  .osc1_event_op        (osc1_event),                                
//  .osc1_event_op_trig   (osc1_event_trig),    
//  .osc1_event_op_stop   (osc1_event_stop),    
//  .osc1_event_op_start  (osc1_event_start),   
//  .osc1_event_op_reset  (osc1_event_reset),   
  .osc1_trig_op         (osc1_trig),          
  .osc2_event_op        (osc2_event),                                   
//  .osc2_event_op_trig   (osc2_event_trig),    
//  .osc2_event_op_stop   (osc2_event_stop),    
//  .osc2_event_op_start  (osc2_event_start),   
//  .osc2_event_op_reset  (osc2_event_reset),   
  .osc2_trig_op         (osc2_trig),                                        
  .s_axi_reg_aclk       (s_axi_reg_aclk),         
  .s_axi_reg_aresetn    (rst_n),      
  .s_axi_reg_awaddr     (s_axi_reg_awaddr),       
  .s_axi_reg_awprot     (),       
  .s_axi_reg_awvalid    (s_axi_reg_awvalid),      
  .s_axi_reg_awready    (s_axi_reg_awready),      
  .s_axi_reg_wdata      (s_axi_reg_wdata),        
  .s_axi_reg_wstrb      (4'hf),        
  .s_axi_reg_wvalid     (s_axi_reg_wvalid),       
  .s_axi_reg_wready     (s_axi_reg_wready),       
  .s_axi_reg_bresp      (),        
  .s_axi_reg_bvalid     (s_axi_reg_bvalid),       
  .s_axi_reg_bready     (s_axi_reg_bready),       
  .s_axi_reg_araddr     (s_axi_reg_araddr),       
  .s_axi_reg_arprot     (),       
  .s_axi_reg_arvalid    (s_axi_reg_arvalid),      
  .s_axi_reg_arready    (s_axi_reg_arready),      
  .s_axi_reg_rdata      (),        
  .s_axi_reg_rresp      (),        
  .s_axi_reg_rvalid     (s_axi_reg_rvalid),       
  .s_axi_reg_rready     (s_axi_reg_rready),     
  .m_axi_osc1_aclk      (clk_125),
  .m_axi_osc1_aresetn   (rst_n),     
  .m_axi_osc1_awaddr    (),      
  .m_axi_osc1_awlen     (),       
  .m_axi_osc1_awsize    (),      
  .m_axi_osc1_awburst   (),     
  .m_axi_osc1_awprot    (),      
  .m_axi_osc1_awcache   (),     
  .m_axi_osc1_awvalid   (),     
  .m_axi_osc1_awready   (1'b1),     
  .m_axi_osc1_wdata     (),       
  .m_axi_osc1_wstrb     (),       
  .m_axi_osc1_wlast     (),       
  .m_axi_osc1_wvalid    (),      
  .m_axi_osc1_wready    (1'b1),      
  .m_axi_osc1_bresp     (),       
  .m_axi_osc1_bvalid    (1'b1),      
  .m_axi_osc1_bready    (),          
  .m_axi_osc2_aclk      (clk_125),
  .m_axi_osc2_aresetn   (rst_n),                
  .m_axi_osc2_awaddr    (),      
  .m_axi_osc2_awlen     (),       
  .m_axi_osc2_awsize    (),      
  .m_axi_osc2_awburst   (),     
  .m_axi_osc2_awprot    (),      
  .m_axi_osc2_awcache   (),     
  .m_axi_osc2_awvalid   (),     
  .m_axi_osc2_awready   (1'b1),     
  .m_axi_osc2_wdata     (),       
  .m_axi_osc2_wstrb     (),       
  .m_axi_osc2_wlast     (),       
  .m_axi_osc2_wvalid    (),      
  .m_axi_osc2_wready    (1'b1),      
  .m_axi_osc2_bresp     (),       
  .m_axi_osc2_bvalid    (1'b1),      
  .m_axi_osc2_bready    ());     
  
always @(posedge clk_125)
begin
  adc_data_ch1 <= sine_mem[j];  
end  

always @(posedge clk_125)
begin
  if (rst_n == 0) begin
    j <= 0;
  end else begin
    if (j == 628) begin
      j <= 0;
    end else begin
      j <= j + 1;
    end
  end
end  

always @(posedge clk_125)
begin
  if (rst_n == 1) begin
    $fwrite(file_sine_wr, "%d\n", $signed(dat));
  end
end  

assign dat = U_rp_osc.s_axis_osc1_tdata;

initial
begin
  s_axi_reg_awaddr  = 0;
  s_axi_reg_awvalid = 0;
  s_axi_reg_wdata   = 0;
  s_axi_reg_wvalid  = 0;
  s_axi_reg_araddr  = 0;
  s_axi_reg_arvalid = 0;
  s_axi_reg_rready  = 0;

  file_sine_wr = $fopen("sine_16bit_op.txt", "w");    
  file_sine_rd = $fopen("sine_16bit_ip.txt", "r");  
  
  j = 0;
  while(!($feof(file_sine_rd))) begin
    status = $fscanf(file_sine_rd, "%d\n", sine_dat);
    sine_mem[j] = sine_dat;
    j = j+1;
  end
  
  $fclose(file_sine_rd);    
  
  wait (rst_n == 1)
  
  #6us;
  $fclose(file_sine_wr);     
  
  // OSC 0
  axi_reg_write(96, 32'h00000400);  // DMA buf size  
  axi_reg_write(88, 32'hc0000000);  // DMA dst addr 1
  axi_reg_write(92, 32'hd0000000);  // DMA dst addr 2
  axi_reg_write(80, 32'h00000211);  // DMA control reg
//  axi_reg_write(60, 32'h00000000);  // filter bypass 
//  axi_reg_write(64, 32'h00000001);  // filter coeff aa
//  axi_reg_write(68, 32'h00000001);  // filter coeff bb
//  axi_reg_write(72, 32'h00000001);  // filter coeff kk
//  axi_reg_write(76, 32'h00000001);  // filter coeff pp        
  axi_reg_write(4,  32'h00000002);  // select OSC 0 event
  //axi_reg_write(8,  32'h00000004);  // trigger mask
//  axi_reg_write(32,  -4);           // Trigger low level
//  axi_reg_write(36,  4);            // Trigger high level
//  axi_reg_write(40,  0);            // Trigger edge
//  axi_reg_write(16,  256);          // Trigger pre samples      
//  axi_reg_write(20,  256);          // Trigger post samples
//  axi_reg_write(96,  0);            // Calib offset  
//  axi_reg_write(100, 1);            // Calib gain  
  axi_reg_write(0,  32'h00000001);  // Control rst
  axi_reg_write(0,  32'h00000002);  // Control start   
  
  //#20us;

  
  wait(U_rp_osc.intr == 1);
  axi_reg_write(80, 32'h00000002);  // DMA control reg  
  #200ns;
  axi_reg_write(80, 32'h00000004);  // DMA control reg  
//  #1us;
//  axi_reg_write(0,  32'h00000004);  // Control stop  

//  #100000;
//  // OSC 1
//  axi_reg_write(256+88, 32'h00000400);  // DMA buf size  
//  axi_reg_write(256+84, 32'hc0000000);  // DMA dst addr
//  axi_reg_write(256+80, 32'h00000001);  // DMA control reg
//  axi_reg_write(256+60, 32'h00000001);  // bypass filter
//  axi_reg_write(256+4,  32'h00000003);  // select OSC 1 event
//  axi_reg_write(256+8,  32'h00000004);  // trigger mask
//  axi_reg_write(256+32,  -4);           // Trigger low level
//  axi_reg_write(256+36,  4);            // Trigger high level
//  axi_reg_write(256+40,  0);            // Trigger edge
//  axi_reg_write(256+16,  128);          // Trigger pre samples      
//  axi_reg_write(256+20,  256);          // Trigger post samples
//  axi_reg_write(256+92,  0);            // Calib offset  
//  axi_reg_write(256+96,  1);            // Calib gain    
//  axi_reg_write(256+0,  32'h00000001);  // Control rst
//  axi_reg_write(256+0,  32'h00000002);  // Control start     
  
//  wait(U_rp_osc.intr == 1);
//  axi_reg_write(256+80, 32'h00000002);  // DMA control reg  
end
  
task axi_reg_write;
  input [S_AXI_ADDR_BITS-1:0] addr;
  input [31:0] data;
begin
  @(posedge s_axi_reg_aclk)
  s_axi_reg_awaddr   = addr;
  s_axi_reg_awvalid  = 1;  
  s_axi_reg_wvalid   = 1;
  s_axi_reg_wdata    = data;
    
  wait (s_axi_reg_awready == 1);
  
  @(posedge s_axi_reg_aclk)
  s_axi_reg_awaddr   = 0;
  s_axi_reg_awvalid  = 0;
  s_axi_reg_wvalid   = 0;

  @(posedge s_axi_reg_aclk)  
  
  s_axi_reg_bready   = 1;
  
  wait (s_axi_reg_bvalid == 1);  
  
  @(posedge s_axi_reg_aclk)  
  s_axi_reg_bready   = 0;    
end    
endtask

task axi_reg_read;
  input [S_AXI_ADDR_BITS-1:0] addr;
begin
  @(posedge s_axi_reg_aclk)
  s_axi_reg_araddr   = addr;
  s_axi_reg_arvalid  = 1;  
  s_axi_reg_rready   = 1;
  
  wait (s_axi_reg_arready == 1);
  
  @(posedge s_axi_reg_aclk)
  s_axi_reg_araddr   = 0;
  s_axi_reg_arvalid  = 0;
end    
endtask

endmodule
