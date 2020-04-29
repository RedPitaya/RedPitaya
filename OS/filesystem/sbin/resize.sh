#!/bin/sh
#
# Taken from http://www.raspberryvi.org/wiki/doku.php/raspi-expand-rootfs
#
# Resize the root filesystem of a newly flashed Raspbian image.
# Directly equivalent to the expand_rootfs section of raspi-config.
# No claims of originality are made.
# Mike Ray.  Feb 2013.  No warranty is implied.  Use at your own risk.
#
# Run as root.  Expands the root file system.  After running this,
# reboot and give the resizefs-once script a few minutes to finish expanding the file system.
# Check the file system with 'df -h' once it has run and you should see a size
# close to the known size of your card.
#

# Get the starting offset of the root partition
PART_START=$(parted /dev/mmcblk0 -ms unit s p | grep "^2" | cut -f 2 -d: | grep -o '[0-9]*')
[ "$PART_START" ] || return 1
# Return value will likely be error for fdisk as it fails to reload the
# partition table because the root fs is mounted
fdisk /dev/mmcblk0 <<EOF
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

# now set up an init.d script
cat <<\EOF > /etc/init.d/resize2fs_once &&
#!/bin/sh
### BEGIN INIT INFO
# Provides:          resize2fs_once
# Required-Start:
# Required-Stop:
# Default-Start: 2 3 4 5 S
# Default-Stop:
# Short-Description: Resize the root filesystem to fill partition
# Description:
### END INIT INFO
. /lib/lsb/init-functions
case "$1" in
  start)
    log_daemon_msg "Starting resize2fs_once" &&
    resize2fs /dev/mmcblk0p2 &&
    update-rc.d -f resize2fs_once remove &&
    log_end_msg $?
    ;;
  *)
    echo "Usage: $0 start" >&2
    exit 3
    ;;
esac
EOF
chmod +x /etc/init.d/resize2fs_once &&
update-rc.d resize2fs_once defaults &&
  echo "Root partition has been resized. The filesystem will be enlarged upon the next reboot"
