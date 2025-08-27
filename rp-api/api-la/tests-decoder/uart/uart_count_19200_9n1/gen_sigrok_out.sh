#/bin/bash
sigrok-cli -i ./*.sr -P uart:parity=none:data_bits=9:baudrate=19200 | grep -e "^.\{11\}$" -e Start -e Stop -e Parity > sigrock.out
