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

X=$(monitor 0x40200000)
hexToDec $X
echo "Configuration 0x40200000 : $X ($RET)"
echo "	- chA trigger selector: $(bitWiseAnd $X 0xF)"
echo "	- chA SM wrap: $(bitWiseAnd $X 0x10)"
echo "	- Reserve: $(bitWiseAnd $X 0x20)"
echo "	- chA SM reset: $(bitWiseAnd $X 0x40)"
echo "	- chA output: $(bitWiseAnd $X 0x80)"
echo "	- chA external gated burst: $(bitWiseAnd $X 0x100)"
echo
echo "	- chB trigger selector: $(bitWiseAnd $X 0xF0000)"
echo "	- chB SM wrap: $(bitWiseAnd $X 0x100000)"
echo "	- Reserve: $(bitWiseAnd $X 0x200000)"
echo "	- chB SM reset: $(bitWiseAnd $X 0x400000)"
echo "	- chB output: $(bitWiseAnd $X 0x800000)"
echo "	- chB external gated burst: $(bitWiseAnd $X 0x1000000)"
echo

X=$(monitor 0x40200004)
hexToDec $X
echo "Ch A Amplitude and Scale 0x40200004 : $X ($RET)"
echo "	- Scale: $(bitWiseAnd $X 0x3FFF)"
echo "	- Offset: $(bitWiseAnd $X 0x3FFF0000)"
echo

X=$(monitor 0x40200008)
hexToDec $X
echo "Ch A Counter wrap 0x40200008 : $X ($RET)"

X=$(monitor 0x4020000C)
hexToDec $X
echo "Ch A Start offset 0x4020000C : $X ($RET)"

X=$(monitor 0x40200010)
hexToDec $X
echo "Ch A Counter step 0x40200010 : $X ($RET)"

X=$(monitor 0x40200014)
hexToDec $X
echo "Ch A Buffer current read pointer 0x40200014 : $X ($(bitWiseAnd $X 0xFFFC))"

X=$(monitor 0x40200018)
hexToDec $X
echo "Ch A number of read cycles in one burst 0x40200018 : $X ($RET)"

X=$(monitor 0x4020001C)
hexToDec $X
echo "Ch A number of burst repetitions 0x4020001C : $X ($RET)"


X=$(monitor 0x40200020)
hexToDec $X
echo "Ch A delay between burst repetitions 0x40200020 : $X ($RET)"



X=$(monitor 0x40200024)
hexToDec $X
echo "Ch B Amplitude and Scale 0x40200024 : $X ($RET)"
echo "  - Scale: $(bitWiseAnd $X 0x3FFF)"
echo "  - Offset: $(bitWiseAnd $X 0x3FFF0000)"
echo

X=$(monitor 0x40200028)
hexToDec $X
echo "Ch B Counter wrap 0x40200028 : $X ($RET)"

X=$(monitor 0x4020002C)
hexToDec $X
echo "Ch B Start offset 0x4020002C : $X ($RET)"

X=$(monitor 0x40200030)
hexToDec $X
echo "Ch B Counter step 0x40200030 : $X ($RET)"

X=$(monitor 0x40200034)
hexToDec $X
echo "Ch B Buffer current read pointer 0x40200034 : $X ($(bitWiseAnd $X 0xFFFC))"

X=$(monitor 0x40200038)
hexToDec $X
echo "Ch B number of read cycles in one burst 0x40200038 : $X ($RET)"

X=$(monitor 0x4020003C)
hexToDec $X
echo "Ch B number of burst repetitions 0x4020003C : $X ($RET)"

X=$(monitor 0x40200040)
hexToDec $X
echo "Ch B delay between burst repetitions 0x40200040 : $X ($RET)"


echo

X=$(monitor 0x40200044)
hexToDec $X
echo "Ch A last value after burst 0x40200044 : $X ($RET)"

X=$(monitor 0x40200048)
hexToDec $X
echo "Ch B last value after burst 0x40200048 : $X ($RET)"

