#image=$1
image=debian.img

#size=$2
size=1024

dd if=/dev/zero of=$image bs=1M count=$size

device=`losetup -f`

losetup                $device $image
sh OS/debian/debian.sh $device
losetup -d             $device
