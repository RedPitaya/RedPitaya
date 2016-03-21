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
// DMA
//
// The DMA is provided by Xilinx
//
////////////////////////////////////////////////////////////////////////////////
```

# Memory map

The memory map is composed of smaller structures each describing a reusable
subcomponent. Same components might be used in the oscilloscope an logic
analyzer.

The trigger source is a separate logic analyzer component, it provides only one
of the available trigger sources.

```C
// trigger source configuration
struct {
    uint32_t cmp_msk; // [DW-1:0]  // digital comparator mask
    uint32_t cmp_val; // [DW-1:0]  // digital comparator value
    uint32_t edg_pos; // [DW-1:0]  // digital edge positive mask
    uint32_t edg_neg; // [DW-1:0]  // digital edge negative mask
} regset_la_trg_t;
```

The acquire module is shared between the oscilloscope and logic analyzer. At
the input it receives a continuous data stream. At the output it provides either
a continuous stream, or packets (of known or undefined-open size).

Trigger configuration allows for setting the acquisition of single data packets.

In the following example data around a trigger is acquired. `trg_siz` can be used
to require more than one trigger event inside the acquired package. `trg_pre` is
used to specify the minimum number of samples to be streamed before trigger
counting starts. After `trg_cnt` reaches `trg_siz` another `trg_pst` samples must
be stored, before the last sample in the packet is streamed and acquisition stops.

```
trg_cnt = 0..................1.......2.......3 = trg_siz

........_____________________T_______T_______T__________________________........
                             
       |<------------------->|               |<------------------------>|
               trg_pre                                 trg_pst
```

Another example requires just a specified amount of samples after acquire is
started (`run` request). I am not sure yet, how to handle the start of acquisition.

```
.......T__________________________........
       
       |<------------------------>|
                 trg_pst
```

Status registers provide values mainly used in modes, where the acquisition
stream is running continuously. The number of samples from acquisition start is
counted, as is the number of trigger events since start. There is a limited
size FIFO holding the position of the last `trg_siz` samples. `trg_num` holds
the number of trigger events still present in the FIFO, one of the bits is used
to indicate overflow. Trigger FIFO size is configurable so that for example only
the last trigger is needed it can be obtained without reading the whole FIFO. 

```C
struct {
   // trigger configuration
   uint32_t trg_pre;  // [32-1:0]  // least amount of data to store pre trigger
   uint32_t trg_pst;  // [32-1:0]  // amount of data to store post trigger
   uint32_t trg_hld;  // [32-1:0]  // trigger hold, minimum delay between two triggers
   uint32_t trg_siz;  // [32-1:0]  // trigger FIFO buffer size
   // status
   uint32_t acq_cnt;  // [32-1:0]  // counter of acquired samples        in the current stream
   uint32_t trg_cnt;  // [32-1:0]  // counter of received trigger events in the current stream
   uint32_t trg_num;  // [32-1:0]  // number of trigger events stored in FIFO buffer
   uint32_t trg_buf;  // [32-1:0]  // trigger FIFO buffer read register
   // control/status
   uint32_t trg_ena;  // [TW-1:0]  // trigger source mask, more than one source is allowed
   uint32_t sts_run :1;            // write - run enable, read - run status
   uint32_t sts_trg :1;            // read - trigger status
} regset_acq_t;
```

The whole register set structure is a combination of the above structures and
other configuration registers.

```C
struct {
   // input stage
   uint32_t dio_clk_src; // [0]       // sampling clock source (0 - internal, 1 - external)
   uint32_t dio_msk;     // [DW-1:0]  // Digital input mask
   uint32_t cdc_ena;     // [0]       // clock domain crossing enable
   uint32_t dgf_ena;     // [0]       // digital glitch filter enable
   uint32_t dec;         // [32-1:0]  // (dec+1) equals the decimation factor
   // configuration - trigger
   regset_la_trg_t trg;
   // acquire
   regset_acq_t    acq;
   // output stage
   uint32_t lgn_mask;                 // realign mask, lists bits which should go forward
   uint32_t rle_ena;                  // run length encoding enable
} la_regset_t;
```

```C
////////////////////////////////////////////////////////////////////////////////
//
// API:
//
// The current plan is to create an API function for each configuration option,
// some options like trigger will be combined into a single API function.
//
// The other option would be to provide a configuration structure similar to the
// memmory map, containing all configuration option, and then provide a single
// API function, which would accept this structure as argument.
//
// int rp_la_cfg (int unsigned channel, rp_la_cfg_t cfg);
//
// The would be separate API functions for starting the acquire process, the
// plan is to make them blocking, and would return a pointer to data once the
// trigger and post trigger data arrives.
//
////////////////////////////////////////////////////////////////////////////////

int get_data (uint32_t trg_pos, int32_t from, int32_t to, bool block, int16_t *data, uint32_t *size);
```

```C
////////////////////////////////////////////////////////////////////////////////
//
// Programming sequence
//
// 1. program output stage registers
// 2. program trigger condition and trigger source (mask allows multiple sources)
// 3. program input stage registers to enable the data folow
// 4. run rp_la_dat (data) and wait for it to return, some applications will have
//    to run is in a separate thread due to blocking
//
////////////////////////////////////////////////////////////////////////////////
```

# Acquire use cases

## 1. Auto refreshing oscilloscope

The data is continuously written into a circular buffer by the DMA. The buffer
is large enough to avoid observed data being overwritten in all but extreme
cases.

When the display code requires new data it first asks for the last trigger
event. The returned value is the position of the last received trigger inside
the data stream.

```C
uint32_t trg_pos;
get_trg_pos(&trg_pos);
```

If the trigger position is the same as by the previous request, then the
new data is the same as what is displayed, so there is no need to resend it.
If the trigger position changed, then new data can be requested.

```C
int16_t data [1024]; // 
get_data (uint32_t trg_pos, int32_t from, int32_t to, bool block, int16_t *data, uint32_t *size);
```

Values `from` and `to` are relative positions compared to the trigger position,
which is absolute. If the value is negative, data before the trigger is
requested, if it is positive, data after trigger is requested. Data is returned
as a pointer to the raw data buffer. There is no data conversion, and no
copies are made. All processing (conversion to float, math, ...) must be done
on smaller segments.

Data before trigger is available immediately, while data after trigger might
require some waiting. This waiting is implemented by blocking the return from
the function.

### Request data before trigger

Both `from` and `to` values are negative. Data is available, unless there was
no trigger received yet. User must check that `trigger+from > 0` otherwise
some data can be invalid.

```
 ---------------------------------------------------------------------------
|                                                                           |
 ---------------------------------------------------------------------------
       ^             ^                  ^   ^
       from         to            trigger   write pointer
```

### Request data around trigger (most common case)

This is the most common case, where the user observes the waveform around the
trigger on the screen. Since it is possible some requested data after the
trigger did not arrive yet, the request might block.

```
 ---------------------------------------------------------------------------
|                                                                           |
 ---------------------------------------------------------------------------
       ^                   ^   ^                      ^
       from          trigger   write pointer         to
```

### Request data after trigger
Depending on data rate and the distance from the trigger, blocking might take
a long time. This will reduce the display rate, so it should be used
carefully.

```
 ---------------------------------------------------------------------------
|                                                                           |
 ---------------------------------------------------------------------------
                ^   ^              ^                  ^
          trigger   write pointer  from              to
```

There are two additional modes described here, for fast signals it makes sense
to get the whole request in one piece. This is achieved by setting `block=1`.
With low data rates, it makes sense to display data as it is received, if
`block=0` the function will not block, instead it will return the `size` of
the returned data, and the UI will be able to draw the waveform as new data
arrives. The rate of this non blocking requests should be limited to the
desired refresh rate, so it should not be done in a short loop.

## 2. Auto refreshing oscilloscope (low power mode)

The size of the buffer is reduced to a minimum. Data is not streamed
continuously, instead streaming is only started on request by the user
interface. The time required for getting data after a request is high and
affecting the display rate.

## 3. Single shot oscilloscope and logic analyzer mode

In this mode a single data package is stored into the buffer. After starting
acquisition the data will be streamed into the buffer as if it was circular.
After the pre trigger time is exhausted, new triggers will start the post
trigger time counter. After this is exhausted, the streaming ends. It is also
possible to stop acquisition using an explicit `stop`, but then there would
be no trigger to display.
After the buffer is loaded, SW can display its contents relative to the
trigger.
It is not clear yet, how will 

## 4. Continuous acquisition

There is no need for a trigger, so only the `run` bit is used to control
the data flow. Acquisition is started by writing `1` into the `run` bit, and
stopped by writing `0` into the `run` bit. The stop condition will add a last
signal to the AXI4-Stream.
The DMA should be configured to return from a data request after a packet of
data with a specified size is received. The packet size depends on the data
rate (decimation), for example can be aligned to the screen refresh rate.
The packet containing the last sample can be handled differently by the DMA,
especially, since it might be shorter then other packets.
Another option would be for the application to only request data at a
specific rate (for example 60fps), and the DMA driver would return as much
data as it received after the previous request was made. If the last sample
was received, this must be indicated.

## 5. Acquisition started by run or trigger and ended by counter (optionally stop)

This mode is intended for measurements combining the generator and acquire.
The main point of this mode is the data in the buffer is never overwritten,
so only the amount of data which can fit into the buffer is streamed into
RAM.
