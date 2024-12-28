#/bin/bash
sigrok-cli -i ./*.sr -P can:can_rx=CAN_RX:nominal_bitrate=125000 | grep 0x > sigrock.out
