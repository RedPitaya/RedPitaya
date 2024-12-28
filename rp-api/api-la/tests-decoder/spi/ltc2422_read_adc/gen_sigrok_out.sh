#/bin/bash
sigrok-cli -i ./*.sr -P spi:mosi=1:clk=0:cs=2 | grep "^.\{9\}$" > sigrock.out
