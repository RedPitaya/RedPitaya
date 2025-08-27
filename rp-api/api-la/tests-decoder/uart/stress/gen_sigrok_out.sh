#/bin/bash
sigrok-cli -i ./*.sr -P uart:parity=none:data_bits=8:baudrate=19200 | grep -e "^.\{10\}$" -e Start -e Stop -e Parity -e 'Frame error' > sigrock.out
