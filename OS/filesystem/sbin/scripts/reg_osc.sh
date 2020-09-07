#!/bin/bash

bitWiseAnd() {
    local VALUE=$1
    local COUNTER=$2
    while [  $(( $COUNTER & 1 )) -eq 0 ]; do
        COUNTER=$(($COUNTER>>1))
	VALUE=$(($VALUE>>1))
    done
    printf "%s" "$(( $COUNTER & $VALUE ))"
}

hexToDec(){
   RET=$(printf "%d" $1)
}

X=$(monitor 0x40100000)
hexToDec $X
echo "Configuration 0x40100000 : $X ($RET)"
echo "	- Start writing data into memory (ARM trigger): $(bitWiseAnd $X 0x1)"
echo "	- Reset write state machine: $(bitWiseAnd $X 0x2)"
echo "	- Trigger has arrived stays on (1) until next arm or reset: $(bitWiseAnd $X 0x4)"
echo "	- Trigger remains armed after ACQ delay passes: $(bitWiseAnd $X 0x8)"
echo "	- ACQ delay has passed / (all data was written to buffer): $(bitWiseAnd $X 0x10)"
echo

X=$(monitor 0x40100004)
hexToDec $X
echo "Trigger source 0x40100004 : $X ($RET)"
echo "	- Trigger source: $(bitWiseAnd $X 0xF)"
echo

X=$(monitor 0x40100008)
hexToDec $X
echo "Ch A threshold 0x40100008 : $X ($RET)"

X=$(monitor 0x4010000C)
hexToDec $X
echo "Ch B threshold 0x4010000C : $X ($RET)"

X=$(monitor 0x40100010)
hexToDec $X
echo "Delay after trigger 0x40100010 : $X ($RET)"

X=$(monitor 0x40100014)
hexToDec $X
echo "Data decimation 0x40100014 : $X ($RET)"

X=$(monitor 0x40100018)
hexToDec $X
echo "Write pointer - current 0x40100018 : $X ($RET)"

X=$(monitor 0x4010001C)
hexToDec $X
echo "Write pointer - trigger 0x4010001C : $X ($RET)"


X=$(monitor 0x40100020)
hexToDec $X
echo "Ch A hysteresis 0x40100020 : $X ($RET)"


X=$(monitor 0x40100024)
hexToDec $X
echo "Ch B hysteresis 0x40100024 : $X ($RET)"
echo

X=$(monitor 0x40100028)
hexToDec $X
echo "Reserved Enable signal average at decimation 0x40100028 : $X ($RET)"

X=$(monitor 0x4010002C)
hexToDec $X
echo "PreTrigger Counter 0x4010002C : $X ($RET)"

X=$(monitor 0x40100030)
hexToDec $X
echo "CH A Equalization filter 0x40100030 : $X ($RET)"

X=$(monitor 0x40100034)
hexToDec $X
echo "CH A Equalization filter 0x40100034 : $X ($RET)"

X=$(monitor 0x40100038)
hexToDec $X
echo "CH A Equalization filter 0x40100038 : $X ($RET)"

X=$(monitor 0x4010003C)
hexToDec $X
echo "CH A Equalization filter 0x4010003C : $X ($RET)"


X=$(monitor 0x40100040)
hexToDec $X
echo "CH B Equalization filter 0x40100040 : $X ($RET)"

X=$(monitor 0x40100044)
hexToDec $X
echo "CH B Equalization filter 0x40100044 : $X ($RET)"

X=$(monitor 0x40100048)
hexToDec $X
echo "CH B Equalization filter 0x40100048 : $X ($RET)"

X=$(monitor 0x4010004C)
hexToDec $X
echo "CH B Equalization filter 0x4010004C : $X ($RET)"


X=$(monitor 0x40100050)
hexToDec $X
echo "CH A AXI lower address 0x40100050 : $X ($RET)"

X=$(monitor 0x40100054)
hexToDec $X
echo "CH A AXI upper address 0x40100054 : $X ($RET)"

X=$(monitor 0x40100058)
hexToDec $X
echo "CH A AXI delay after trigger 0x40100058 : $X ($RET)"

X=$(monitor 0x4010005C)
hexToDec $X
echo "CH A AXI enable master 0x4010005C : $X ($RET)"


X=$(monitor 0x40100060)
hexToDec $X
echo "CH A AXI write pointer - trigger 0x40100060 : $X ($RET)"

X=$(monitor 0x40100064)
hexToDec $X
echo "CH A AXI write pointer - current 0x40100064 : $X ($RET)"

X=$(monitor 0x40100070)
hexToDec $X
echo "CH B AXI lower address 0x40100070 : $X ($RET)"

X=$(monitor 0x40100074)
hexToDec $X
echo "CH B AXI upper address 0x40100074 : $X ($RET)"


X=$(monitor 0x40100078)
hexToDec $X
echo "CH B AXI delay after trigger 0x40100078 : $X ($RET)"

X=$(monitor 0x4010007C)
hexToDec $X
echo "CH B AXI enable master 0x4010007C : $X ($RET)"


X=$(monitor 0x40100080)
hexToDec $X
echo "CH B AXI write pointer - trigger 0x40100080 : $X ($RET)"

X=$(monitor 0x40100084)
hexToDec $X
echo "CH B AXI write pointer - current 0x40100084 : $X ($RET)"


X=$(monitor 0x40100090)
hexToDec $X
echo "Trigger debouncer time 0x40100090 : $X ($RET)"


X=$(monitor 0x401000A0)
hexToDec $X
echo "Accumulator data sequence length 0x401000A0 : $X ($RET)"

X=$(monitor 0x401000A4)
hexToDec $X
echo "Accumulator data offset corection ChA 0x401000A4 : $X ($RET)"

X=$(monitor 0x401000A8)
hexToDec $X
echo "Accumulator data offset corection ChB 0x401000A8 : $X ($RET)"