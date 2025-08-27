#/bin/bash
sigrok-cli -i ./*.sr -P i2c | grep -e Start -e ACK -e "Address" -e "Data" > sigrock.out
