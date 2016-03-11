#!/bin/sh

cd /tmp

echo "Changing mode to RW ..."
rw

echo "Removing radiobox directory ..."
rm -rf /opt/redpitaya/www/apps/radiobox

echo "Unpacking of radiobox*.zip file ..."
cd /opt/redpitaya/www/apps
unzip /tmp/radiobox*.zip >/dev/null
cd /tmp

echo "Removing radiobox*.zip file ..."
rm -f /tmp/radiobox-*.zip

echo "Syncing ..."
sync

echo "Changing mode to RO ..."
(sleep 1; ro)&

echo "Starting nginx ..."
/opt/redpitaya/www/apps/radiobox/bin/nginx_restart_0-94.sh

echo "done."
