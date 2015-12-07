```C
////////////////////////////////////////////////////////////////////////////////
//
// Block diagram:
//
//             input [DW-1:0] o
//                            |
//                     ----------------          -----
//  mask [DW-1:0] --> | DDR input pins | <--o-- | PLL | <-- configuration
//                     ----------------     |    -----
//                            |             |
//             data [2DW-1:0] o             |
//                            |             |
//                     ----------------     |
//  enable        --> | CDC            | <--
//                     ----------------
//                            |
//                     ----------------
//  enable        --> | glitch filter  |
//                     ----------------
//                            |
//                     ----------------
//  dec  [32-1:0] --> | decimation     |
//                     ----------------
//                            |
//                            o---------------------
//                            |                     |
//                     ----------------     ----------------  
//  delay         --> | delay buffer   |   | trigger        | <-- configuration
//                     ----------------     ----------------  
//                            |                     |
//                     ----------------             |
//  configuration --> | acquire        | <----------
//                     ----------------
//                            |
//                     ----------------
//  mask          --> | realign        |
//                     ----------------
//                            |
//                     ----------------
//  configuration --> | RLE            |
//                     ----------------
//                            |
//                     ----------------
//  configuration --> | byter          |
//                     ----------------
//                            |
//                            |
//                            |
//                     ----------------
//  configuration --> | DMA            |
//                     ----------------
//
////////////////////////////////////////////////////////////////////////////////
//
// Description:
//
// DDR input pins
//
// Depending on the available clocks, the output from this block can run at
// double the data rate, or double the data width. A basic mask is applied here
// to avoid toggling on unused inputs from propagating further, where it
// increases power consumption. In addition to DDR capabilities, this block also
// prevents metastable signals from propagating further.
// A configurable PLL can be used for fine grained sample clock configuration,
// frequency and phase can be programmed. It can also be used to move the
// sampling phase of an external clock.
// 
// CDC
//
// In case an external sampling clock is provided, it is necessary to implement
// clock domain crossing. Otherwise this block can be bypassed.
//
// Glitch filter
//
// It is possible to remove single sample glitches from the stream. Use with
// care, since it can result in data loose.
//
// Decimation
//
// If input sampling frequency can not be configuret, than this module decimates
// the data rate.
//
// Trigger, delay buffer
//
// Detection of simple and complex trigger events. The trigger signal is than
// passed to acquire. Since detecting a complex trigger can be pipelined, it can
// take a few samples. A delay buffer is provided, to delay the data stream
// by the same amount.
// The next equations describe trigger conditions. Comparator is used to allow
// triggering only if the current masked data is equal to the required value.
// Edge detection alows detecting positive (data_old==0, data==1) and negative
// (data_old==1, data==0) edges. It is possible to trigger on both edges. If
// edge detection is enabled on more than one bit, then an edge on any of the
// active bits will cause a trigger event. Both the comparator and edge
// conditions must be met for a trigger. Both have a default state, so that
// only one of them can be used for the trigger condition.
//
// trg_cmp = (data & trg_cmp_msk) == (trg_cmp_val & trg_cmp_msk);
// trg_edg = |(trg_edg_pos & (~data_old &  data)
//         | |(trg_edg_neg & ( data_old & ~data);
// trg     = trg_cmp & trg_edg;
//
// Acquire
//
// This is the same block used in the osciloscope. It packs the input stream
// into packets with a known trigger position. Should support at leaset the
// next functionality.
// 1. sending the stream continuously (continuous mode)
// 2. sending the stream continuously and stopping it a known number of samples
//    after the trigger arrives (trigger mode)
// 3. sending a single packet with a known number of samples
// 4. the number of samples sent before a trigger must also be counted
//
// Realign
//
// Realigning moves data bits inside the whole data vector, so that all used
// bits are moved togather, and the unused part of the vector, can be skipped
// in the byter.
//
// RLE
//
// Run length encoding is a simple loseless data compression technique. Which
// can be applied to a continuous data stream.
//
// Byter
//
// Organize the data stream into one or 2^n bytes, since this is required by
// the DMA, and further memory storage.
//
//
////////////////////////////////////////////////////////////////////////////////
```

```C
// memory map

struct {
   // input stage
   uint32_t dio_clk_src; // [0]       // sampling clock source (0 - internal, 1 - external)
   uint32_t dio_msk;     // [DW-1:0]  // Digital input mask
   uint32_t cdc_ena;     // [0]       // clock domain crossing enable
   uint32_t dgf_ena;     // [0]       // digital glitch filter enable
   uint32_t dec;         // [32-1:0]  // (dec+1) equals the decimation factor
   // trigger
   uint32_t trg_cmp_msk; // [DW-1:0]  // digital comparator mask
   uint32_t trg_cmp_val; // [DW-1:0]  // digital comparator value
   uint32_t trg_edg_pos; // [DW-1:0]  // digital edge positive mask
   uint32_t trg_edg_neg; // [DW-1:0]  // digital edge negative mask
   // acquire
   uint32_t acq_run;                  // write - run enable, read - run status
   uint32_t acq_trg;                  // read - trigger status
   uint32_t acq_trg_src;              // trigger source multiplexer
   // output stage
   uint32_t lgn_mask;                 // realign mask, lists bits which should go forward
   uint32_t rle_ena;                  // run length encoding enable
} la_regset_t;

```

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

