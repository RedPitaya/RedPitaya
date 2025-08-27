#/bin/bash
sigrok-cli -i ./*.sr -P uart:parity=even:data_bits=8 | grep -e "^.\{10\}$" -e Start -e Stop -e Parity > sigrock.out
