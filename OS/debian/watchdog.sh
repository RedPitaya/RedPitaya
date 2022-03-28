#Enable watchdog

sed -i 's/#RuntimeWatchdogSec=0/RuntimeWatchdogSec=5s/g' $ROOT_DIR/etc/systemd/system.conf
sed -i 's/#ShutdownWatchdogSec=10min/ShutdownWatchdogSec=10min/g' $ROOT_DIR/etc/systemd/system.conf
