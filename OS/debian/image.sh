script=$1
image=$2

size=512

if [ $# -eq 3 ]
then
  size=$3
fi

dd if=/dev/zero of=$image bs=1M count=$size

device=`losetup -f`

losetup $device $image

sh $script $device

losetup -d $device
