#/bin/bash
sigrok-cli -i ./*.sr -P uart:parity=none:data_bits=6:baudrate=19200 | grep -e "^.\{10\}$" -e Start -e Stop -e Parity > sigrock.out
