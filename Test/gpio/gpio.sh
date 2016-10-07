#!/bin/sh

# GPIO base index
export BASE=906

echo "GPIO E2"
#for i in 8 9
#for i in 10 11 12 13
for i in 8 9 10 11 12 13
do
    INDEX=$(( $BASE + $i ))
    echo $INDEX > /sys/class/gpio/export
#    echo out    > /sys/class/gpio/gpio$INDEX/direction
#    echo 1      > /sys/class/gpio/gpio$INDEX/value
    echo in     > /sys/class/gpio/gpio$INDEX/direction
    echo -n "reading GPIO pin " $i " at index " $INDEX " VAL="
    cat           /sys/class/gpio/gpio$INDEX/value
    echo $INDEX > /sys/class/gpio/unexport
done

# GPIO base index
export BASE=968

echo "GPIO E1"
for i in $(seq 8 24)
do
    INDEX=$(( $BASE + $i ))
    echo $INDEX > /sys/class/gpio/export
#    echo out    > /sys/class/gpio/gpio$INDEX/direction
#    echo 1      > /sys/class/gpio/gpio$INDEX/value
    echo in     > /sys/class/gpio/gpio$INDEX/direction
    echo -n "reading GPIO pin " $i " at index " $INDEX " VAL="
    cat           /sys/class/gpio/gpio$INDEX/value
    echo $INDEX > /sys/class/gpio/unexport
done
