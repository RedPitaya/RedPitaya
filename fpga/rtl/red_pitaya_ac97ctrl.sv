/**
 * $Id: red_pitaya_ac97.v 001 2016-04-12 18:55:00Z DF4IAH $
 *
 * @brief Red Pitaya AC97 adapter, used to import and export AC97 audio streams
 * for connection to and from the Red Pitaya FPGA.
 *
 * @Author Ulrich Habel, DF4IAH
 *
 * (c) Ulrich Habel / GitHub.com open source  https://github.com/DF4IAH/RedPitaya_RadioBox/
 *
 * This part of code is written in Verilog hardware description language (HDL).
 * Please visit http://en.wikipedia.org/wiki/Verilog
 * for more details on the language used herein.
 */

/**
 * GENERAL DESCRIPTION:
 *
 * The AC97 streams are connected to internal register nodes. Any input node can be used
 * as an additional audio/signal source, any output node can be used as an additional
 * audio/signal sink clocked with 48 kHz. This gives the Linux kernel a good chance to
 * attach an ALSA driver to this FPGA configuration.
 *
 * This module emulates the register map behaviour seen by the CPUs of the XILINX ML403 board,
 * which includes a Texas Instruments ML4550 AC97-Codec. The ML403 has an IP available called
 * "OPB AC97 Sound Controller" (opb_ac97_controller_ref) to interface the FPGA to the AC97-Codec.
 *
 * This module is a re-implementation of such a AC97-Controller with the interface described in
 * UG082, chapter 6 in detail.
 *
 * On the Linux kernel side a ALSA sound driver implements the interfacing of the ALSA sound
 * system to this FPGA configuration register map. Due to already working driver implementation
 * the module  (linux-xlnx)/sound/drivers/ml403-ac97cr.c  is selected to to this job.
 */


`timescale 1ns / 1ps

module red_pitaya_ac97ctrl #(
  parameter C_OPB_AWIDTH             = 32,      // silently ignored
  parameter C_OPB_DWIDTH             = 32,      // register width
  parameter C_BASEADDR       = 'h40700000,      // silently ignored - corresponds to red_pitaya_top address map connection
  parameter C_HIGHADDR       = 'h407FFFFF,      // silently ignored - corresponds to red_pitaya_top address map connection
  parameter C_PLAYBACK               =  1,      // 0 = no playback operation, 1 = playback operation active
  parameter C_RECORD                 =  1,      // 0 = no record operation,   1 = record operation active
  parameter C_PLAY_INTR_LEVEL        =  2,      // 0 = No Interrupt, 1 = empty Num Words = 0, 2 = halfempty Num Words <= 7, 3 = halffull Num Words >= 8, 4 = full Num Words = 16
  parameter C_REC_INTR_LEVEL         =  3       // 0 = No Interrupt, 1 = empty Num Words = 0, 2 = halfempty Num Words <= 7, 3 = halffull Num Words >= 8, 4 = full Num Words = 16
)(
   // ADC clock & reset
   input                 clk_adc_125mhz  ,      // ADC based clock, 125 MHz
   input                 adc_rstn_i      ,      // ADC reset - active low
   output       [  1: 0] ac97_clks_o     ,      // audio/sound/signal sample frequencies

   // AC97 lines
   output    [ 2*16-1:0] ac97_line_out_o ,      // AC97 line output nodes - [15:0] = LEFT, [31:16] = RIGHT
   input     [ 2*16-1:0] ac97_line_in_i  ,      // AC97 line input nodes  - [15:0] = LEFT, [31:16] = RIGHT

   // Interrupts
   output reg            ac97_irq_play_o ,      // IRQ line signaling play   FIFO interrupt - high active
   output reg            ac97_irq_rec_o  ,      // IRQ line signaling record FIFO interrupt - high active

   // DEBUGGING LEDs
   output       [  7: 0] ac97_leds_o     ,      // DEBUGGING: diagnose LEDs

   // System bus - slave
   input        [ 31: 0] sys_addr        ,      // bus saddress
   input        [ 31: 0] sys_wdata       ,      // bus write data
   input        [  3: 0] sys_sel         ,      // bus write byte select
   input                 sys_wen         ,      // bus write enable
   input                 sys_ren         ,      // bus read enable
   output reg   [ 31: 0] sys_rdata       ,      // bus read data
   output reg            sys_err         ,      // bus error indicator
   output reg            sys_ack                // bus acknowledge signal
);


//---------------------------------------------------------------------------------
//  Registers accessed by the system bus

enum {
    REG_WO_AC97CTRL_FIFO_PLAY              = 20'h000, // AC97CTRL play   FIFO, use LSB 16 bits
    REG_RO_AC97CTRL_FIFO_REC               = 20'h004, // AC97CTRL record FIFO, use LSB 16 bits
    REG_RO_AC97CTRL_STAT                   = 20'h008, // AC97CTRL status register, @see REG_AC97CTRL_STAT_ENUMS
    REG_WO_AC97CTRL_FIFO_RESET             = 20'h00C, // AC97CTRL FIFO reset register, @see REG_AC97CTRL_FIFO_RESET_ENUMS
    REG_WO_AC97CTRL_CODEC_ADDR             = 20'h010, // AC97CTRL CODEC address register, @see REG_AC97CTRL_CTRL_ADDR_REG_ENUMS
    REG_RO_AC97CTRL_CODEC_DATA_READ        = 20'h014, // AC97CTRL CODEC data read register, use LSB 8 bits
    REG_WO_AC97CTRL_CODEC_DATA_WRITE       = 20'h018, // AC97CTRL CODEC data write register, use LSB 8 bits

    REG_AC97CTRL_COUNT
} REG_AC97CTRL_ENUMS;

enum {
    BIT_AC97CTRL_STAT_FIFO_PLAY_FULL       =       0, // 0 = Playback FIFO not Full,                                                              1 = Playback FIFO Full
    BIT_AC97CTRL_STAT_FIFO_PLAY_HALFFULL            , // 0 = Playback FIFO not Half Full,                                                         1 = Playback FIFO Half Full
    BIT_AC97CTRL_STAT_FIFO_REC_FULL                 , // 0 = Record FIFO not Full,                                                                1 = Record FIFO Full
    BIT_AC97CTRL_STAT_FIFO_REC_EMPTY                , // 0 = Record FIFO not Empty,                                                               1 = Record FIFO Empty
    BIT_AC97CTRL_STAT_REG_ACCESS_FINISHED           , // 0 = AC97 Controller waiting for access to control/status register in Codec to complete.  1 = AC97 Controller is finished accessing the control/status register in Codec.
    BIT_AC97CTRL_STAT_CODEC_READY                   , // 0 = Codec is not ready to receive commands or data.                                      1 = Codec ready to run
    BIT_AC97CTRL_STAT_FIFO_PLAY_UNDERRUN            , // 0 = FIFO has not underrun                                                                1 = FIFO has underrun
    BIT_AC97CTRL_STAT_FIFO_REC_UNDERRUN               // 0 = FIFO has not underrun                                                                1 = FIFO has underrun
} REG_AC97CTRL_STAT_ENUMS;

enum {
    BIT_AC97CTRL_FIFO_RESET_PLAY           =       0, // 0 = Do not Reset Play FIFO                                                               1 = Reset Play FIFO. Resetting the Play FIFO also clears the "Play FIFO Underrun" status bit.
    BIT_AC97CTRL_FIFO_RESET_REC                       // 0 = Do not Reset Record FIFO                                                             1 = Reset Record FIFO. Resetting the record FIFO also clears the "Record FIFO Overrun" status bit.
} REG_AC97CTRL_FIFO_RESET_ENUMS;

localparam MASK_AC97CTRL_CTRL_ADDR_REG     =    'h07; // Sets the 7-bit address of control or status register in the Codec chip to be accessed. Writing to this register clears the "Register Access Finish" status bit.
enum {
    BIT_AC97CTRL_CTRL_ADDR_RW              =       7  // 0 = Perform a write to the AC97 address register.                                      1 = Performs a read to the AC97 address register.
} REG_AC97CTRL_CTRL_ADDR_REG_ENUMS;


// === OMNI section ===

localparam C_FIFO_SIZE                          = 16;

reg           ac97ctrl_fifo_play_reset          = 1'b0;
reg           ac97ctrl_fifo_rec_reset           = 1'b0;
reg           ac97ctrl_codec_ready              = 1'b0;
reg           ac97ctrl_access_ready             = 1'b0;

wire          ac97ctrl_play_fifo_empty;
reg           ac97ctrl_play_fifo_empty_d        = 1'b0;
wire          ac97ctrl_play_fifo_he;
reg           ac97ctrl_play_fifo_he_d           = 1'b0;
wire          ac97ctrl_play_fifo_hf;
reg           ac97ctrl_play_fifo_hf_d           = 1'b0;
wire          ac97ctrl_play_fifo_full;
reg           ac97ctrl_play_fifo_full_d         = 1'b0;
reg           ac97ctrl_play_fifo_underrun       = 1'b0;

wire          ac97ctrl_rec_fifo_empty;
reg           ac97ctrl_rec_fifo_empty_d         = 1'b0;
wire          ac97ctrl_rec_fifo_he;
reg           ac97ctrl_rec_fifo_he_d            = 1'b0;
wire          ac97ctrl_rec_fifo_hf;
reg           ac97ctrl_rec_fifo_hf_d            = 1'b0;
wire          ac97ctrl_rec_fifo_full;
reg           ac97ctrl_rec_fifo_full_d          = 1'b0;
reg           ac97ctrl_rec_fifo_overrun         = 1'b0;

reg           ac97ctrl_reset_delay              = 1'b0;
reg  unsigned [2:0] ac97ctrl_reset_delay_ctr    =  'b0;

always @(posedge clk_adc_125mhz)                                                                            // assign ac97ctrl_reset_delay, play reset does a pre-fill of the play FIFO
if (!adc_rstn_i || ac97ctrl_fifo_play_reset) begin
   ac97ctrl_reset_delay     <= 1'b1;
   ac97ctrl_reset_delay_ctr <= 3'b111;
   end
else if (!ac97ctrl_reset_delay_ctr)
   ac97ctrl_reset_delay     <= 1'b0;
else
   ac97ctrl_reset_delay_ctr = ac97ctrl_reset_delay_ctr - 1;

always @(posedge clk_adc_125mhz)                                                                            // assign ac97ctrl_codec_ready
if (!adc_rstn_i || ac97ctrl_reset_delay)
   ac97ctrl_codec_ready <= 1'b0;
else if (!ac97ctrl_reset_delay)
   ac97ctrl_codec_ready <= 1'b1;


//---------------------------------------------------------------------------------
//  CLK_48KHZ and CLK_8KHZ generation

localparam CLK_48KHZ_CTR_MAX = 2604;                                                                        // long run max value
localparam CLK_48KHZ_FRC_MAX = 5;

reg  [ 11: 0] clk_48khz_ctr  = 'b0;
reg  [  2: 0] clk_48khz_frc  = 'b0;
reg           clk_48khz_r    = 'b0;
reg           clk_8khz_r     = 'b0;

always @(posedge clk_adc_125mhz)                                                                            // assign clk_48khz, clk_8khz
if (!adc_rstn_i) begin
   clk_48khz_ctr <= 'b0;
   clk_48khz_frc <= 'b0;
   clk_48khz_r <= 'b0;
   clk_8khz_r  <= 'b0;
   end
else
   if (clk_48khz_ctr == CLK_48KHZ_CTR_MAX) begin
      clk_48khz_r <= 1'b1;
      if (clk_48khz_frc == CLK_48KHZ_FRC_MAX) begin
         clk_48khz_frc <= 1'b0;
         clk_48khz_ctr <= 1'b0;                                                                             // overflow of the frac part makes a long run
         clk_8khz_r <= 1'b1;
         end
      else begin
         clk_48khz_frc <= clk_48khz_frc + 1;
         clk_48khz_ctr <= 12'b1;                                                                            // short run
         end
      end
   else begin
      clk_8khz_r  <= 1'b0;
      clk_48khz_r <= 1'b0;
      clk_48khz_ctr <= clk_48khz_ctr + 1;
      end

// drive clocks
BUFG bufg_ac97_48khz_clk ( .O (clk_8khz  ), .I ( clk_8khz_r  ) );
BUFG bufg_ac97_8khz_clk  ( .O (clk_48khz ), .I ( clk_48khz_r ) );

assign ac97_clks_o = { clk_48khz, clk_8khz };                                                               // ascending order


//---------------------------------------------------------------------------------
// AC97-CODEC registers

reg  [  1: 0] ac97ctrl_codec_pwrdn          =  'b0;
reg  [  5: 0] ac97ctrl_codec_addr           =  'b0;                                                         // word offset address
reg  [ 15: 0] ac97ctrl_codec_data_write     =  'b0;
wire [ 15: 0] ac97ctrl_codec_data_read_out;
reg  [ 15: 0] ac97ctrl_codec_data_read      =  'b0;
reg           ac97ctrl_codec_data_store     = 1'b0;
reg           ac97ctrl_codec_data_recall    = 1'b0;
reg  [  1: 0] ac97ctrl_codec_data_recall_d  =  'b0;

ac97ctrl_16x64_nc_blkmem i_ac97ctrl_regs (
  .clka                    ( clk_adc_125mhz              ),  // global 125 MHz clock
  .addra                   ( ac97ctrl_codec_addr[5:0]    ),  // AC97-CODEC word address
  .dina                    ( ac97ctrl_codec_data_write   ),  // AC97-CODEC content data word
  .wea                     ( ac97ctrl_codec_data_store   ),  // store data word

  .clkb                    ( clk_adc_125mhz              ),  // global 125 MHz clock
  .addrb                   ( ac97ctrl_codec_addr[5:0]    ),  // AC97-CODEC word address
  .doutb                   ( ac97ctrl_codec_data_read_out)   // AC97-CODEC content data word
);

always @(posedge clk_adc_125mhz)                                                                            // assign ac97ctrl_codec_data_read
if (!adc_rstn_i)
   ac97ctrl_codec_pwrdn <= 'b0;
else if (ac97ctrl_codec_addr[5:0] == (6'h26 >> 1))                                                          // [1]: 1 = play disabled, [0]: 1 = capture disabled
   ac97ctrl_codec_pwrdn[1:0] <= { (ac97ctrl_codec_data_write[9] || ac97ctrl_codec_data_write[12] || ac97ctrl_codec_data_write[13])  ,
                                  (ac97ctrl_codec_data_write[8] || ac97ctrl_codec_data_write[12] || ac97ctrl_codec_data_write[13]) };

always @(posedge clk_adc_125mhz)                                                                            // assign ac97ctrl_codec_data_read
if (!adc_rstn_i)
   ac97ctrl_codec_data_read <= 'b0;
else if (ac97ctrl_codec_data_recall_d[1])
   if (ac97ctrl_codec_addr[5:0] == (6'h26 >> 1))
      ac97ctrl_codec_data_read <= ac97ctrl_codec_data_read_out | 16'h000f;                                  // status always being "ready"
   else
      ac97ctrl_codec_data_read <= ac97ctrl_codec_data_read_out;


//---------------------------------------------------------------------------------
//  Play & Record FIFOs

reg  [ 15: 0] ac97ctrl_play_left       =  'b0;
reg  [ 15: 0] ac97ctrl_play_right      =  'b0;
reg           ac97ctrl_play_is_right   = 1'b0;
reg           ac97ctrl_play_is_right_d = 1'b0;
reg           ac97ctrl_play_fifo_push  = 1'b0;

always @(posedge clk_adc_125mhz)                                                                            // assign ac97ctrl_play_fifo_push
if (!adc_rstn_i || ac97ctrl_fifo_play_reset)
   ac97ctrl_play_fifo_push <= 1'b0;
else if (ac97ctrl_reset_delay)
   ac97ctrl_play_fifo_push <= 1'b1;
else
   ac97ctrl_play_fifo_push <= (!ac97ctrl_play_is_right && ac97ctrl_play_is_right_d) ?  1'b1 : 1'b0;         // toggled right --> left


wire          ac97ctrl_play_fifo_reset = !adc_rstn_i || ac97ctrl_fifo_play_reset;
wire [ 31: 0] ac97ctrl_play_fifo_write = ac97ctrl_reset_delay ?  32'b0 : { ac97ctrl_play_right, ac97ctrl_play_left };
wire [ 31: 0] ac97ctrl_play_fifo_read;
wire          ac97ctrl_play_fifo_pop   = ac97ctrl_codec_pwrdn[1] ?  1'b0 : clk_48khz;
wire [  3: 0] ac97ctrl_play_fifo_ctr;

ac97ctrl_16x32_sr_fifo i_ac97ctrl_play_fifo (
  // global signals
  .clk                     ( clk_adc_125mhz              ),  // global 125 MHz clock
  .srst                    ( ac97ctrl_play_fifo_reset    ),  // play FIFO reset

  .din                     ( ac97ctrl_play_fifo_write    ),  // 16 bit play sample data to be pushed into the FIFO
  .wr_en                   ( ac97ctrl_play_fifo_push     ),  // push new data

  .dout                    ( ac97ctrl_play_fifo_read     ),  // 16 bit play sample data to be poped from the FIFO
  .rd_en                   ( ac97ctrl_play_fifo_pop      ),  // pop data

  .data_count              ( ac97ctrl_play_fifo_ctr      )   // content counter
);

assign ac97_line_out_o[ 2*16-1:0]  = ac97ctrl_play_fifo_read[31:0];


wire          ac97ctrl_rec_fifo_reset = !adc_rstn_i || ac97ctrl_fifo_rec_reset;
reg           ac97ctrl_rec_is_right   = 1'b0;
wire [ 31: 0] ac97ctrl_rec_fifo_write = ac97_line_in_i[2*16-1:0];
wire          ac97ctrl_rec_fifo_push  = ac97ctrl_codec_pwrdn[0] ?  1'b0 : clk_48khz;
wire [ 31: 0] ac97ctrl_rec_fifo_read;
reg           ac97ctrl_rec_fifo_pop   = 1'b0;
wire [  3: 0] ac97ctrl_rec_fifo_ctr;

ac97ctrl_16x32_sr_fifo i_ac97ctrl_rec_fifo (
  // global signals
  .clk                     ( clk_adc_125mhz              ),  // global 125 MHz clock
  .srst                    ( ac97ctrl_rec_fifo_reset     ),  // record FIFO reset

  .din                     ( ac97ctrl_rec_fifo_write     ),  // 16 bit play sample data to be pushed into the FIFO
  .wr_en                   ( ac97ctrl_rec_fifo_push      ),  // push new data

  .dout                    ( ac97ctrl_rec_fifo_read      ),  // 16 bit play sample data to be poped from the FIFO
  .rd_en                   ( ac97ctrl_rec_fifo_pop       ),  // pop data

  .data_count              ( ac97ctrl_rec_fifo_ctr       )   // content counter
);

assign ac97ctrl_play_fifo_empty   = (!ac97ctrl_play_fifo_ctr)                       ?  1'b1 : 1'b0;
assign ac97ctrl_play_fifo_he      = ( ac97ctrl_play_fifo_ctr <  (C_FIFO_SIZE >> 1)) ?  1'b1 : 1'b0;
assign ac97ctrl_play_fifo_hf      = ( ac97ctrl_play_fifo_ctr >= (C_FIFO_SIZE >> 1)) ?  1'b1 : 1'b0;
assign ac97ctrl_play_fifo_full    = ( ac97ctrl_play_fifo_ctr ==  C_FIFO_SIZE -  1 ) ?  1'b1 : 1'b0;

always @(posedge clk_adc_125mhz)                                                                            // assign ac97_irq_play_o
if (!adc_rstn_i) begin
   ac97_irq_play_o            <= 1'b0;
   ac97ctrl_play_fifo_empty_d <= 1'b0;
   ac97ctrl_play_fifo_he_d    <= 1'b0;
   ac97ctrl_play_fifo_hf_d    <= 1'b0;
   ac97ctrl_play_fifo_full_d  <= 1'b0;
   end

else if (ac97ctrl_fifo_play_reset)                                                                          // clear immediately when reset
   ac97_irq_play_o <= 1'b0;

else if (C_PLAYBACK) begin
   case (C_PLAY_INTR_LEVEL)                                                                                 // 0 = No Interrupt, 1 = empty Num Words = 0, 2 = halfempty Num Words <= 7, 3 = halffull Num Words >= 8, 4 = full Num Words = 16

   1: begin
      if (      ac97ctrl_play_fifo_empty && !ac97ctrl_play_fifo_empty_d)
         ac97_irq_play_o <= 1'b1;
      else if (!ac97ctrl_play_fifo_empty &&  ac97ctrl_play_fifo_empty_d)
         ac97_irq_play_o <= 1'b0;
      end
   2: begin
      if (      ac97ctrl_play_fifo_he    && !ac97ctrl_play_fifo_he_d   )                                    // <-- default setting, see @ top of file
         ac97_irq_play_o <= 1'b1;
      else if (!ac97ctrl_play_fifo_he    &&  ac97ctrl_play_fifo_he_d   )
         ac97_irq_play_o <= 1'b0;
      end
   3: begin
      if (      ac97ctrl_play_fifo_hf    && !ac97ctrl_play_fifo_hf_d   )
         ac97_irq_play_o <= 1'b1;
      else if (!ac97ctrl_play_fifo_hf    &&  ac97ctrl_play_fifo_hf_d   )
         ac97_irq_play_o <= 1'b0;
      end
   4: begin
      if (      ac97ctrl_play_fifo_full  && !ac97ctrl_play_fifo_full_d )
         ac97_irq_play_o <= 1'b1;
      else if (!ac97ctrl_play_fifo_full  &&  ac97ctrl_play_fifo_full_d )
         ac97_irq_play_o <= 1'b0;
      end

   default:
      ac97_irq_play_o <= 1'b0;

   endcase

   ac97ctrl_play_fifo_empty_d <= ac97ctrl_play_fifo_empty;
   ac97ctrl_play_fifo_he_d <= ac97ctrl_play_fifo_he;
   ac97ctrl_play_fifo_hf_d <= ac97ctrl_play_fifo_hf;
   ac97ctrl_play_fifo_full_d <= ac97ctrl_play_fifo_full;
   end


assign ac97ctrl_rec_fifo_empty    = (!ac97ctrl_rec_fifo_ctr)                        ?  1'b1 : 1'b0;
assign ac97ctrl_rec_fifo_he       = ( ac97ctrl_rec_fifo_ctr  <  (C_FIFO_SIZE >> 1)) ?  1'b1 : 1'b0;
assign ac97ctrl_rec_fifo_hf       = ( ac97ctrl_rec_fifo_ctr  >= (C_FIFO_SIZE >> 1)) ?  1'b1 : 1'b0;
assign ac97ctrl_rec_fifo_full     = ( ac97ctrl_rec_fifo_ctr  ==  C_FIFO_SIZE -  1 ) ?  1'b1 : 1'b0;

always @(posedge clk_adc_125mhz)                                                                            // assign ac97_irq_rec_o
if (!adc_rstn_i) begin
   ac97_irq_rec_o            <= 1'b0;
   ac97ctrl_rec_fifo_empty_d <= 1'b0;
   ac97ctrl_rec_fifo_he_d    <= 1'b0;
   ac97ctrl_rec_fifo_hf_d    <= 1'b0;
   ac97ctrl_rec_fifo_full_d  <= 1'b0;
   end

else if (ac97ctrl_fifo_rec_reset)                                                                           // clear immediately when reset
   ac97_irq_rec_o <= 1'b0;

else if (ac97ctrl_rec_fifo_empty && (C_REC_INTR_LEVEL != 4'h1))                                             // clear when level gets below the limit - all cases but '1'
   ac97_irq_rec_o <= 1'b0;

else if (!ac97ctrl_rec_fifo_empty && (C_REC_INTR_LEVEL == 4'h1))                                            // clear when level gets below the limit - special case '1'
   ac97_irq_rec_o <= 1'b0;

else if (C_RECORD) begin
   case (C_REC_INTR_LEVEL)                                                                                  // 0 = No Interrupt, 1 = empty Num Words = 0, 2 = halfempty Num Words <= 7, 3 = halffull Num Words >= 8, 4 = full Num Words = 16

   1: begin
      if (      ac97ctrl_rec_fifo_empty && !ac97ctrl_rec_fifo_empty_d)
         ac97_irq_rec_o <= 1'b1;
      else if (!ac97ctrl_rec_fifo_empty &&  ac97ctrl_rec_fifo_empty_d)
         ac97_irq_rec_o <= 1'b0;
      end
   2: begin
      if (      ac97ctrl_rec_fifo_he    && !ac97ctrl_rec_fifo_he_d   )
         ac97_irq_rec_o <= 1'b1;
      else if (!ac97ctrl_rec_fifo_he    &&  ac97ctrl_rec_fifo_he_d   )
         ac97_irq_rec_o <= 1'b0;
      end
   3: begin
      if (      ac97ctrl_rec_fifo_hf    && !ac97ctrl_rec_fifo_hf_d   )                                      // <-- default setting, see @ top of file
         ac97_irq_rec_o <= 1'b1;
      else if (!ac97ctrl_rec_fifo_hf    &&  ac97ctrl_rec_fifo_hf_d   )
         ac97_irq_rec_o <= 1'b0;
      end
   4: begin
      if (      ac97ctrl_rec_fifo_full  && !ac97ctrl_rec_fifo_full_d )
         ac97_irq_rec_o <= 1'b1;
      else if (!ac97ctrl_rec_fifo_full  &&  ac97ctrl_rec_fifo_full_d )
         ac97_irq_rec_o <= 1'b0;
      end

   default:
      ac97_irq_rec_o <= 1'b0;

   endcase

   ac97ctrl_rec_fifo_empty_d <= ac97ctrl_rec_fifo_empty;
   ac97ctrl_rec_fifo_he_d <= ac97ctrl_rec_fifo_he;
   ac97ctrl_rec_fifo_hf_d <= ac97ctrl_rec_fifo_hf;
   ac97ctrl_rec_fifo_full_d <= ac97ctrl_rec_fifo_full;
   end


always @(posedge clk_adc_125mhz)                                                                            // assign ac97ctrl_play_fifo_underrun
if (!adc_rstn_i || ac97ctrl_reset_delay || ac97ctrl_fifo_play_reset)
   ac97ctrl_play_fifo_underrun <= 1'b0;
else if (ac97ctrl_play_fifo_empty && clk_48khz)
   ac97ctrl_play_fifo_underrun <= 1'b1;

always @(posedge clk_adc_125mhz)                                                                            // assign ac97ctrl_rec_fifo_overrun
if (!adc_rstn_i || ac97ctrl_reset_delay || ac97ctrl_fifo_rec_reset)
   ac97ctrl_rec_fifo_overrun <= 1'b0;
else if (ac97ctrl_rec_fifo_full && clk_48khz)
   ac97ctrl_rec_fifo_overrun <= 1'b1;


// === Bus handling ===

//---------------------------------------------------------------------------------
//  System bus connection

// bus write access
always @(posedge clk_adc_125mhz)
if (!adc_rstn_i || ac97ctrl_reset_delay) begin
   ac97ctrl_play_left                             <=  'b0;
   ac97ctrl_play_right                            <=  'b0;
   ac97ctrl_play_is_right                         <= 1'b0;
   ac97ctrl_play_is_right_d                       <= 1'b0;
   ac97ctrl_access_ready                          <= 1'b0;
   ac97ctrl_codec_data_write                      <=  'b0;
   ac97ctrl_codec_data_recall                     <= 1'b0;
   ac97ctrl_codec_data_recall_d                   <=  'b0;
   ac97ctrl_codec_data_store                      <= 1'b0;
   ac97ctrl_fifo_play_reset                       <= 1'b0;
   ac97ctrl_fifo_rec_reset                        <= 1'b0;
   end

else begin
   if (ac97ctrl_codec_data_recall_d[1] || ac97ctrl_codec_data_store)
      ac97ctrl_access_ready <= 1'b1;

   ac97ctrl_play_is_right_d                       <= ac97ctrl_play_is_right;
   ac97ctrl_codec_data_recall_d[1:0]              <= { ac97ctrl_codec_data_recall_d[0], ac97ctrl_codec_data_recall };

   ac97ctrl_codec_data_recall                     <= 1'b0;                                                  // this and the next lines are pre-sets and are returning to zero after each clock resulting a pulse
   ac97ctrl_codec_data_store                      <= 1'b0;
   ac97ctrl_fifo_play_reset                       <= 1'b0;
   ac97ctrl_fifo_rec_reset                        <= 1'b0;

   if (sys_wen) begin
      casez (sys_addr[19:0])

      /* FIFO */
      REG_WO_AC97CTRL_FIFO_PLAY: begin
         if (!ac97ctrl_play_is_right)
            ac97ctrl_play_left                    <= sys_wdata[15:0];
         else
            ac97ctrl_play_right                   <= sys_wdata[15:0];
         ac97ctrl_play_is_right                   <= !ac97ctrl_play_is_right;
         end

      /* control */
      REG_WO_AC97CTRL_FIFO_RESET: begin
         if (sys_wdata[0]) begin
            ac97ctrl_fifo_play_reset              <= 1'b1;
            ac97ctrl_play_is_right                <= 1'b0;
            end
         if (sys_wdata[1]) begin
            ac97ctrl_fifo_rec_reset               <= 1'b1;
            // ac97ctrl_rec_is_right  is set in the read access section
            end
         end
      REG_WO_AC97CTRL_CODEC_ADDR: begin
         ac97ctrl_codec_addr[5:0]                 <= sys_wdata[ 6:1];
         ac97ctrl_access_ready                    <= 1'b0;
         if (sys_wdata[7])
            ac97ctrl_codec_data_recall            <= 1'b1;                                                  // pulse to read  AC97 register data from one of the AC97 registers
         else
            ac97ctrl_codec_data_store             <= 1'b1;                                                  // pulse to write AC97 register data to   one of the AC97 registers
         end
      REG_WO_AC97CTRL_CODEC_DATA_WRITE: begin
         ac97ctrl_codec_data_write                <= sys_wdata[15:0];
         end

      endcase
      end
   end


wire sys_en = sys_wen | sys_ren;

// bus read access
always @(posedge clk_adc_125mhz)
if (!adc_rstn_i || ac97ctrl_reset_delay) begin
   sys_err                                        <= 1'b0;
   sys_ack                                        <= 1'b0;
   sys_rdata                                      <=  'b0;
   ac97ctrl_rec_fifo_pop                          <= 1'b0;
   ac97ctrl_rec_is_right                          <= 1'b0;
   end

else begin
   sys_err <= 1'b0;
   ac97ctrl_rec_fifo_pop <= 1'b0;                                                                           // default value, to be overwritten

   if (sys_ren) begin
      sys_ack                                     <= sys_en;
      case (sys_addr[19:0])

      /* FIFO */
      REG_RO_AC97CTRL_FIFO_REC: begin
         sys_ack                                  <= sys_en;
         if (!ac97ctrl_rec_is_right)
            sys_rdata                             <= { {C_OPB_DWIDTH - 16{1'b0}}, ac97ctrl_rec_fifo_read[15: 0] };
         else begin
            sys_rdata                             <= { {C_OPB_DWIDTH - 16{1'b0}}, ac97ctrl_rec_fifo_read[31:16] };
            ac97ctrl_rec_fifo_pop                 <= 1'b1;
            end
         ac97ctrl_rec_is_right                    <= !ac97ctrl_rec_is_right;
         end

      /* control */
      REG_RO_AC97CTRL_STAT: begin
         sys_ack                                  <= sys_en;
         sys_rdata                                <= { {C_OPB_DWIDTH -   8{1'b0}}, ac97ctrl_rec_fifo_overrun, ac97ctrl_play_fifo_underrun, ac97ctrl_codec_ready, ac97ctrl_access_ready, ac97ctrl_rec_fifo_empty, ac97ctrl_rec_fifo_full, ac97ctrl_play_fifo_hf, ac97ctrl_play_fifo_full};
         end
      REG_RO_AC97CTRL_CODEC_DATA_READ: begin
         sys_ack                                  <= sys_en;
         sys_rdata                                <= { {C_OPB_DWIDTH -  16{1'b0}}, ac97ctrl_codec_data_read[15:0] };
         end

      default:   begin
         sys_ack                                  <= sys_en;
         sys_rdata                                <= 'b0;
         end

      endcase
      end

   else if (sys_wen) begin                                                                                  // keep sys_ack assignment in this process to remove the need for a comb. logic to generate this resulting signal from write || read
      sys_ack <= sys_en;
      if ((sys_addr[19:0] == REG_WO_AC97CTRL_FIFO_RESET) && sys_wdata[1])                                   // reset record FIFO
         ac97ctrl_rec_is_right <= 1'b0;
      end

   else begin
      sys_ack <= 1'b0;
      end
   end

assign ac97_leds_o[7:0] = { ac97ctrl_codec_ready, ac97ctrl_access_ready, 1'b0, ac97ctrl_rec_fifo_overrun, ac97_irq_rec_o, 1'b0, ac97ctrl_play_fifo_underrun, ac97_irq_play_o };

endmodule
