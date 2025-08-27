#/bin/bash
sigrok-cli -i ./*.sr -P uart:parity=none:data_bits=8:baudrate=38400 | grep -e "^.\{10\}$" -e Start -e Stop -e Parity > sigrock.out
