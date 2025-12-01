#/bin/bash
sigrok-cli -i ./*.sr -P spi:mosi=1:clk=0:cs=3 | grep "^.\{9\}$" > sigrock.out
