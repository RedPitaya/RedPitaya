#!/bin/bash

if [ -z "$(dpkg -l | grep ' tree ')" ]; then
    apt install tree -y
fi

DEST_FILE=$1
if [ "$DEST_FILE" = "" ]; then
    DEST_FILE=/opt/redpitaya
fi

TEST_TMP_DIR=$(mktemp -d)
ZIP_FILE=$(date +'%F_%H-%M-%S').zip

CURDIR=$(pwd)

dmesg > $TEST_TMP_DIR/dmesg.log
journalctl > $TEST_TMP_DIR/journalctl.log
systemctl >  $TEST_TMP_DIR/systemctl.log
systemctl status --all  >  $TEST_TMP_DIR/systemctl_status.log

lsblk > $TEST_TMP_DIR/lsblk.log
df -h > $TEST_TMP_DIR/df.log
tree /opt   -s -D --info > $TEST_TMP_DIR/tree.log
ifconfig -a > $TEST_TMP_DIR/ifconfig.log
ip a > $TEST_TMP_DIR/ip_a.log
i2cdetect -y -r 0 > $TEST_TMP_DIR/i2c.log
lsusb  -t > $TEST_TMP_DIR/usb.log

fw_printenv > $TEST_TMP_DIR/fw_printenv.log

mkdir -p $TEST_TMP_DIR/fpga
mkdir -p $TEST_TMP_DIR/ecosystem
mkdir -p $TEST_TMP_DIR/logs

/opt/redpitaya/sbin/scripts/reg_house.sh > $TEST_TMP_DIR/fpga/reg_house.log
/opt/redpitaya/sbin/scripts/reg_osc.sh > $TEST_TMP_DIR/fpga/reg_osc.log
/opt/redpitaya/sbin/scripts/reg_sig_gen.sh > $TEST_TMP_DIR/fpga/reg_sig_gen.log



cp /tmp/loaded_fpga.inf  $TEST_TMP_DIR/loaded_fpga.inf
cp /tmp/sysinfo.json $TEST_TMP_DIR/sysinfo.json
cp /opt/redpitaya/wpa_supplicant.conf $TEST_TMP_DIR/wpa_supplicant.conf
cp /opt/redpitaya/hostapd.conf $TEST_TMP_DIR/hostapd.conf


cp -r /root/.config/redpitaya $TEST_TMP_DIR/ecosystem
cp /root/.version $TEST_TMP_DIR/OS_Ver.log
cp -r /var/log $TEST_TMP_DIR/logs

calib -rv > $TEST_TMP_DIR/ecosystem/calib_rv.log
calib -rvf > $TEST_TMP_DIR/ecosystem/calib_rvf.log
calib -rvx > $TEST_TMP_DIR/ecosystem/calib_rvx.log
calib -u > $TEST_TMP_DIR/ecosystem/calib_u.log

monitor -p > $TEST_TMP_DIR/ecosystem/monitor_p.log
monitor -ams > $TEST_TMP_DIR/ecosystem/monitor_ams.log
echo $(monitor -f) > $TEST_TMP_DIR/ecosystem/monitor.log
echo $(monitor -i) >> $TEST_TMP_DIR/ecosystem/monitor.log
echo $(monitor -n) >> $TEST_TMP_DIR/ecosystem/monitor.log

rw
# pack report
cd $TEST_TMP_DIR
zip -9 -r $ZIP_FILE *
cp -f $ZIP_FILE $DEST_FILE
cd $CURDIR
ro

rm -rf $TEST_TMP_DIR
