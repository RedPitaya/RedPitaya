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

X=$(monitor 0x40000000)
hexToDec $X
echo "ID 0x40000000 : $X ($RET)"

X=$(monitor 0x40000004)
hexToDec $X
echo "DNA1 0x40000004 : $X ($RET)"

X=$(monitor 0x40000008)
hexToDec $X
echo "DNA2 0x40000008 : $X ($RET)"

X=$(monitor 0x4000000C)
hexToDec $X
echo "Digital Loop 0x4000000C : $X ($RET)"

X=$(monitor 0x40000010)
hexToDec $X
echo "Expansion connector direction P 0x40000010 : $X ($RET)"

X=$(monitor 0x40000014)
hexToDec $X
echo "Expansion connector direction N 0x40000014 : $X ($RET)"

X=$(monitor 0x40000018)
hexToDec $X
echo "Expansion connector output P 0x40000018 : $X ($RET)"

X=$(monitor 0x4000001C)
hexToDec $X
echo "Expansion connector output N 0x4000001C : $X ($RET)"


X=$(monitor 0x40000020)
hexToDec $X
echo "Expansion connector input P 0x40000020 : $X ($RET)"

X=$(monitor 0x40000024)
hexToDec $X
echo "Expansion connector input N 0x40000024 : $X ($RET)"

X=$(monitor 0x40000030)
hexToDec $X
echo "LED control 0x40000030 : $X ($RET)"
