#!/bin/sh
#
# Run as root.  Expands the root file system.  After running this,
# reboot and give the script a few minutes to finish expanding the file system.
# Check the file system with 'df -h' once it has run and you should see a size
# close to the known size of your card.
#

# Get the starting offset of the root partition
PART_START=$(parted /dev/mmcblk0 -ms unit s p | grep "^2" | cut -f 2 -d: | grep -o '[0-9]*')
[ "$PART_START" ] || return 1
# Return value will likely be error for fdisk as it fails to reload the
# partition table because the root fs is mounted
sudo fdisk /dev/mmcblk0 <<EOF
p
d
2
n
p
2
$PART_START

p
w
EOF


rw
grep -v "resize2fs" /opt/redpitaya/sbin/startup.sh > /opt/redpitaya/sbin/startup.sh2; mv /opt/redpitaya/sbin/startup.sh2 /opt/redpitaya/sbin/startup.sh
echo resize2fs /dev/mmcblk0p2 >> /opt/redpitaya/sbin/startup.sh
echo 'rw;grep -v "resize2fs" /opt/redpitaya/sbin/startup.sh > /opt/redpitaya/sbin/startup.sh2; mv /opt/redpitaya/sbin/startup.sh2 /opt/redpitaya/sbin/startup.sh;ro' >> /opt/redpitaya/sbin/startup.sh
ro
echo "Root partition has been resized. The filesystem will be enlarged upon the next reboot"
