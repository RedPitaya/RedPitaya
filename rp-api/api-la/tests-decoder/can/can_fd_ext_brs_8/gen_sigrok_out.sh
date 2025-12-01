#/bin/bash
sigrok-cli -i ./*.sr -P can | grep 0x > sigrock.out
