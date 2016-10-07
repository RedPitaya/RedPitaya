#!/bin/sh

# GPIO base index
export BASE=906

echo "GPIO"
for i in $(seq 0 128)
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
